/***************************************************************************
* Copyright (C) 2010 by Ammar Qammaz *
* ammarkov@gmail.com *
* *
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or *
* (at your option) any later version. *
* *
* This program is distributed in the hope that it will be useful, *
* but WITHOUT ANY WARRANTY; without even the implied warranty of *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the *
* GNU General Public License for more details. *
* *
* You should have received a copy of the GNU General Public License *
* along with this program; if not, write to the *
* Free Software Foundation, Inc., *
* 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. *
***************************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "VariableDatabase.h"
#include "NetworkFramework.h"
#include "JobTables.h"
#include "Peers.h"
#include "helper.h"
#include "ProtocolThreads.h"
#include "RemoteVariableSupport.h"


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/uio.h>

void * RemoteVariableServer_Thread(void * ptr);
void * RemoteVariableClient_Thread(void * ptr);

int SendRAWTo(int clientsock,char * message,unsigned int length)
{
  fprintf(stderr,"Trying to send `%s` to peer socket %d \n",message,clientsock);
  return send(clientsock,message,length, 0);
}

int RecvRAWFrom(int clientsock,char * message,unsigned int length)
{
  fprintf(stderr,"Trying RecvRAWFrom\n");
  memset (message,0,length);

  int retres=recv(clientsock,message,length, 0);

  fprintf(stderr,"Received `%s` from peer socket %d  \n",message,clientsock);
  return retres;
}

int RecvVariableFrom(struct VariableShare * vsh,int clientsock,unsigned int variable_id)
{
 struct NetworkRequestGeneralPacket data;
 data.RequestType = READFROM ; // This should remain readfrom after receiving the general packet..!
 data.data_size = vsh->share.variables[variable_id].size_of_ptr;


 fprintf(stderr,"Receiving GeneralPacket\n");
 int data_recv= recv(clientsock, (char*) & data, sizeof (data), 0); // SEND START FRAME!
 if (data_recv != sizeof (data) ) { fprintf(stderr,"Incorrect starting frame received ( %u instead of %u )\n",data_recv , sizeof (data) ); return 0; }
 if (data.data_size != vsh->share.variables[variable_id].size_of_ptr ) { fprintf(stderr,"Incorrect pointer size at frame received ( %u instead of %u )\n",data.data_size , vsh->share.variables[variable_id].size_of_ptr ); return 0; }


 //TODO BEEF UP SECURITY HERE :P
 fprintf(stderr,"Receiving Payload ( var was %d ",vsh->share.variables[variable_id].ptr);
 data_recv= recv(clientsock,vsh->share.variables[variable_id].ptr,data.data_size, 0); // GET VAR PAYLOAD!


 fprintf(stderr,"now %d  )\n",vsh->share.variables[variable_id].ptr);
 if ( data_recv != data.data_size ) { fprintf(stderr,"Incorrect payload received ( %u instead of %u )\n",data_recv , vsh->share.variables[variable_id].size_of_ptr ); return 0; }
 if ( vsh->share.variables[variable_id].GUARD_BYTE != RVS_GUARD_VALUE ) {  error("Buffer overflow attack @ RecvVariableFrom detected\n"); vsh->global_state=VSS_SECURITY_ALERT; return 0; }

 // TODO CHECK OPERATIONS ETC HERE!
 fprintf(stderr,"Updating hash value  \n");
 vsh->share.variables[variable_id].hash=GetVariableHash(vsh,variable_id);

 fprintf(stderr,"RecvVariableFrom , Great Success :) \n");
  return 1;
}

int SendVariableTo(struct VariableShare * vsh,int clientsock,unsigned int variable_id)
{
  struct NetworkRequestGeneralPacket data;
  data.RequestType=WRITEVAR;
  data.data_size = vsh->share.variables[variable_id].size_of_ptr;

  fprintf(stderr,"Sending GeneralPacket\n");
  int data_sent= send(clientsock, (char*) & data, sizeof (data), 0); // SEND START FRAME!
  if ( data_sent != sizeof (data) ) { fprintf(stderr,"Incorrect starting frame transmission ( %d instead of %u )\n",data_sent , sizeof (data) ); return 0; }

  fprintf(stderr,"Sending Payload\n");
  data_sent= send(clientsock,vsh->share.variables[variable_id].ptr,data.data_size , 0); // SEND VARIABLE!
  if ( data_sent != data.data_size ) { fprintf(stderr,"Incorrect payload transmission ( %d instead of %u )\n",data_sent , data.data_size ); return 0; }


  fprintf(stderr,"SendVariableTo , Great Success :) \n");
  return 1;
}


int RecvFileFrom(struct VariableShare * vsh,int clientsock,unsigned int variable_id)
{
  fprintf(stderr,"RecvFileFrom not implemented\n");
  return 0;
}

int SendFileTo(struct VariableShare * vsh,int clientsock,unsigned int variable_id)
{
  fprintf(stderr,"SendFileTo not implemented\n");
  return 0;
}


int WaitForSocketLockToClear(int peersock,unsigned int * peerlock)
{
  if (peerlock==0) { fprintf(stderr,"WaitForSocketLock skipped by null pointer \n"); return 0; }
  unsigned int waitloops=0;
  while (*peerlock)
   {
     usleep(1000);
     ++waitloops;
   }

  if (waitloops) { fprintf(stderr,"WaitForSocketLockToClear waited for %u ms ( loops )\n",waitloops); }
  return 1;
}

int LockSocket(int peersock,unsigned int * peerlock)
{
  if (peerlock==0) { fprintf(stderr,"LockSocket skipped by null pointer \n"); return 0; }
  if (*peerlock!=0) {
                      fprintf(stderr,"LockSocket found locked variable ! , waiting for it to clear out\n");
                      WaitForSocketLockToClear(peersock,peerlock);
                    }
  *peerlock=1;
  return 1;
}

int UnlockSocket(int peersock,unsigned int * peerlock)
{
  if (peerlock==0) { fprintf(stderr,"UnlockSocket skipped by null pointer \n"); return 0; }
  *peerlock=0;
  return 1;
}




/*
--------------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------
                /\  /\  /\   /\
                ||  ||  ||   ||                   LOW level socket operations

--------------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------



--------------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------

                ||  ||  ||   ||                   SERVER Framework
                \/  \/  \/   \/
--------------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------

*/



int UnifiedNetworkAndJobHandling(struct VariableShare * vsh,int peersock , unsigned int * peerlock)
{

   if(ProtocolServeResponse(vsh,peersock,peerlock))  { fprintf(stderr,"Handled Client successfully\n"); } else
                                                        { fprintf(stderr,"Error while handling client\n"); return 0; }

   /* TEMPORARY INSTEAD OF JOB EXECUTIONER */
   if (!vsh->pause_job_thread)
   {
     vsh->total_jobs_done+=ExecutePendingJobs(vsh);
   }
   /* TEMPORARY INSTEAD OF JOB EXECUTIONER */

  return 1;
}



void *
PeerServer_Thread(void * ptr)
{
  fprintf(stderr,"Peer Thread : New Peer thread just spawned , trying to find its context ");
  struct PeerServerContext * thread_context=(struct PeerServerContext *) ptr;
  struct VariableShare * vsh = (struct PeerServerContext *) thread_context->vsh;
  unsigned int peer_id = (struct PeerServerContext *) thread_context->peer_id;
  int peersock = (struct PeerServerContext *) thread_context->peersock;
  fprintf(stderr," done\n");

  fprintf(stderr,"Peer Thread : New Peer thread : peer_id=%u , socket=%d \n",peer_id,peersock);

   //First thing to do negotiate with peer about the list , passwords etc
   if (Accept_Handshake(vsh,peersock))
     {
       debug_say("Peer Thread : Successfully accepted connection handshake\n");
       vsh->global_state=VSS_NORMAL;
     } else
     {
       fprintf(stderr,"Peer Thread : Could not accept handshake for RemoteVariable Share , ignoring client\n");
       close(peersock);
       return 0;
     }

   fprintf(stderr,"Peer Thread : Getting peerlock for Server HandleClientLoop using %u peer id , initial value is %u ",peer_id,vsh->peer_list[peer_id].socket_locked);
   unsigned int * peerlock =  &vsh->peer_list[peer_id].socket_locked;
   unsigned int total_jobs_done=0;

   while (UnifiedNetworkAndJobHandling(vsh,peersock,peerlock))
    {
        usleep(100);
    }

    RemPeerBySock(vsh,peersock);
    return 0;
}

int GenerateNewClientLoop(struct VariableShare * vsh,int clientsock,struct sockaddr_in client,unsigned int clientlen)
{
   fprintf(stderr,"Server Thread : Client connected: %s\n", inet_ntoa(client.sin_addr));
   unsigned int peer_id=AddPeer(vsh,inet_ntoa(client.sin_addr),0,clientsock);
   if ( peer_id == 0 ) { error("Server Thread : Failed to add peer to list while @ RemoteVariableClient_Thread "); }
   --peer_id; // Step down one value !


    vsh->peer_list[peer_id].peer_thread=0;
    vsh->peer_list[peer_id].pause_peer_thread=0;
    vsh->peer_list[peer_id].stop_peer_thread=0;


  vsh->pause_server_thread=0;
  vsh->stop_server_thread=0;

  struct PeerServerContext pass_to_thread;
  pass_to_thread.vsh=vsh;
  pass_to_thread.peer_id = peer_id;
  pass_to_thread.peersock = clientsock;

  fprintf(stderr,"Server Thread : Server is ready to spawn a new dedicated thread for the client .. ");
  int retres = pthread_create(&vsh->peer_list[peer_id].peer_thread,0,PeerServer_Thread,(void*) &pass_to_thread);
  fprintf(stderr,"done\n");

  if (retres!=0) retres = 0; else
                 retres = 1;

  return retres;
}

void *
RemoteVariableServer_Thread(void * ptr)
{
  if ( debug_msg() ) printf("Server Thread : Remote Variable TCP Server thread started..\n");

  struct VariableShare * vsh = (struct VariableShare *) ptr;


  int serversock,clientsock;
  unsigned int serverlen = sizeof(struct sockaddr_in),clientlen = sizeof(struct sockaddr_in);
  struct sockaddr_in server;
  struct sockaddr_in client;

  serversock = socket(AF_INET, SOCK_STREAM, 0);
    if ( serversock < 0 ) { error("Server Thread : Opening socket"); return 0; }


  bzero(&client,clientlen);
  bzero(&server,serverlen);

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(vsh->port);

  if ( bind(serversock,(struct sockaddr *) &server,serverlen) < 0 ) { error("Server Thread : Error binding master port for RemoteVariables!"); return 0; }
  if (listen(serversock,10) < 0)  { error("Server Thread : Failed to listen on server socket"); return 0; }


  while (vsh->stop_server_thread==0)
  {
    debug_say("Server Thread : Waiting for a new client");
    /* Wait for client connection */
    if ( (clientsock = accept(serversock,(struct sockaddr *) &client, &clientlen)) < 0) { error("Server Thread : Failed to accept client connection"); }
      else
      {
           fprintf(stderr,"Server Thread : Accepted new client \n");
           if (!GenerateNewClientLoop(vsh,clientsock,client,clientlen))
            {
                fprintf(stderr,"Server Thread : Client failed, while handling him\n");
                close(clientsock);
            }
      }
 }
  return 0;
}





/*
--------------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------
                /\  /\  /\   /\
                ||  ||  ||   ||                   SERVER Framework

--------------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------



--------------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------

                ||  ||  ||   ||                   Client Framework
                \/  \/  \/   \/
--------------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------

*/


int RemoteVariable_InitiateConnection(struct VariableShare * vsh)
{
  int sockfd;
  struct hostent *he=0;
  struct sockaddr_in their_addr;


  if ((he=gethostbyname(vsh->ip)) == 0)
  {
    fprintf(stderr,"Error getting host by name %s \n",vsh->ip);
    return 0;
   }
    else
    printf("Client-The remote host is: %s\n", vsh->ip);

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    error("Could not create the socket for the connection\n");
    return 0;
  }
    else
    printf("Client socket ok\n");


    // host byte order
    their_addr.sin_family = AF_INET;
    // short, network byte order
    printf("Server-Using %s:%d...\n", vsh->ip, vsh->port);
    their_addr.sin_port = htons(vsh->port);
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    // zero the rest of the struct

     memset(&(their_addr.sin_zero), '\0', 8);



   if(connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1)
   {
    error("Could not connect the created socket \n");
    vsh->global_state=VSS_CONNECTION_FAILED;
    return 0;
   }
    else
   {
    printf("Client-The connect() is OK...\n");
   }

   vsh->master.socket_to_client=sockfd;
   return 1;
}



void *
RemoteVariableClient_Thread(void * ptr)
{
  debug_say("Remote Variable TCP Server thread started..\n");
  struct VariableShare *vsh=0;
  vsh = (struct VariableShare *) ptr;
  if (vsh==0) { fprintf(stderr,"Virtual Share Parameter is damaged \n"); return 0; }


  if (!RemoteVariable_InitiateConnection(vsh))
    {
      fprintf(stderr,"Could not Initiate Connection\n");
      vsh->global_state=VSS_CONNECTION_FAILED;
      return 0;
    }

   int peer_id = 0;
   int peersock =  vsh->master.socket_to_client;
   //First thing to do negotiate with peer about the list , passwords etc
   if (Connect_Handshake(vsh,peersock))
     {
       fprintf(stderr,"Successfull connection handshake , socket to master = %d \n",peersock);

       peer_id = AddPeer(vsh,vsh->ip,vsh->port,peersock); // peersock doesnt seem to be the right value to pass :S or it gets overflowed
       if ( peer_id == 0 ) { error("Failed to add peer to list while @ RemoteVariableClient_Thread "); }
       --peer_id; // Step down one value !

       vsh->global_state=VSS_NORMAL;
     } else
     {
       fprintf(stderr,"Could not handshake for RemoteVariable Share , stopping everything\n");
       vsh->global_state=VSS_HANDSHAKE_FAILED;
       return 0;
     }

   // ClearAllJobs(vsh); // <- This has to be added later to enforce sync issues .. we are now synced to master so clear all local jobs
   fprintf(stderr,"Getting peerlock for Client thread using %u peer id , initial value is %u ",peer_id,vsh->peer_list[peer_id].socket_locked);
   unsigned int * peerlock =  &vsh->peer_list[peer_id].socket_locked;
   unsigned int total_jobs_done=0;

   while (vsh->stop_client_thread==0)
   {
      UnifiedNetworkAndJobHandling(vsh,peersock,peerlock);
      usleep(100);
   }

  RemPeerBySock(vsh,peersock);

  return 0;
}



/*
--------------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------
                /\  /\  /\   /\
                ||  ||  ||   ||                   Client Framework

--------------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------



--------------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------

                ||  ||  ||   ||                   Thread Starters
                \/  \/  \/   \/
--------------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------

*/

void RemoteVariableClient_Thread_Pause(struct VariableShare * vsh)
{
  vsh->pause_client_thread=1;
}

void RemoteVariableClient_Thread_Resume(struct VariableShare * vsh)
{
  vsh->pause_client_thread=0;
}



int
StartRemoteVariableConnection(struct VariableShare * vsh)
{
   vsh->pause_client_thread=0;
   vsh->stop_client_thread=0;
   int retres = pthread_create( &vsh->client_thread, NULL,  RemoteVariableClient_Thread ,(void*) vsh);
   if (retres!=0) retres = 0; else
                  retres = 1;
   return retres;
}



void RemoteVariableServer_Thread_Pause(struct VariableShare * vsh)
{
  vsh->pause_server_thread=1;
}

void RemoteVariableServer_Thread_Resume(struct VariableShare * vsh)
{
  vsh->pause_server_thread=0;
}


int
StartRemoteVariableServer(struct VariableShare * vsh)
{
  vsh->pause_server_thread=0;
  vsh->stop_server_thread=0;
  int retres = pthread_create( &vsh->server_thread, NULL,  RemoteVariableServer_Thread ,(void*) vsh);
  if (retres!=0) retres = 0; else
                 retres = 1;
  return retres;
}
