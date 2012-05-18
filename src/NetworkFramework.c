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

void * RemoteVariableServer_Thread(void * ptr);
void * RemoteVariableClient_Thread(void * ptr);

int SendRAWTo(int clientsock,char * message,unsigned int length)
{
  return send(clientsock,message,length, 0);
}

int RecvRAWFrom(int clientsock,char * message,unsigned int length)
{
  return recv(clientsock,message,length, 0);
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

// ________________________________________________________
// HANDLE CLIENT


int HandleClient(struct VariableShare * vsh,int clientsock,struct sockaddr_in client,unsigned int clientlen)
{

      struct NetworkRequestGeneralPacket request={0};
      int data_received = recv(clientsock, (char*) & request, sizeof (request), 0);



      int packeterror = 0;
     if ( data_received < 0 )
     {
        //We have a disconnect
        error("Error RecvFrom , dropping session");
        return 0;
     }else
     {
         //RECEIVED PACKET OK
         packeterror = 0;


         /*DISASSEMBLE REQUEST @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
           RQST ID        NAME       PAYLOAD SIZE
           1 byte   |   32 bytes   |   2 bytes       ....  PACKET CONTINUES DATA WILL BE RECEIVED BY SUB FUNCTIONS ...................
              A            B             C
         */
         if (request.RequestType>=INVALID_TYPE)
         { /*INVALID PACKET*/ packeterror=1; error("Invalid Packet Type Received, Wrong Version/Incompatible Client ?"); } else
         if (request.RequestType==ERROR)
         { /* ERROR PACKET*/  packeterror=1; error("Error Packet Received");
         } else
         if (request.RequestType==OK)
         { /* OK PACKET */ packeterror=1; error("Packet Acknowledgement Received"); }

         if ( packeterror == 0 )
         { debug_say("No Packet Request Error , Passing through to VariableDatabase Check");
           if ( (request.RequestType==READVAR) || (request.RequestType==WRITEVAR) )
           {
              debug_say("Generic Packet should contain Variable Packet as a payload");
              fprintf(stderr,"Incoming Packet info \n Name : %s \n Type : %u \n Data Size : %u \n",request.name,request.RequestType,request.data_size);

              if ( request.RequestType==READVAR)
               {
                 signed int varnum = FindVariable_Database(vsh,request.name);
                 if ( CanWriteTo_VariableDatabase(vsh,varnum) == 1 )
                  {
                   RecvVariableFrom(vsh,clientsock,varnum);
                  } else
                  { fprintf(stderr,"No permission to READ \n");}
               } else
              if ( request.RequestType==WRITEVAR)
               {
                 signed int varnum = FindVariable_Database(vsh,request.name);
                 if ( CanWriteTo_VariableDatabase(vsh,varnum) == 1 )
                 {
                  SendVariableTo(vsh,clientsock,varnum);
                 } else
                 { fprintf(stderr,"No permission to WRITE \n");}
               }
           }
         }else
         {
          fprintf(stderr,"Incoming Request not passed through..\n");
         }
     }//RECEIVED PACKET OK

   return 1;
} // CLIENT HANDLE !


int HandleClientLoop(struct VariableShare * vsh,int clientsock,struct sockaddr_in client,unsigned int clientlen)
{
   fprintf(stderr,"Client connected: %s\n", inet_ntoa(client.sin_addr));

   //First thing to do negotiate with peer about the list , passwords etc
   if (Accept_Handshake(vsh,clientsock))
     {
       debug_say("Successfully accepted connection handshake\n");
     } else
     {
       fprintf(stderr,"Could not accept handshake for RemoteVariable Share , ignoring client\n");
       close(clientsock);
       return 0;
     }


   unsigned int client_online=1;

   struct NetworkRequestGeneralPacket peek_request={0};
   int data_received = 0;


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

        if (HandleClient(vsh,clientsock,client,clientlen) )
          {

          } else
          {
           client_online=0;
          }
        usleep(5000);
       }
    }

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
    if ( (clientsock = accept(serversock,(struct sockaddr *) &client, &clientlen)) < 0) { error("Failed to accept client connection"); } else
      {
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

void *
RemoteVariableClient_Thread(void * ptr)
{
  debug_say("Remote Variable TCP Server thread started..\n");
  struct VariableShare *vsh=0;
  vsh = (struct VariableShare *) ptr;
  if (vsh==0) { fprintf(stderr,"Virtual Share Parameter is damaged \n"); return 0; }




   //First thing to do negotiate with peer about the list , passwords etc
   if (Connect_Handshake(vsh,mastersock))
     {
       debug_say("Successfull connection handshake\n");
     } else
     {
       fprintf(stderr,"Could not handshake for RemoteVariable Share , stopping everything\n");
       vsh->global_state=VSS_HANDSHAKE_FAILED;
       return 0;
     }


   int new_job_id=-1;
   while (vsh->stop_client_thread==0)
   {
    /* if (vsh->global_policy!=VSP_MANUAL)
      {
        debug_say("Trying to auto-generate a job");
        FindJobsFrom_VariableDatabase(vsh);
      }*/


     debug_say_nocr(".c."); //Client Thread Waiting for a job
     usleep(100);
     /* */
     new_job_id=GetNextJobIDOperation(vsh,READFROM);
     if (new_job_id!=-1)
       {
           /* TODO CALL THE NETWORK FUNCTION
           TO DO THE TCP/UDP TRANSACTION AND THEN DECLARE THE JOB DONE
           WHERE -1 GOES I SHOULD ADD THE RESULT OF THE OPERATION
           */
           if ( -1 != -1 )
             { /* JOB DONE! */
               DoneWithJob(vsh,new_job_id);
               RemJob(vsh,new_job_id);
               new_job_id=-1;
             }
       }

     new_job_id=GetNextJobIDOperation(vsh,WRITETO);

     if (new_job_id!=-1)
       {
           /* TODO CALL THE NETWORK FUNCTION
           TO DO THE TCP/UDP TRANSACTION AND THEN DECLARE THE JOB DONE
           WHERE -1 GOES I SHOULD ADD THE RESULT OF THE OPERATION

           */
           //SendVariableTo(struct VariableShare * vsh,int clientsock,unsigned int variable_id)

           if ( -1 != -1 )
             { /* JOB DONE! */
               DoneWithJob(vsh,new_job_id);
               RemJob(vsh,new_job_id);
               new_job_id=-1;
             }
       }
   }

  return 0;
}

// ________________________________________________________
// THREAD STARTERS

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
