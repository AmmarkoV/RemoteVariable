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

#include "ProtocolThreads.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "helper.h"

#include "VariableDatabase.h"
#include "NetworkFrameworkLowLevel.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/uio.h>
#include <errno.h>
#include <fcntl.h>
/*

    HERE IS THE SPACE WHERE THE SERVER THREAD AND CLIENT THREAD OF EACH VARIABLE SHARE WILL
    BE WRITTEN


*/


/*
   The server thread of each Variable Share should wait for network connections and when the network connection is present , it should
   analyze the message received , and according to the state respond with a correct response

*/


/*
   The client thread should do 3 things!

   #1 Check the state of the variables shared from our share ( master variables ) and for each one that needs refreshing in a peer share
   add a job that describes the operation

   #2 Keep a TCP connection to the master remote variable in case our node is a clone , and if the connection is broken , re-establish it

   #3 Carry out the jobs mentioned above
*/

char RVS_PROTOCOL_VERSION='A';



int Connect_Handshake(struct VariableShare * vsh,int peersock /*unsigned int *peerlock These operations dont need the peerlock mechanism since they are the only ones happening on init*/)
{
  fprintf(stderr,"Awaiting challenge\n");

  char message[RVS_MAX_RAW_HANDSHAKE_MESSAGE];
  memset (message,0,RVS_MAX_RAW_HANDSHAKE_MESSAGE);

  RecvRAWFrom(peersock,message,RVS_MAX_RAW_HANDSHAKE_MESSAGE);
  fprintf(stderr,"Received %s challenge\n",message);

  if (strcmp(message,"HELLO")!=0) { error("Error at connect handshaking : 1 "); return 0; }

  // 2ND MESSAGE SENT
  sprintf(message,"VERSION=%c\0",RVS_PROTOCOL_VERSION);
  SendRAWTo(peersock,message,strlen(message)+1);

  //THIS MESSAGE RECEIVES THE VERSION OF THE PEER
  RecvRAWFrom(peersock,message,RVS_MAX_RAW_HANDSHAKE_MESSAGE);
  if (strncmp(message,"VERSION=",8)!=0) { error("Error at connect handshaking : 2"); return 0;}
  if ((unsigned int ) message[8]!= RVS_PROTOCOL_VERSION) { error("Error at connect handshaking : Incorrect version for peer"); return 0;}


  // FOURTH MESSAGE SENT
  sprintf(message,"CON=%s\0",vsh->sharename);
  SendRAWTo(peersock,message,strlen(message)+1);


  RecvRAWFrom(peersock,message,RVS_MAX_RAW_HANDSHAKE_MESSAGE);
  if ( strncmp(message,"NO",2) == 0 )
    {
          fprintf(stderr,"Peer declined access to share %s\n",vsh->sharename);
          return 0;
    } else
  if ( strncmp(message,"OK",2) == 0 )
    {
          fprintf(stderr,"Peer accepted access to share %s\n",vsh->sharename);
//          AddMaster(vsh,"NOTSET\0",0,peersock);
          return 1;
    }

  fprintf(stderr,"Did not understand peer reply .. :S , disconnecting \n");

  return 0;
}

int Accept_Handshake(struct VariableShare * vsh,int peersock /*unsigned int *peerlock These operations dont need the peerlock mechanism since they are the only ones happening on init*/)
{

  char message[RVS_MAX_RAW_HANDSHAKE_MESSAGE];
  memset (message,0,RVS_MAX_RAW_HANDSHAKE_MESSAGE);

  // 1ST MESSAGE SENT
  strcpy(message,"HELLO\0");
  SendRAWTo(peersock,message,strlen(message)+1); //+1 for sending null termination accross


  //THIS MESSAGE RECEIVES THE VERSION OF THE PEER
  RecvRAWFrom(peersock,message,RVS_MAX_RAW_HANDSHAKE_MESSAGE);
  if (strncmp(message,"VERSION=",8)!=0) { error("Error at accept handshaking : 1"); return 0;}
  if ((unsigned int ) message[8]!= RVS_PROTOCOL_VERSION) { error("Error at accept handshaking : Incorrect version for peer"); return 0;}

  // THIRD MESSAGE SENT
  sprintf(message,"VERSION=%c\0",RVS_PROTOCOL_VERSION);
  SendRAWTo(peersock,message,strlen(message)+1); //+1 for sending null termination accross


  RecvRAWFrom(peersock,message,RVS_MAX_RAW_HANDSHAKE_MESSAGE);
  if (strncmp(message,"CON=",4)!=0) { error("Error at accept handshaking : 2"); return 0;}
  if ( strlen(message) <= 4 ) { error("Error at accepting handshake , very small share name "); return 0; }
  memmove (message,message+4,strlen(message)-3);

  fprintf(stderr,"Peer Wants Access to %s\n",message);

  if (strcmp(message,vsh->sharename)!=0)
   {
     error(" Peer asked for the wrong share ..");
     fprintf(stderr,"Requested share = `%s` , our share = `%s` \n",message,vsh->sharename);

     strcpy(message,"NO\0");
     SendRAWTo(peersock,message,3); //+1 for sending null termination accross
     return 0;
   }

  strcpy(message,"OK\0");
  SendRAWTo(peersock,message,3);  //+1 for sending null termination accross

  return 1;
}

/*
 ----------------------------------------------------------------------------------
 ----------------------------------------------------------------------------------
 ----------------------------------------------------------------------------------
*/


int InitCloneShare_Handshake(struct VariableShare * vsh)
{

  return 0;
}

int AcceptCloneShare_Handshake(struct VariableShare * vsh)
{

  return 0;
}


/*
 ----------------------------------------------------------------------------------
 ----------------------------------------------------------------------------------
 ----------------------------------------------------------------------------------
*/


int RequestVariable_Handshake(struct VariableShare * vsh,unsigned int var_id,int peersock,unsigned int *peerlock)
{
  WaitForSocketLockToClear(peersock,peerlock);
  LockSocket(peersock,peerlock);


  char message[RVS_MAX_RAW_HANDSHAKE_MESSAGE];
  memset (message,0,RVS_MAX_RAW_HANDSHAKE_MESSAGE);

  // 1ST MESSAGE SENT
  sprintf(message,"GET=%s\0",vsh->share.variables[var_id].ptr_name);
  int opres=SendRAWTo(peersock,message,strlen(message)+1); // +1 to send the null termination accross
  if (opres!=strlen(message)+1)
    {
      fprintf(stderr,"Error @RequestVariable_Handshake got response %d instead of %u",opres,(unsigned int ) strlen(message)+1);
      UnlockSocket(peersock,peerlock);
      return 0;
    }


  opres=RecvRAWFrom(peersock,message,3);
  if (strncmp(message,"OK",2)!=0)
    {
      error("Error at RequestVariable_Handshake handshaking : 1");
      UnlockSocket(peersock,peerlock);
      return 0;
    }


  fprintf(stderr,"Successfull RequestVariable_Handshake handshaking..!\n");
  opres=RecvVariableFrom(vsh,peersock,var_id);

  fprintf(stderr,"RequestVariable_Handshake exiting\n");

  UnlockSocket(peersock,peerlock);
  return opres;
}

int AcceptRequestVariable_Handshake(struct VariableShare * vsh,int peersock,unsigned int *peerlock)
{
  WaitForSocketLockToClear(peersock,peerlock);
  LockSocket(peersock,peerlock);

  char message[RVS_MAX_RAW_HANDSHAKE_MESSAGE];
  memset (message,0,RVS_MAX_RAW_HANDSHAKE_MESSAGE);

  int opres=RecvRAWFrom(peersock,message,4);

  fprintf(stderr,"Received %s request for variable\n",message);
  if (strncmp(message,"GET=",4)!=0)
   {
     error("Error at accept request for variable  handshaking : 1");
     UnlockSocket(peersock,peerlock);
     return 0;
   }

  message[0]=0; message[1]=0; message[2]=0; message[3]=0; message[4]=0;
  opres=RecvRAWFrom(peersock,message,RVS_MAX_RAW_HANDSHAKE_MESSAGE);
  if ( strlen(message) <= 4 )
   {
      error("Error at accepting request for variable handshake , very small share name ");
      UnlockSocket(peersock,peerlock);
      return 0;
   }
  if (opres!=strlen(message)+1)
   {
      fprintf(stderr,"RecRAWFrom received %u bytes , string received is now %u bytes long \n",opres,(unsigned int) strlen(message)+1 );
      return 0;
   }

 // memmove (message,message+4,strlen(message)-3);

  fprintf(stderr,"Peer Signaled that it wants a variable %s changed \n",message);

  unsigned int var_id = FindVariable_Database(vsh,message);

  if (var_id == 0)
  {
     fprintf(stderr,"Variable %s doesnt exist\n",message);
     strcpy(message,"NO\0");
     opres=SendRAWTo(peersock,message,3);
  } else
  {
     --var_id; // FindVariable offsets results by +1 to have null return value for negative find operations
     fprintf(stderr,"TODO Add check for wether it is in security/policy to send him the variable requested\n");

     strcpy(message,"OK\0");
     opres=SendRAWTo(peersock,message,3);
     opres=SendVariableTo(vsh,peersock,var_id);
     UnlockSocket(peersock,peerlock);

     return opres;
  }

  UnlockSocket(peersock,peerlock);
  return 0;
}


/*
 ----------------------------------------------------------------------------------
 ----------------------------------------------------------------------------------
 ----------------------------------------------------------------------------------
*/


int MasterSignalChange_Handshake(struct VariableShare * vsh,unsigned int var_changed,int peersock,unsigned int *peerlock)
{
  WaitForSocketLockToClear(peersock,peerlock);
  LockSocket(peersock,peerlock);

  char message[RVS_MAX_RAW_HANDSHAKE_MESSAGE];
  memset (message,0,RVS_MAX_RAW_HANDSHAKE_MESSAGE);

  // 1ST MESSAGE SENT
  sprintf(message,"SIG=%s\0",vsh->share.variables[var_changed].ptr_name);
  unsigned int length_of_full_str = 4+ vsh->share.variables[var_changed].ptr_name_length + 1;
  int opres=SendRAWTo(peersock,message,length_of_full_str);


  opres=RecvRAWFrom(peersock,message,RVS_MAX_RAW_HANDSHAKE_MESSAGE);
  if (opres<0)
   {
     fprintf(stderr,"Error at RecvFrom @ MasterSignalChange_Handshake got error code %d\n",errno);
     UnlockSocket(peersock,peerlock);
     return 0;
   } else
  if (strncmp(message,"OK",2)!=0)
   {
     error("Error at signal change handshaking : 1");
     UnlockSocket(peersock,peerlock);
     return 0;
    }
  fprintf(stderr,"Successfull Signal change handshaking..!\n");

  UnlockSocket(peersock,peerlock);
  return 1;
}

int MasterAcceptChange_Handshake(struct VariableShare * vsh,int peersock,unsigned int *peerlock)
{
  WaitForSocketLockToClear(peersock,peerlock);
  LockSocket(peersock,peerlock);

  char message[RVS_MAX_RAW_HANDSHAKE_MESSAGE];
  memset (message,0,RVS_MAX_RAW_HANDSHAKE_MESSAGE);

  RecvRAWFrom(peersock,message,4);
  fprintf(stderr,"Received %s signal\n",message);
  if (strncmp(message,"SIG=",4)!=0)
  {
    error("Error at accept change  handshaking : 1");
    UnlockSocket(peersock,peerlock);
    return 0;
  }
  RecvRAWFrom(peersock,message,RVS_MAX_RAW_HANDSHAKE_MESSAGE);

  MarkVariableAsNeedsRefresh_VariableDatabase(vsh,message,peersock);

  fprintf(stderr,"Peer Signaled that variable %s changed \n",message);
  strcpy(message,"OK\0");
  SendRAWTo(peersock,message,3);

  UnlockSocket(peersock,peerlock);
  return 1;
}


/*
  ALL THE SOCKET STUFF SHOULD BE WRITTEN IN NETWORK FRAMEWORK
  ALL THE MESSAGES SENT/RECEVEIED SHOULD BE PROCESSED HERE
*/


int ProtocolServeResponse(struct VariableShare * vsh ,int peersock,unsigned int *peerlock)
{
  //WaitForSocketLockToClear(peersock,peerlock);
  //LockSocket(peersock,peerlock);

   char peek_request[RVS_MAX_RAW_HANDSHAKE_MESSAGE];

  int rest_perms;
  rest_perms=fcntl(peersock,F_GETFL,0);
  fcntl(peersock,F_SETFL,rest_perms | O_NONBLOCK);

   memset (peek_request,0,RVS_MAX_RAW_HANDSHAKE_MESSAGE);
   int data_received = recv(peersock, (char*) peek_request,RVS_MAX_RAW_HANDSHAKE_MESSAGE, MSG_DONTWAIT |MSG_PEEK); //MSG_DONTWAIT |


  fcntl(peersock,F_SETFL,rest_perms);
   //UnlockSocket(peersock,peerlock);

   if (data_received>0)
   {
     if (strncmp(peek_request,"SIG=",4)==0)
      {
          fprintf(stderr,"ProtocolServeResponse peeked on a Signal request \n");
          return MasterAcceptChange_Handshake(vsh,peersock,peerlock);
      } else
     if (strncmp(peek_request,"GET=",4)==0)
      {
          fprintf(stderr,"ProtocolServeResponse peeked on a Get Share request \n");
          return AcceptRequestVariable_Handshake(vsh,peersock,peerlock);
      } else
      {
        fprintf(stderr,"Incoming message from background data processed of length %u received and ignored ( str = `%s` ) , flushing it down the drain\n",(unsigned int) strlen(peek_request),peek_request);
        recv(peersock, (char*) peek_request,RVS_MAX_RAW_HANDSHAKE_MESSAGE,0);
        memset (peek_request,0,RVS_MAX_RAW_HANDSHAKE_MESSAGE);
      }
   } else
   if (data_received<0)
    {
      data_received = errno;

      if (data_received == EWOULDBLOCK)
        {
          //This actually isnt an error!
          //If no messages are available at the socket and O_NONBLOCK is set on the socket's file descriptor, recv() shall fail and set errno to [EAGAIN] or [EWOULDBLOCK].
          return 1;
        }


      error("ProtocolServeResponse encountered an error while peeking on recv\n");
      fprintf(stderr," Error received is  :");

      switch (data_received)
      {
        // case EAGAIN :
         case EWOULDBLOCK :
               fprintf(stderr,"The socket is marked nonblocking and the receive operation would block, or a receive timeout had been set and the timeout expired before data was received.\n");
               break;
         case EBADF :
               fprintf(stderr,"The argument sockfd is an invalid descriptor.\n");
               break;
         case ECONNREFUSED :
               fprintf(stderr,"A remote host refused to allow the network connection (typically because it is not running the requested service).\n"); break;
         case EFAULT :
               fprintf(stderr,"The receive buffer pointer(s) point outside the process's address space. \n"); break;
         case EINTR :
               fprintf(stderr,"The receive was interrupted by delivery of a signal before any data were available; see signal(7).\n"); break;
         case EINVAL :
               fprintf(stderr,"Invalid argument passed.\n");  break;
         case ENOMEM :
               fprintf(stderr,"Could not allocate memory for recvmsg().\n"); break;
         case ENOTCONN :
               fprintf(stderr,"The socket is associated with a connection-oriented protocol and has not been connected (see connect(2) and accept(2)).\n"); break;
         case ENOTSOCK :
               fprintf(stderr,"The argument sockfd does not refer to a socket.\n"); break;
         default :
             fprintf(stderr," ERROR CODE %d \n",data_received);
             break;
      };

      return 0;
    }
   return 1;
}


