#include "Protocol.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "VariableDatabase.h"
#include "NetworkFramework.h"
#include "Peers.h"
#include "helper.h"
#include "Protocol.h"
#include "MessageTables.h"
#include "SocketAdapterToMessageTables.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/uio.h>
#include <errno.h>
#include <sys/time.h>

char RVS_PROTOCOL_VERSION='B';

/* ----------------------------------- TIMERS CODE ----------------------------------- */
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
/*   ----------------------------------- TIMERS CODE ----------------------------------- */



int SendStringTo(int clientsock,char * message,unsigned int length)
{
  fprintf(stderr,"SendStringTo Trying to send length %u to peer socket %d \n",length,clientsock);
  int opres1 = send(clientsock,&length,sizeof(length),0);
  if ( opres1 < 0 ) { fprintf(stderr,"Error %u while SendStringTo the length packet\n",errno); return opres1; } else
  if ( opres1 < sizeof(length) ) { fprintf(stderr,"Did not send the whole length packet ( only %u bytes ) while SendStringTo\n",opres1); return opres1; }

  fprintf(stderr,"SendStringTo Trying to send `%s` to peer socket %d \n",message,clientsock);
  int opres2 = send(clientsock,message,length, 0);
  if ( opres2 < 0 ) { fprintf(stderr,"Error %u while SendStringTo the length packet\n",errno); return opres2; } else
  if ( opres2 < sizeof(length) ) { fprintf(stderr,"Did not send the whole length packet ( only %u bytes ) while SendStringTo\n",opres2); return opres2; }

  return opres2;
}

int RecvStringFrom(int clientsock,char * message,unsigned int length)
{
  fprintf(stderr,"Trying RecvStringFrom\n");
  memset (message,0,length);

  unsigned int incoming_string_length=0;
  int opres1=recv(clientsock,(char*) &incoming_string_length,sizeof(incoming_string_length),MSG_WAITALL);
  if ( opres1 < 0 ) { fprintf(stderr,"Error %u while RecvStringFrom the length packet\n",errno); return opres1; } else
  if ( opres1 < sizeof(length) ) { fprintf(stderr,"Did not receive the whole length packet ( only %u bytes ) while RecvStringFrom\n",opres1); return opres1; }
  fprintf(stderr,"Incoming string is %u bytes long ( from peer socket %d ) \n",incoming_string_length,clientsock);

  if (length<incoming_string_length)
   {
     fprintf(stderr,"Incoming string will overflow .. clipping it to our buffer size ( %u ).. \n",length);
     incoming_string_length=length;
   }

  int opres2=recv(clientsock,message,incoming_string_length,MSG_WAITALL);
  if ( opres2 < 0 ) { fprintf(stderr,"Error %u while RecvStringFrom the length packet\n",errno); return opres2; } else
  if ( opres2 < incoming_string_length ) { fprintf(stderr,"Did not receive the whole length packet ( only %u bytes ) while RecvStringFrom\n",opres2); return opres2; }
  fprintf(stderr,"Incoming string is `%s` ( from peer socket %d ) \n",message,clientsock);

  return opres2;
}

/*
    ------------------------------------------------------
                DIRECT SEND/RECV HANDSHAKES
    ------------------------------------------------------
*/
int Start_Version_Handshake(struct VariableShare * vsh,int peersock)
{
  fprintf(stderr,"Connect_Handshake , Awaiting challenge\n");

  char message[RVS_MAX_RAW_HANDSHAKE_MESSAGE]={0};

  memset (message,0,6);
  int opres=recv(peersock,message,4,MSG_WAITALL);
  if (opres<0) { fprintf(stderr,"Error %u while Start_Version_Handshake while recv!! \n",errno); return 0; }
  fprintf(stderr,"recv `%s`\n",message);
  if (strncmp(message,"HI!!",4)!=0) { error("Error at connect handshaking : Did not receive HELLO "); return 0; }

  // 2ND MESSAGE SENT
  opres=send(peersock,"VER=",4,MSG_WAITALL);
  opres=send(peersock,&RVS_PROTOCOL_VERSION,1,MSG_WAITALL);
  fprintf(stderr,"send VER`%c`\n",RVS_PROTOCOL_VERSION);

  //THIS MESSAGE RECEIVES THE VERSION OF THE PEER
  memset (message,0,6);
  opres=recv(peersock,message,5,MSG_WAITALL);
  fprintf(stderr,"recv `%s`\n",message);
  if (strncmp(message,"VER=",4)!=0) { fprintf(stderr,"Error at connect handshaking : Did not receive version string ( received %s ) \n",message); return 0;} else
  if (message[4]!=RVS_PROTOCOL_VERSION ) { fprintf(stderr,"Error at connect handshaking : Incorrect version for peer ( got %c we have %c ) ",message[4],RVS_PROTOCOL_VERSION); return 0;}


  // FOURTH MESSAGE SENT
  opres=send(peersock,"CON=",4,MSG_WAITALL);
  SendStringTo(peersock,vsh->sharename,strlen(vsh->sharename));

  memset (message,0,3);
  opres=recv(peersock,message,2,MSG_WAITALL);
  if ( strncmp(message,"NO",2) == 0 ) { fprintf(stderr,"Peer declined access to share %s\n",vsh->sharename); return 0; } else
  if ( strncmp(message,"OK",2) == 0 ) { fprintf(stderr,"Peer accepted access to share %s\n",vsh->sharename); return 1; }

  fprintf(stderr,"Did not understand peer reply .. :S , disconnecting \n");
  return 0;
}

int Accept_Version_Handshake(struct VariableShare * vsh,int peersock)
{
  fprintf(stderr,"Accept_Version_Handshake started \n");
  char message[RVS_MAX_RAW_HANDSHAKE_MESSAGE]={0};
  memset (message,0,RVS_MAX_RAW_HANDSHAKE_MESSAGE);

  // 1ST MESSAGE SENT
  int opres=send(peersock,"HI!!",4,MSG_WAITALL);
  if (opres<0) { fprintf(stderr,"Error %u while Accept_Version_Handshake sending HI!! \n",errno); return 0; }
  fprintf(stderr,"send `HI!!`\n");

  //THIS MESSAGE RECEIVES THE VERSION OF THE PEER
  memset (message,0,6);
  opres=recv(peersock,message,5,MSG_WAITALL);
  if (opres<0) { fprintf(stderr,"Error %u while Accept_Version_Handshake recv !! \n",errno); return 0; }
  fprintf(stderr,"recv `%s`\n",message);
  if (strncmp(message,"VER=",4)!=0) { error("Error at connect handshaking : Did not receive version string "); return 0;} else
  if (message[4]!=RVS_PROTOCOL_VERSION ) { fprintf(stderr,"Error at connect handshaking : Incorrect version for peer ( got %c we have %c ) ",message[4],RVS_PROTOCOL_VERSION); return 0;}

  // THIRD MESSAGE SENT
  opres=send(peersock,"VER=",4,MSG_WAITALL);
  opres=send(peersock,&RVS_PROTOCOL_VERSION,1,MSG_WAITALL);
  fprintf(stderr,"send VER=%c\n",RVS_PROTOCOL_VERSION);

  message[0]=0; message[1]=0; message[2]=0; message[3]=0; message[4]=0; message[5]=0;
  opres=recv(peersock,message,4,MSG_WAITALL);
  fprintf(stderr,"recv `%s`\n",message);
  if (strncmp(message,"CON=",4)!=0) { error("Error at accept handshaking : 2"); return 0;}

  RecvStringFrom(peersock,message,RVS_MAX_RAW_HANDSHAKE_MESSAGE);
  fprintf(stderr,"Peer Wants Access to %s\n",message);
  if (strcmp(message,vsh->sharename)!=0)
   {
     fprintf(stderr,"Peer asked for the wrong share ..\n Requested share = `%s` , our share = `%s` \n",message,vsh->sharename);
     opres=send(peersock,"NO",2,MSG_WAITALL);
     return 0;
   }

  opres=send(peersock,"OK",2,MSG_WAITALL);
  fprintf(stderr,"Accept_Version_Handshake completed \n");
  return 1;
}


/*
    ------------------------------------------------------
                DIRECT SEND/RECV HANDSHAKES
    ------------------------------------------------------

    ------------------------------------------------------
    ------------------------------------------------------
    ------------------------------------------------------
    ------------------------------------------------------

    ------------------------------------------------------
    ------------------------------------------------------
    ------------------------------------------------------
    ------------------------------------------------------

    ------------------------------------------------------
                HANDSHAKES THROUGH MESSAGE TABLES
    ------------------------------------------------------
*/

int Request_WriteVariable(struct VariableShare * vsh,unsigned int peer_id,unsigned int var_id,int peersock,unsigned int groupid ,  unsigned int * protocol_progress , unsigned int * last_protocol_mid)
{
  return 0;
}

int AcceptRequest_WriteVariable(struct VariableShare * vsh,unsigned int peer_id,struct MessageTable * mt,unsigned int mt_id,int peersock, unsigned int groupid , unsigned int * protocol_progress , unsigned int * last_protocol_mid)
{
  return 0;
}

/*

    ------------------------------------------------------  ------------------------------------------------------
    ------------------------------------------------------  ------------------------------------------------------
    ------------------------------------------------------  ------------------------------------------------------
    ------------------------------------------------------  ------------------------------------------------------
    ------------------------------------------------------  ------------------------------------------------------
    ------------------------------------------------------  ------------------------------------------------------
    ------------------------------------------------------  ------------------------------------------------------

*/

struct failint NewProtocolRequest_Send(struct VariableShare * vsh,unsigned int peer_id,unsigned int var_id,unsigned char OPTYPE,unsigned int free_malloc_at_disposal,void * payload,unsigned int payload_size)
{
  struct failint retres={0};

  struct PacketHeader header={0};
  header.incremental_value=vsh->peer_list[peer_id].incremental_value;
  header.operation_type=OPTYPE;
  header.var_id=var_id;
  header.payload_size=payload_size;


  if (protocol_msg())
  {
    fprintf(stderr,"NewProtocolRequest_Send called for peer_id %u , var_id %u , incremental value %u  type ",peer_id,var_id,header.incremental_value);
    PrintMessageType(&header);
    fprintf(stderr,"\n");
  }

  //struct failint AddToMessageTable(struct MessageTable * mt,unsigned int incoming,unsigned int free_malloc_at_disposal,struct PacketHeader * header,void * payload,unsigned int msg_timer)
  retres=AddMessage(&vsh->peer_list[peer_id].messages,0,free_malloc_at_disposal,&header,payload,vsh->central_timer);


  return retres;
}



/*!
    ------------------------------------------------------
                GENERIC MESSAGE TABLE FUNCTIONS
    ------------------------------------------------------

    ------------------------------------------------------
    ------------------------------------------------------
    ------------------------------------------------------
    ------------------------------------------------------

    ------------------------------------------------------
    ------------------------------------------------------
    ------------------------------------------------------
    ------------------------------------------------------

    ------------------------------------------------------
                HANDSHAKES THROUGH MESSAGE TABLES
    ------------------------------------------------------
!*/


int Request_ReadVariable(struct VariableShare * vsh,unsigned int peer_id,unsigned int var_id,int peersock , unsigned int groupid , unsigned int * protocol_progress , unsigned int * last_protocol_mid)
{
  // When we initiate a request we increment our inc_value to signal a new group of messages
  ++vsh->peer_list[peer_id].incremental_value;


  if (*protocol_progress==0)
  { //We send a new READFROM request
    struct failint msg1=NewProtocolRequest_Send(vsh,peer_id,var_id,READFROM,0,0,0);
    //msg1.value has the value of the new outgoing messagetable entry
    if (msg1.failed) { fprintf(stderr,"ERROR : Could not add Request_Variable to local MessageTable STEP 1\n"); return 0;  }
    *protocol_progress=1;
    *last_protocol_mid=msg1.value;
  }

  if (*protocol_progress==1)
  {
  //We wait for the success indicator recv and subsequent pass to our message table
  unsigned char optypeForMSG3=SIGNALMSGSUCCESS;
  struct failint msg2=WaitForVariableAndCopyItAtMessageTableItem(&vsh->peer_list[peer_id].messages,*last_protocol_mid,vsh,var_id,0);
  if ( msg2.failed==1 ) { return 0;  /*Only change message type the rest remains the same*/ } else
  if ( msg2.failed==2 ) { optypeForMSG3=SIGNALMSGFAILURE; /*Only change message type the rest remains the same*/ } else
                        { optypeForMSG3=SIGNALMSGSUCCESS; /*Only change message type the rest remains the same*/ }
  //We remove payloads ,etc and sent a positive or negative confirmation to end the protocol handshake
  struct failint msg3=NewProtocolRequest_Send(vsh,peer_id,var_id,optypeForMSG3,0,0,0);
  if (msg3.failed) { fprintf(stderr,"Could not add Request_Variable to local MessageTable STEP 2\n"); return 0;  }
  *protocol_progress=2;
  *last_protocol_mid=msg3.value;
  }

  if (*protocol_progress==2)
  {
    //Wait for message to be sent
    WaitForMessageTableItemToBeSent(&vsh->peer_list[peer_id].messages.table[*last_protocol_mid]);


    //The messages sent and received can now be removed..!
    SetAllMessagesOfGroup_Flag_ForRemoval(&vsh->peer_list[peer_id].messages,groupid);

    *protocol_progress=3;
    return 1;
  }


  if (*protocol_progress>=3)
  { fprintf(stderr,"Request_ReadVariable protocol progress , ended..!\n"); }


  return 0;
}

/*!
    ------------------------------------------------------  ------------------------------------------------------
    ------------------------------------------------------  ------------------------------------------------------
!*/
int AcceptRequest_ReadVariable(struct VariableShare * vsh,unsigned int peer_id,struct MessageTable * mt,unsigned int mt_id,int peersock, unsigned int groupid , unsigned int * protocol_progress , unsigned int * last_protocol_mid)
{
  // We have accepted a new Message Table entry which contains a Request_ReadVariable so we will try to accept it..
  // First make a local copy of the header ..

  if (*protocol_progress==0)
  {
  // Secondly this incremental_value is now the last for this client , if we make a new request it should have a different inc_value than this..
  if (mt->table[mt_id].header.incremental_value>vsh->peer_list[peer_id].incremental_value)
   { vsh->peer_list[peer_id].incremental_value = mt->table[mt_id].header.incremental_value; }
   else { fprintf(stderr,"Header accepted has a smaller inc value ( %u ) than our current one ( %u ) , ignoring it ..\n",mt->table[mt_id].header.incremental_value,vsh->peer_list[peer_id].incremental_value); }

  unsigned int var_id = mt->table[mt_id].header.var_id;
  //We received a READFROM request ( otherwise this function wouldnt have been triggered so lets respond to it )
  /* TODO HERE , HANDLE PERMISSIONS ETC*/
  //fprintf(stderr,"TODO HERE , HANDLE PERMISSIONS ETC\n");
  if (!VariableIdExists(vsh,var_id))
   {
       fprintf(stderr,"ERROR : AcceptRequest_ReadVariable called for non existing var_id %u \n",var_id);
       //TODO RESPOND HERE WITH MSG_FAILURE
       return 0;
   }

  // Sending RESP_WRITETO back to the peer along with the payload..!
  struct failint msg1=NewProtocolRequest_Send(vsh,peer_id,var_id,RESP_WRITETO,0,vsh->share.variables[var_id].ptr,vsh->share.variables[var_id].size_of_ptr);
  if (msg1.failed) { fprintf(stderr,"Could not add AcceptRequest_ReadVariable to local MessageTable\n"); return 0; }
  *protocol_progress=1;
  *last_protocol_mid=msg1.value;
  }

  if (*protocol_progress==1)
  {
    struct failint msg2=WaitForSuccessIndicatorAtMessageTableItem(&vsh->peer_list[peer_id].messages,*last_protocol_mid,0);
    if (msg2.failed==1) { return 0; } //2 means negative response , we let it pass through

  //The messages sent and received can now be removed..!
  SetAllMessagesOfGroup_Flag_ForRemoval(mt,groupid);
  *protocol_progress=2;
  return 1;
  }


  if (*protocol_progress>=2)
  { fprintf(stderr,"AcceptRequest_ReadVariable protocol progress , ended..!\n"); }

 return 0;
}


/*!

    ------------------------------------------------------  ------------------------------------------------------
    ------------------------------------------------------  ------------------------------------------------------
    ------------------------------------------------------  ------------------------------------------------------
    ------------------------------------------------------  ------------------------------------------------------
    ------------------------------------------------------  ------------------------------------------------------
    ------------------------------------------------------  ------------------------------------------------------
    ------------------------------------------------------  ------------------------------------------------------

!*/

int Request_SignalChangeVariable(struct VariableShare * vsh,unsigned int peer_id,unsigned int var_id,int peersock, unsigned int groupid , unsigned int * protocol_progress , unsigned int * last_protocol_mid)
{
  // When we initiate a request we increment our inc_value to signal a new group of messages
  ++vsh->peer_list[peer_id].incremental_value;

  if (*protocol_progress==0)
  {
    // We also want a new header to go with our message!
    struct failint msg1=NewProtocolRequest_Send(vsh,peer_id,var_id,SIGNALCHANGED,0,0,0);
    if (msg1.failed) { fprintf(stderr,"Could not add SignalChange_Variable to local MessageTable\n"); return 0; }
    *protocol_progress=1;
    *last_protocol_mid=msg1.value;
  }


  if (*protocol_progress==1)
  {
   //We wait for the success indicator recv and subsequent pass to our message table
   struct failint msg2=WaitForSuccessIndicatorAtMessageTableItem(&vsh->peer_list[peer_id].messages,*last_protocol_mid,0);
   if (msg2.failed==1) { return 0; } //2 means negative response , we let it pass through

   //The messages sent and received can now be removed..!
   SetAllMessagesOfGroup_Flag_ForRemoval(&vsh->peer_list[peer_id].messages,groupid);
   *protocol_progress=2;
   return 1;
  }


  if (*protocol_progress>=2)
  { fprintf(stderr,"Request_SignalChangeVariable protocol progress , ended..!\n"); }

  return 0;
}

int AcceptRequest_SignalChangeVariable(struct VariableShare * vsh,unsigned int peer_id,struct MessageTable * mt,unsigned int mt_id,int peersock, unsigned int groupid, unsigned int * protocol_progress , unsigned int * last_protocol_mid)
{
  // We have accepted a new Message Table entry which contains a Request_ReadVariable so we will try to accept it..
  // First make a local copy of the header ..

  if (*protocol_progress==0)
  {
  // Secondly this incremental_value is now the last for this client , if we make a new request it should have a different inc_value than this..
  if (mt->table[mt_id].header.incremental_value>vsh->peer_list[peer_id].incremental_value)
   { vsh->peer_list[peer_id].incremental_value = mt->table[mt_id].header.incremental_value; }
   else { fprintf(stderr,"Header accepted has a smaller inc value ( %u ) than our current one ( %u ) , ignoring it ..\n",mt->table[mt_id].header.incremental_value,vsh->peer_list[peer_id].incremental_value); }

  //We received a READFROM request ( otherwise this function wouldnt have been triggered so lets respond to it )
  /* TODO HERE , HANDLE PERMISSIONS ETC*/
  //fprintf(stderr,"TODO HERE , HANDLE PERMISSIONS ETC\n");

  // If we manage to mark the variable as needing refresh from this socket we will emmit back a signalmsgsuccess , if not we will emmit a failure signal
  unsigned char respTYPE=SIGNALMSGFAILURE;
  unsigned int var_id=mt->table[mt_id].header.var_id;
  if ( MarkVariableAsNeedsRefresh_VariableDatabase(vsh,var_id,peersock) )
   {
     fprintf(stderr,"Peer Signaled that variable %u changed \n",var_id);
     respTYPE=SIGNALMSGSUCCESS; // Only change message type the rest remains the same
   } else
   {
     fprintf(stderr,"Error Peer Signaled that a variable that does not exist (%u) changed \n",var_id);
     respTYPE=SIGNALMSGFAILURE; // Only change message type the rest remains the same
   }

   //We send the aforementioned signal and gracefully exit
   struct failint msg1=NewProtocolRequest_Send(vsh,peer_id,var_id,respTYPE,0,0,0);
   if (msg1.failed) { fprintf(stderr,"Could not add AcceptRequest_Variable to local MessageTable\n"); return 0; }
   *protocol_progress=1;
   *last_protocol_mid=msg1.value;
  }

  if (*protocol_progress==1)
  {
   //Wait for message to be sent
   WaitForMessageTableItemToBeSent(&vsh->peer_list[peer_id].messages.table[*last_protocol_mid]);

   SetAllMessagesOfGroup_Flag_ForRemoval(mt,groupid);
   *protocol_progress=1;
   return 1;
  }

  if (*protocol_progress>=1)
  { fprintf(stderr,"AcceptRequest_SignalChangeVariable protocol progress , ended..!\n"); }

  return 0;
}

