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
  fprintf(stderr,"Trying to send `%s` \n",message);
  return send(clientsock,message,length, 0);
}

int RecvRAWFrom(int clientsock,char * message,unsigned int length)
{
  int retres=recv(clientsock,message,length, 0);
  fprintf(stderr,"Received `%s` \n",message);
  return retres;
}

int RecvVariableFrom(struct VariableShare * vsh,int clientsock,unsigned int variable_id)
{
 struct NetworkRequestGeneralPacket data={0};
 unsigned int data_length=sizeof(data)-sizeof(void *);
 int data_recv= recv(clientsock, (char*) & data, sizeof (data_length), 0); // SEND START FRAME!

 if ( data.RequestType == WRITEVAR )
  {
   data.RequestType=READVAR;
   strcpy(data.name,vsh->share.variables[variable_id].ptr_name);

   data.data_size = vsh->share.variables[variable_id].size_of_ptr;


   data_recv= send(clientsock, (char*) & vsh->share.variables[variable_id].ptr,data.data_size, 0); // SEND START FRAME!
  } else
  { error("Dropping General Packet , out of context..");}

  return 0;
}

int SendVariableTo(struct VariableShare * vsh,int clientsock,unsigned int variable_id)
{
 struct NetworkRequestGeneralPacket data={0};
 unsigned int data_length=sizeof(data)-sizeof(void *);

  data.RequestType=READVAR;
  strcpy(data.name,vsh->share.variables[variable_id].ptr_name);

  data.data_size = vsh->share.variables[variable_id].size_of_ptr;

  int data_sent= send(clientsock, (char*) & data, sizeof (data_length), 0); // SEND START FRAME!

       data_sent= send(clientsock, (char*) & vsh->share.variables[variable_id].ptr,data.data_size, 0); // SEND VARIABLE!

  return 0;
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

// ________________________________________________________
// HANDLE CLIENT




int HandleClientLoop(struct VariableShare * vsh,int clientsock,struct sockaddr_in client,unsigned int clientlen)
{
   fprintf(stderr,"Client connected: %s\n", inet_ntoa(client.sin_addr));

   //First thing to do negotiate with peer about the list , passwords etc
   if (Accept_Handshake(vsh,clientsock))
     {
       debug_say("Successfully accepted connection handshake\n");
       vsh->global_state=VSS_NORMAL;
     } else
     {
       fprintf(stderr,"Could not accept handshake for RemoteVariable Share , ignoring client\n");
       close(clientsock);
       return 0;
     }


   unsigned int client_online=1;


   struct NetworkRequestGeneralPacket peek_request={0};
   int data_received = 0;

   AddPeer(vsh,inet_ntoa(client.sin_addr),0,clientsock);

   while (client_online)
    {
       data_received = recv(clientsock, (char*) & peek_request, sizeof (peek_request), MSG_PEEK);

       if (data_received < 0)
       {
        //We have a disconnect
        error("Error RecvFrom while peeking , dropping session");
       } else
       if (data_received == 0)
       {
         //Lets check our jobs..!

       } else
       if (data_received > 0)
       {
        fprintf(stderr,"Waiting For Client input\n");

       // if (HandleClient(vsh,clientsock,client,clientlen) )
          if(ProtocolServeResponse(vsh,clientsock))
          {
            fprintf(stderr,"Handled Client successfully\n");
          } else
          {
           fprintf(stderr,"Error while handling client\n");
           client_online=0;
          }
        usleep(5000);
       }
    }

    RemPeerBySock(vsh,clientsock);

    return 1;
}

void *
RemoteVariableServer_Thread(void * ptr)
{
  if ( debug_msg() ) printf("Remote Variable TCP Server thread started..\n");

  struct VariableShare *vsh;
  vsh = (struct VariableShare *) ptr;


  int serversock,clientsock;
  unsigned int serverlen = sizeof(struct sockaddr_in),clientlen = sizeof(struct sockaddr_in);
  struct sockaddr_in server;
  struct sockaddr_in client;

  serversock = socket(AF_INET, SOCK_STREAM, 0);
    if ( serversock < 0 ) { error("Opening socket"); return 0; }


  bzero(&client,clientlen);
  bzero(&server,serverlen);

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(vsh->port);

  if ( bind(serversock,(struct sockaddr *) &server,serverlen) < 0 ) { error("binding master port for RemoteVariables!"); return 0; }
  if (listen(serversock,10) < 0)  { error("Failed to listen on server socket"); return 0; }


  while (vsh->stop_server_thread==0)
  {
    debug_say("Waiting for a client");
    /* Wait for client connection */
    if ( (clientsock = accept(serversock,(struct sockaddr *) &client, &clientlen)) < 0) { error("Failed to accept client connection"); }
      else
      {
          fprintf(stderr,"Accepted new client \n");
           if (!HandleClientLoop(vsh,clientsock,client,clientlen))
            {
                fprintf(stderr,"Client failed, while handling him\n");
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


   int peersock =  vsh->master.socket_to_client;
   //First thing to do negotiate with peer about the list , passwords etc
   if (Connect_Handshake(vsh,peersock))
     {
       debug_say("Successfull connection handshake\n");
       vsh->global_state=VSS_NORMAL;
     } else
     {
       fprintf(stderr,"Could not handshake for RemoteVariable Share , stopping everything\n");
       vsh->global_state=VSS_HANDSHAKE_FAILED;
       return 0;
     }

   // ClearAllJobs(vsh); // <- This has to be added later to enforce sync issues .. we are now synced to master so clear all local jobs

//   int new_job_id=-1;
   while (vsh->stop_client_thread==0)
   {
    /* if (vsh->global_policy!=VSP_MANUAL)
      {
        debug_say("Trying to auto-generate a job");
        FindJobsFrom_VariableDatabase(vsh);
      }*/

     ProtocolServeResponse(vsh,peersock);
     debug_say_nocr(".c."); //Client Thread Waiting for a job
     usleep(1000);


   }

  return 0;
}

// ________________________________________________________
// THREAD STARTERS
// ________________________________________________________

int
StartRemoteVariableServer(struct VariableShare * vsh)
{
  vsh->stop_server_thread=0;
  int retres = pthread_create( &vsh->server_thread, NULL,  RemoteVariableServer_Thread ,(void*) vsh);
  if (retres!=0) retres = 0; else
                 retres = 1;
  return retres;
}

int
StartRemoteVariableConnection(struct VariableShare * vsh)
{
   vsh->stop_client_thread=0;
   int retres = pthread_create( &vsh->client_thread, NULL,  RemoteVariableClient_Thread ,(void*) vsh);
   if (retres!=0) retres = 0; else
                  retres = 1;
   return retres;
}
