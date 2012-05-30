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

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/uio.h>
#include <errno.h>
#include <fcntl.h>

char RVS_PROTOCOL_VERSION='A';



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

int Connect_Handshake(struct VariableShare * vsh,int peersock /*unsigned int *peerlock These operations dont need the peerlock mechanism since they are the only ones happening on init*/)
{
  fprintf(stderr,"Awaiting challenge\n");

  char message[RVS_MAX_RAW_HANDSHAKE_MESSAGE];
  memset (message,0,RVS_MAX_RAW_HANDSHAKE_MESSAGE);

  RecvRAWFrom(peersock,message,RVS_MAX_RAW_HANDSHAKE_MESSAGE,0);
  fprintf(stderr,"Received %s challenge\n",message);

  if (strcmp(message,"HELLO")!=0) { error("Error at connect handshaking : 1 "); return 0; }

  // 2ND MESSAGE SENT
  SendRAWTo(peersock,"VERSION=",8);
  SendRAWTo(peersock,&RVS_PROTOCOL_VERSION,1);

  //THIS MESSAGE RECEIVES THE VERSION OF THE PEER
  memset (message,0,RVS_MAX_RAW_HANDSHAKE_MESSAGE);
  RecvRAWFrom(peersock,message,8,MSG_WAITALL);
  if (strncmp(message,"VERSION=",8)!=0) { error("Error at connect handshaking : Did not receive version string "); return 0;}



  char remote_version=0;
  RecvRAWFrom(peersock,&remote_version,1,MSG_WAITALL);
  if (remote_version!= RVS_PROTOCOL_VERSION) { fprintf(stderr,"Error at connect handshaking : Incorrect version for peer ( got %c we have %c ) ",remote_version,RVS_PROTOCOL_VERSION); return 0;}

  // FOURTH MESSAGE SENT
  memset (message,0,RVS_MAX_RAW_HANDSHAKE_MESSAGE);
  sprintf(message,"CON=%s",vsh->sharename);
  SendRAWTo(peersock,message,strlen(message)+1);

  memset (message,0,RVS_MAX_RAW_HANDSHAKE_MESSAGE);
  RecvRAWFrom(peersock,message,2,MSG_WAITALL);
  if ( strncmp(message,"NO",2) == 0 ) { fprintf(stderr,"Peer declined access to share %s\n",vsh->sharename); return 0; } else
  if ( strncmp(message,"OK",2) == 0 ) { fprintf(stderr,"Peer accepted access to share %s\n",vsh->sharename); return 1; }

  fprintf(stderr,"Did not understand peer reply .. :S , disconnecting \n");
  return 0;
}

int Accept_Handshake(struct VariableShare * vsh,int peersock /*unsigned int *peerlock These operations dont need the peerlock mechanism since they are the only ones happening on init*/)
{

  char message[RVS_MAX_RAW_HANDSHAKE_MESSAGE];
  memset (message,0,RVS_MAX_RAW_HANDSHAKE_MESSAGE);

  // 1ST MESSAGE SENT
  SendRAWTo(peersock,"HELLO",5);

  //THIS MESSAGE RECEIVES THE VERSION OF THE PEER
  memset (message,0,RVS_MAX_RAW_HANDSHAKE_MESSAGE);
  RecvRAWFrom(peersock,message,8,MSG_WAITALL);
  if (strncmp(message,"VERSION=",8)!=0) { error("Error at accept handshaking : Did not receive version string "); return 0;}

  char remote_version=0;
  RecvRAWFrom(peersock,&remote_version,1,MSG_WAITALL);
  if (remote_version!= RVS_PROTOCOL_VERSION) { fprintf(stderr,"Error at accept handshaking : Incorrect version for peer ( got %c we have %c ) ",remote_version,RVS_PROTOCOL_VERSION); return 0;}

  // THIRD MESSAGE SENT
  SendRAWTo(peersock,"VERSION=",8);
  SendRAWTo(peersock,&RVS_PROTOCOL_VERSION,1);

  message[0]=0; message[1]=0; message[2]=0; message[3]=0; message[4]=0; message[5]=0;
  RecvRAWFrom(peersock,message,4,MSG_WAITALL);
  if (strncmp(message,"CON=",4)!=0) { error("Error at accept handshaking : 2"); return 0;}

  RecvRAWFrom(peersock,message,RVS_MAX_RAW_HANDSHAKE_MESSAGE,0);
  fprintf(stderr,"Peer Wants Access to %s\n",message);
  if (strcmp(message,vsh->sharename)!=0)
   {
     fprintf(stderr,"Peer asked for the wrong share ..\n Requested share = `%s` , our share = `%s` \n",message,vsh->sharename);
     SendRAWTo(peersock,"NO",2);
     return 0;
   }

  SendRAWTo(peersock,"OK",2);
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


int RequestVariable_Handshake(struct VariableShare * vsh,unsigned int peer_id,unsigned int var_id,int peersock,unsigned int *peerlock)
{
  WaitForSocketLockToClear(peersock,peerlock);
  LockSocket(peersock,peerlock);


  struct timeval dur_time , end_time , start_time;
  char message[RVS_MAX_RAW_HANDSHAKE_MESSAGE];
  memset (message,0,RVS_MAX_RAW_HANDSHAKE_MESSAGE);



  TimerStart(&start_time);// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> PING TIMER FUNCTION
  // 1ST MESSAGE SENT
  sprintf(message,"GET=%s",vsh->share.variables[var_id].ptr_name);
  int opres=SendRAWTo(peersock,message,strlen(message)); // +1 to send the null termination accross
  if (opres!=strlen(message))
    {
      fprintf(stderr,"Error @RequestVariable_Handshake got response %d instead of %u",opres,(unsigned int ) strlen(message)+1);
      UnlockSocket(peersock,peerlock);
      return 0;
    }

  message[0]=0; message[1]=0; message[2]=0;
  opres=RecvRAWFrom(peersock,message,2,MSG_WAITALL);
  if (strncmp(message,"OK",2)!=0)
    {
      error("Error at RequestVariable_Handshake handshaking : 1");
      UnlockSocket(peersock,peerlock);
      return 0;
    }
  PeerNewPingValue(vsh,peer_id,TimerEnd(&start_time,&end_time,&dur_time)); // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> PING TIMER FUNCTION


  fprintf(stderr,"Successfull RequestVariable_Handshake handshaking..!\n");
  opres=RecvVariableFrom(vsh,peersock,var_id);
  fprintf(stderr,"RequestVariable_Handshake exiting\n");

  UnlockSocket(peersock,peerlock);
  return opres;
}

int AcceptRequestVariable_Handshake(struct VariableShare * vsh,unsigned int peer_id,int peersock,unsigned int *peerlock)
{
  WaitForSocketLockToClear(peersock,peerlock);
  LockSocket(peersock,peerlock);

  struct timeval dur_time , end_time , start_time;
  char message[RVS_MAX_RAW_HANDSHAKE_MESSAGE];
  memset (message,0,RVS_MAX_RAW_HANDSHAKE_MESSAGE);

  int opres=RecvRAWFrom(peersock,message,4,MSG_WAITALL);
  fprintf(stderr,"Received %s request for variable\n",message);
  if (strncmp(message,"GET=",4)!=0)
   {
     error("Error at accept request for variable  handshaking : 1");
     UnlockSocket(peersock,peerlock);
     return 0;
   }

  TimerStart(&start_time);// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> PING TIMER FUNCTION
  message[0]=0; message[1]=0; message[2]=0; message[3]=0; message[4]=0; message[5]=0;
  opres=RecvRAWFrom(peersock,message,RVS_MAX_RAW_HANDSHAKE_MESSAGE,0);
  fprintf(stderr,"Peer %u Signaled that it wants a variable %s changed \n",peer_id,message);

  unsigned int var_id = FindVariable_Database(vsh,message);

  opres=0;
  if (var_id == 0)
  {
     fprintf(stderr,"Variable %s doesnt exist\n",message);
     opres=SendRAWTo(peersock,"NO",2);
     opres=0;
  } else
  {
     --var_id; // FindVariable offsets results by +1 to have null return value for negative find operations
     fprintf(stderr,"TODO Add check for wether it is in security/policy to send him the variable requested\n");

     opres=SendRAWTo(peersock,"OK",2);
     opres=SendVariableTo(vsh,peersock,var_id);
     opres=1;
  }

  PeerNewPingValue(vsh,peer_id,TimerEnd(&start_time,&end_time,&dur_time)); // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> PING TIMER FUNCTION
  UnlockSocket(peersock,peerlock);
  return opres;
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

  char message[RVS_MAX_RAW_HANDSHAKE_MESSAGE]={0};

  // 1ST MESSAGE SENT
  int opres=SendRAWTo(peersock,"SIG=",4);
  opres=SendRAWTo(peersock,vsh->share.variables[var_changed].ptr_name,vsh->share.variables[var_changed].ptr_name_length);


  memset (message,0,RVS_MAX_RAW_HANDSHAKE_MESSAGE);
  opres=RecvRAWFrom(peersock,message,2,MSG_WAITALL);
  if (opres<0)
   {
     fprintf(stderr,"Error at RecvFrom @ MasterSignalChange_Handshake got error code %d\n",errno);
     UnlockSocket(peersock,peerlock);
     return 0;
   } else
  if (strncmp(message,"OK",2)!=0)
   {
     fprintf(stderr,"Error at signal change handshaking while waiting for OK - instead got `%s`\n",message);
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
  RecvRAWFrom(peersock,message,4,MSG_WAITALL);
  fprintf(stderr,"Received %s signal\n",message);
  if (strncmp(message,"SIG=",4)!=0)
  {
    error("Error at accept change  handshaking : 1");
    UnlockSocket(peersock,peerlock);
    return 0;
  }

  memset (message,0,RVS_MAX_RAW_HANDSHAKE_MESSAGE);
  RecvRAWFrom(peersock,message,RVS_MAX_RAW_HANDSHAKE_MESSAGE,0);
  if ( MarkVariableAsNeedsRefresh_VariableDatabase(vsh,message,peersock) )
   {
     fprintf(stderr,"Peer Signaled that variable %s changed \n",message);
     SendRAWTo(peersock,"OK",2);
   } else
   {
     fprintf(stderr,"Error Peer Signaled that a variable that does not exist (%s) changed \n",message);
     SendRAWTo(peersock,"NO",2);
   }



  UnlockSocket(peersock,peerlock);
  return 1;
}


/*
  ALL THE SOCKET STUFF SHOULD BE WRITTEN IN NETWORK FRAMEWORK
  ALL THE MESSAGES SENT/RECEVEIED SHOULD BE PROCESSED HERE
*/


int ProtocolServeResponse(struct VariableShare * vsh ,unsigned int peer_id,int peersock,unsigned int *peerlock)
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
          return AcceptRequestVariable_Handshake(vsh,peer_id,peersock,peerlock);
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


