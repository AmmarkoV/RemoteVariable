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
#include "NetworkFramework.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/uio.h>
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






int Connect_Handshake(struct VariableShare * vsh,int peersock)
{
  fprintf(stderr,"Awaiting challenge\n");

  char message[RVS_MAX_RAW_HANDSHAKE_MESSAGE]={0};
  RecvRAWFrom(peersock,message,RVS_MAX_RAW_HANDSHAKE_MESSAGE);
  fprintf(stderr,"Received %s challenge\n",message);

  if (strcmp(message,"HELLO")!=0) { error("Error at connect handshaking : 1 "); return 0; }

  // 2ND MESSAGE SENT
  sprintf(message,"VERSION=%c\0",RVS_PROTOCOL_VERSION);
  SendRAWTo(peersock,message,strlen(message));

  //THIS MESSAGE RECEIVES THE VERSION OF THE PEER
  RecvRAWFrom(peersock,message,RVS_MAX_RAW_HANDSHAKE_MESSAGE);
  if (strncmp(message,"VERSION=",8)!=0) { error("Error at connect handshaking : 2"); return 0;}
  if ((unsigned int ) message[8]!= RVS_PROTOCOL_VERSION) { error("Error at connect handshaking : Incorrect version for peer"); return 0;}


  // FOURTH MESSAGE SENT
  sprintf(message,"GET=%s\0",vsh->sharename);
  SendRAWTo(peersock,message,strlen(message));


  RecvRAWFrom(peersock,message,RVS_MAX_RAW_HANDSHAKE_MESSAGE);
  if ( strncmp(message,"NO",2) == 0 )
    {
          fprintf(stderr,"Peer declined access to share %s\n",vsh->sharename);
          return 0;
    } else
  if ( strncmp(message,"OK",2) == 0 )
    {
          fprintf(stderr,"Peer accepted access to share %s\n",vsh->sharename);
          return 1;
    }

  fprintf(stderr,"Did not understand peer reply .. :S , disconnecting \n");

  return 0;
}

int Accept_Handshake(struct VariableShare * vsh,int peersock)
{

  char message[RVS_MAX_RAW_HANDSHAKE_MESSAGE]={0};
  // 1ST MESSAGE SENT
  strcpy(message,"HELLO\0");
  SendRAWTo(peersock,message,strlen(message));


  //THIS MESSAGE RECEIVES THE VERSION OF THE PEER
  RecvRAWFrom(peersock,message,RVS_MAX_RAW_HANDSHAKE_MESSAGE);
  if (strncmp(message,"VERSION=",8)!=0) { error("Error at accept handshaking : 1"); return 0;}
  if ((unsigned int ) message[8]!= RVS_PROTOCOL_VERSION) { error("Error at accept handshaking : Incorrect version for peer"); return 0;}

  // THIRD MESSAGE SENT
  sprintf(message,"VERSION=%c\0",RVS_PROTOCOL_VERSION);
  SendRAWTo(peersock,message,strlen(message));


  RecvRAWFrom(peersock,message,RVS_MAX_RAW_HANDSHAKE_MESSAGE);
  if (strncmp(message,"GET=",4)!=0) { error("Error at accept handshaking : 2"); return 0;}
  if ( strlen(message) <= 4 ) { error("Error at accepting handshake , very small share name "); return 0; }
  memmove (message,message+4,strlen(message)-3);

  fprintf(stderr,"Peer Wants Access to %s\n",message);

  if (strcmp(message,vsh->sharename)!=0)
   {
     error(" Peer asked for the wrong share ..");
     fprintf(stderr,"Requested share = `%s` , our share = `%s` \n",message,vsh->sharename);

     strcpy(message,"NO\0");
     SendRAWTo(peersock,message,strlen(message));
     return 0;
   }

  strcpy(message,"OK\0");
  SendRAWTo(peersock,message,strlen(message));

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


int RequestVariable_Handshake(struct VariableShare * vsh,unsigned int var_id,int peersock)
{
  char message[RVS_MAX_RAW_HANDSHAKE_MESSAGE]={0};
  // 1ST MESSAGE SENT
  sprintf(message,"GET=%s\0",vsh->share.variables[var_id].ptr_name);
  SendRAWTo(peersock,message,strlen(message));

  RecvRAWFrom(peersock,message,3);
  if (strncmp(message,"OK",2)!=0) { error("Error at RequestVariable_Handshake handshaking : 1"); return 0;}
  fprintf(stderr,"Successfull RequestVariable_Handshake handshaking..!\n");
  return RecvVariableFrom(vsh,peersock,var_id);

  return 0;
}

int AcceptRequestVariable_Handshake(struct VariableShare * vsh,int peersock)
{
  char message[RVS_MAX_RAW_HANDSHAKE_MESSAGE]={0};
  RecvRAWFrom(peersock,message,RVS_MAX_RAW_HANDSHAKE_MESSAGE);
  fprintf(stderr,"Received %s request for variable\n",message);
  if (strncmp(message,"GET=",4)!=0) { error("Error at accept request for variable  handshaking : 1"); return 0;}
  if ( strlen(message) <= 4 ) { error("Error at accepting request for variable handshake , very small share name "); return 0; }
  memmove (message,message+4,strlen(message)-3);

  fprintf(stderr,"Peer Signaled that it wants a variable %s changed \n",message);

  unsigned int var_id = FindVariable_Database(vsh,message);

  if (var_id == 0)
  {
     fprintf(stderr,"Variable %s doesnt exist\n",message);
     strcpy(message,"NO\0");
     SendRAWTo(peersock,message,3);
  } else
  {
     --var_id; // FindVariable offsets results by +1 to have null return value for negative find operations
     fprintf(stderr,"TODO Add check for weather it is in security/policy to send him the variable requested\n");

     strcpy(message,"OK\0");
     SendRAWTo(peersock,message,3);
     return SendVariableTo(vsh,peersock,var_id);
  }


  return 0;
}


/*
 ----------------------------------------------------------------------------------
 ----------------------------------------------------------------------------------
 ----------------------------------------------------------------------------------
*/


int MasterSignalChange_Handshake(struct VariableShare * vsh,unsigned int var_changed,int peersock)
{
  char message[RVS_MAX_RAW_HANDSHAKE_MESSAGE]={0};
  // 1ST MESSAGE SENT
  sprintf(message,"SIG=%s\0",vsh->share.variables[var_changed].ptr_name);
  SendRAWTo(peersock,message,strlen(message));

  RecvRAWFrom(peersock,message,RVS_MAX_RAW_HANDSHAKE_MESSAGE);
  if (strncmp(message,"OK",2)!=0) { error("Error at signal change handshaking : 1"); return 0;}
  fprintf(stderr,"Successfull Signal change handshaking..!\n");


  return 1;
}

int MasterAcceptChange_Handshake(struct VariableShare * vsh,int peersock)
{
  char message[RVS_MAX_RAW_HANDSHAKE_MESSAGE]={0};
  RecvRAWFrom(peersock,message,RVS_MAX_RAW_HANDSHAKE_MESSAGE);
  fprintf(stderr,"Received %s signal\n",message);
  if (strncmp(message,"SIG=",4)!=0) { error("Error at accept change  handshaking : 1"); return 0;}
  if ( strlen(message) <= 4 ) { error("Error at accepting change  handshake , very small share name "); return 0; }
  memmove (message,message+4,strlen(message)-3);

  MarkVariableAsNeedsRefresh_VariableDatabase(vsh,message,peersock);

  fprintf(stderr,"Peer Signaled that variable %s changed \n",message);
  strcpy(message,"OK\0");
  SendRAWTo(peersock,message,3);


  return 1;
}


/*

  ALL THE SOCKET STUFF SHOULD BE WRITTEN IN NETWORK FRAMEWORK

  ALL THE MESSAGES SENT/RECEVEIED SHOULD BE PROCESSED HERE

*/


int ProtocolServeResponse(struct VariableShare * vsh , unsigned int peersock)
{
   char peek_request[RVS_MAX_RAW_HANDSHAKE_MESSAGE]={0};
   int data_received = recv(peersock, (char*) peek_request,RVS_MAX_RAW_HANDSHAKE_MESSAGE, MSG_PEEK);
   if (data_received>0)
   {
     if (strncmp(peek_request,"SIG=",4)==0)
      {
          fprintf(stderr,"ProtocolServeResponse peeked on a Signal request \n");
          return MasterAcceptChange_Handshake(vsh,peersock);
      } else
     if (strncmp(peek_request,"GET=",4)==0)
      {
          fprintf(stderr,"ProtocolServeResponse peeked on a Get Share request \n");
          return AcceptRequestVariable_Handshake(vsh,peersock);
      } else
      {
        fprintf(stderr,"Uncatched incoming message = %s \n",peek_request);
      }
   }
   return 0;
}


