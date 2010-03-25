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

#include "NetworkFramework.h"
#include "helper.h"

void * RemoteVariableServer_Thread(void * ptr);
unsigned int stop_server_thread=0;
unsigned int stop_client_thread=0;
pthread_t server_thread;

/*
  TODO
  SPAWN A NEW LISTEN THEAD WHEN
  Start_VariableSharing is called
*/



/*
  TODO
  SPAWN A NEW CONNECTION THEAD WHEN
  ConnectToRemote_VariableSharing is called
*/


/*
  TODO
  KILL EVERYTHING WHEN
  Stop_VariableSharing is called
*/

/*
  TODO INTERCEPT REQUESTS FOR VARIABLES
  PASS THEM TO VARIABLE DATABASE FOR CHECK
  AND IF THEY PASS THE CHECK send THEM
*/

int RecvVariableFrom(struct VariableShare * vsh,int clientsock,unsigned int variable_id)
{
 struct NetworkRequestGeneralPacket data={0};
 unsigned int data_length=sizeof(data)-sizeof(void *);
 int data_recv= recv(clientsock, (char*) & data, sizeof (data_length), 0); // SEND START FRAME!

 if ( data.RequestType == WRITEVAR )
  {
   data.RequestType=READVAR;
   data.name_size=vsh->share.variables[variable_id].size_of_ptr_name;
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
  data.name_size=vsh->share.variables[variable_id].size_of_ptr_name;
  strcpy(data.name,vsh->share.variables[variable_id].ptr_name);

  data.data_size = vsh->share.variables[variable_id].size_of_ptr;

  int data_sent= send(clientsock, (char*) & data, sizeof (data_length), 0); // SEND START FRAME!

  data_sent= send(clientsock, (char*) & vsh->share.variables[variable_id].ptr,data.data_size, 0); // SEND START FRAME!

  return 0;
}

void HandleClient(struct VariableShare * vsh,int clientsock,struct sockaddr_in client,unsigned int clientlen)
{
      printf( "Client connected: %s\n", inet_ntoa(client.sin_addr));

      struct NetworkRequestGeneralPacket request={0};
      int data_received = recv(clientsock, (char*) & request, sizeof (request), 0);



      int packeterror = 0;
     if ( data_received < 0 ) error("Error RecvFrom , dropping session"); else
     { //RECEIVED PACKET OK
         packeterror = 0;
         /*DISASSEMBLE REQUEST @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
           RQST ID   PAYLOAD SIZE   PAYLOAD DATA
           1 byte   |   2 bytes   |   n bytes
              A            B            C
         */
         if (request.RequestType>=INVALID_TYPE) { packeterror=1; } else
         if (request.RequestType==ERROR)
         { // ERROR PACKET
           packeterror=1;
           error("Error Packet Received");
         }


         if ( packeterror == 0 )
         { debug_say("No Packet Request Error , Passing through to VariableDatabase Check");
           if ( (request.RequestType==READVAR) || (request.RequestType==WRITEVAR) )
           {
              debug_say("Generic Packet should contain Variable Packet as a payload");
              // LOOOOOOOOOOOOOOOOOOOOOOTS OF THINGS TODO :P
           }
         }else
         {
          printf("Incoming Request not passed through..\n");
         // fflush(stdout);
          //write(1, request.data, n - 2);
         }


     }//RECEIVED PACKET OK

} // CLIENT HANDLE !


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

  if ( bind(serversock,(struct sockaddr *) &server,serverlen) < 0 ) { error("binding master port for atftp!"); return 0; }
  if (listen(serversock,10) < 0)  { error("Failed to listen on server socket"); return 0; }


  while (stop_server_thread==0)
  {
    debug_say("Waiting for a client");
    /* Wait for client connection */
    if ( (clientsock = accept(serversock,(struct sockaddr *) &client, &clientlen)) < 0) { error("Failed to accept client connection"); } else
      {
          HandleClient(vsh,clientsock,client,clientlen);
      }
 }
  return;

}

int
StartRemoteVariableServer(struct VariableShare * vsh)
{
   stop_server_thread=0;
   pthread_create( &server_thread, NULL,  RemoteVariableServer_Thread ,(void*) vsh);
}

int
StartRemoteVariableConnection(struct VariableShare * vsh)
{
   stop_client_thread=0;
   pthread_create( &server_thread, NULL,  RemoteVariableServer_Thread ,(void*) vsh);
}
