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
#include "RemoteVariableSupport.h"
#include "SocketAdapterToMessageTables.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/uio.h>

void * RemoteVariableServer_Thread(void * ptr);
void * RemoteVariableClient_Thread(void * ptr);


/*
     ----------------------------------- TIMERS CODE -----------------------------------
*/
long timeval_diff ( struct timeval *difference, struct timeval *end_time, struct timeval *start_time )
{
   struct timeval temp_diff;
   if(difference==0) { difference=&temp_diff; }
   difference->tv_sec =end_time->tv_sec -start_time->tv_sec ;
   difference->tv_usec=end_time->tv_usec-start_time->tv_usec;
  /* Using while instead of if below makes the code slightly more robust. */
  while(difference->tv_usec<0)
  {
    difference->tv_usec+=1000000;
    difference->tv_sec -=1;
  }

  return 1000000LL*difference->tv_sec+ difference->tv_usec;
}

long TimerStart(struct timeval *start_time)
{
  return gettimeofday(start_time,0x0);
}

long TimerEnd(struct timeval *start_time,struct timeval *end_time,struct timeval *duration_time)
{
  gettimeofday(end_time,0x0);
  return timeval_diff(duration_time,end_time,start_time);
}
/*
     ----------------------------------- TIMERS CODE -----------------------------------
*/




int GenerateNewClientLoop(struct VariableShare * vsh,int clientsock,struct sockaddr_in client,unsigned int clientlen)
{
   fprintf(stderr,"Server Thread : Client connected: %s\n", inet_ntoa(client.sin_addr));
   unsigned int peer_id=AddPeer(vsh,inet_ntoa(client.sin_addr),0,clientsock);
   if ( peer_id == 0 ) { error("Server Thread : Failed to add peer to list while @ GenerateNewClientLoop "); }
   --peer_id; // Step down one value !

  vsh->peer_list[peer_id].peer_thread=0;
  vsh->peer_list[peer_id].pause_peer_thread=0;
  vsh->peer_list[peer_id].stop_peer_thread=0;

  struct SocketAdapterToMessageTablesContext pass_to_thread;
  pass_to_thread.vsh=vsh;
  pass_to_thread.peer_id = peer_id;
  pass_to_thread.peersock = clientsock;
  pass_to_thread.keep_var_on_stack = 1;
  pass_to_thread.peersock = clientsock;
  pass_to_thread.keep_var_on_stack = 1;

  fprintf(stderr,"Server Thread : Server is ready to spawn a new dedicated thread for the client .. ");
  int retres = pthread_create(&vsh->peer_list[peer_id].peer_thread,0,SocketAdapterToMessageTable_Thread,(void*) &pass_to_thread);
  if ( retres==0 ) { while (pass_to_thread.keep_var_on_stack) { usleep(10); } } // <- Keep PeerServerContext in stack for long enough :P

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


int RemoteVariable_InitiateConnection(struct VariableShare * vsh)
{
  int sockfd;
  struct hostent *he=0;
  struct sockaddr_in their_addr;

  if ((he=gethostbyname(vsh->ip)) == 0) { fprintf(stderr,"Error getting host (%s) by name  \n",vsh->ip); return 0; } else
                                        { printf("Client-The remote host is: %s\n", vsh->ip); }

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) { error("Could not create the socket for the connection\n"); return 0; } else
                                                        { printf("Client socket ok\n"); }

    // host byte order
    their_addr.sin_family = AF_INET;
    // short, network byte order
    printf("Server-Using %s:%d...\n", vsh->ip, vsh->port);
    their_addr.sin_port = htons(vsh->port);
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    // zero the rest of the struct
    memset(&(their_addr.sin_zero), '\0', 8);

   if(connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) { error("Could not connect the created socket \n");
                                                                                           vsh->global_state=VSS_CONNECTION_FAILED;
                                                                                           return 0; } else
                                                                                         { printf("Client-The connect() is OK...\n"); }
   vsh->master.socket_to_client=sockfd;
   return 1;
}



int
StartRemoteVariableConnection(struct VariableShare * vsh)
{

  if (!RemoteVariable_InitiateConnection(vsh))
    {
      fprintf(stderr,"Client Thread : Could not Initiate Connection\n");
      vsh->global_state=VSS_CONNECTION_FAILED;
      return 0;
    }

    /*
   if (Connect_Handshake(vsh,peersock))
     {
       fprintf(stderr,"Client Thread : Successfull connection handshake , socket to master = %d \n",peersock);

       peer_id = AddPeer(vsh,vsh->ip,vsh->port,peersock); // peersock doesnt seem to be the right value to pass :S or it gets overflowed
       if ( peer_id == 0 ) { error("Client Thread : Failed to add peer to list while @ RemoteVariableClient_Thread "); }
       --peer_id; // Step down one value !

       vsh->global_state=VSS_NORMAL;
     } else
     {
       fprintf(stderr,"Client Thread : Could not handshake for RemoteVariable Share , stopping everything\n");
       vsh->global_state=VSS_HANDSHAKE_FAILED;
       return 0;
     }


   int peersock =  vsh->master.socket_to_client;
   */



   vsh->pause_client_thread=0;
   vsh->stop_client_thread=0;
   int retres = pthread_create( &vsh->client_thread, NULL,  SocketAdapterToMessageTable_Thread ,(void*) vsh);
   if (retres!=0) retres = 0; else
                  retres = 1;
   return retres;
}

