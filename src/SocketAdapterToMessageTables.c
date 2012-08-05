/***************************************************************************
* Copyright (C) 2010-2012 by Ammar Qammaz *
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
#include "SocketAdapterToMessageTables.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "helper.h"

#include "MessageTables.h"
#include "VariableDatabase.h"
#include "Protocol.h"
#include "Peers.h"

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/uio.h>
#include <errno.h>
#include <fcntl.h>

void PrintError(unsigned int errornum)
{
      fprintf(stderr," Error received is  :");

      switch (errornum)
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
         case ECONNRESET :
               fprintf(stderr,"Connection reset by peer.\n"); break;
         default :
             fprintf(stderr," ERROR CODE %d \n",errornum);
             break;
      };
}



char * ReturnPrintMessageTypeVal(unsigned char optype)
{
      switch (optype)
      {
         case NOACTION : return "NOACTION"; break;

         case INTERNAL_START_SIGNALCHANGED : return "INTERNAL_START_SIGNALCHANGED"; break;
         case INTERNAL_START_READFROM : return "INTERNAL_START_READFROM"; break;
         case INTERNAL_START_WRITETO : return "INTERNAL_START_WRITETO"; break;

         case RAW_MESSAGE : return "RAW_MESSAGE"; break;


         case RESP_WRITETO : return "RESP_WRITETO"; break;
         case WRITETO : return "WRITETO"; break;
         case READFROM : return "READFROM"; break;
         case SIGNALCHANGED : return "SIGNALCHANGED"; break;
         case SIGNALMSGSUCCESS : return "SIGNALMSGSUCCESS"; break;
         case SIGNALMSGFAILURE : return "SIGNALMSGFAILURE"; break;
         case SYNC : return "SYNC"; break;
         default :
             fprintf(stderr," Unknown message type %u \n",optype);
             return "UNKNOWN";
             break;
      };

      return "NOTHING";
}


void PrintMessageTypeVal(unsigned char optype)
{
  fprintf(stderr," %s ",ReturnPrintMessageTypeVal(optype));
}


void PrintMessageType(struct PacketHeader *header)
{
      PrintMessageTypeVal(header->operation_type);
}



/*!


        -------------------------------------------------------------------------
        -------------------------------------------------------------------------

        -------------------------------------------------------------------------
        -------------------------------------------------------------------------


     SEND/RECEIVE OPERATIONS

          |
          |
         \ /
          -
!*/



struct failint SendMessageToSocket(int clientsock,struct MessageTable * mt,unsigned int item_num)
{
  struct failint retres={0};
  retres.value=0;
  retres.failed=0;

  if (mt->table[item_num].direction != OUTGOING_MSG)
   {
      fprintf(stderr,"ERROR : Asked SendPacketAndPassToMT to send a non Outgoing message ( %u ) , skipping operation\n",item_num);
      retres.failed=1;
      return retres;
   }

  if (mutex_msg()) fprintf(stderr,"Waiting for mutex SendMessageToSocket\n");
  pthread_mutex_lock (&mt->lock); // LOCK PROTECTED OPERATION -------------------------------------------
  if (mutex_msg()) fprintf(stderr,"Entered mutex SendMessageToSocket\n");

  if(sockadap_msg())
  {
    fprintf(stderr,"SENDING %s mt_id=%u , GROUP %u \n",ReturnPrintMessageTypeVal(mt->table[item_num].header.operation_type), item_num, mt->table[item_num].header.incremental_value);
  }

  //This message will trigger will travel to the peer with the send operation but its role is not finished here
  //It will start a protocol action so we set it NOT for removal and NOT executed
  mt->table[item_num].executed=0;
  mt->table[item_num].remove=0;

  //Rest of initialization..
  mt->table[item_num].sent=1; // We set it as sent , if sending fails we will restore it



  int opres=send(clientsock,&mt->table[item_num].header,sizeof(mt->table[item_num].header),MSG_WAITALL|MSG_NOSIGNAL);
  if ( opres < 0 ) { fprintf(stderr,"Error %u while SendPacketAndPassToMT \n",errno); retres.failed=1;  mt->table[item_num].sent=0; pthread_mutex_unlock(&mt->lock); return retres; }

  if ( mt->table[item_num].header.payload_size > 0 )
   {
     unsigned int * payload_val = (unsigned int *) mt->table[item_num].payload;

     fprintf(stderr,"SENDING %s payload %p , payload_val %u , size %u GROUP %u \n",ReturnPrintMessageTypeVal(mt->table[item_num].header.operation_type),mt->table[item_num].payload,*payload_val,mt->table[item_num].header.payload_size,mt->table[item_num].header.incremental_value);
     printf("SENDING %s payload %p , payload_val %u , size %u GROUP %u \n",ReturnPrintMessageTypeVal(mt->table[item_num].header.operation_type),mt->table[item_num].payload,*payload_val,mt->table[item_num].header.payload_size,mt->table[item_num].header.incremental_value);

     opres=send(clientsock,mt->table[item_num].payload,mt->table[item_num].header.payload_size,MSG_WAITALL|MSG_NOSIGNAL);
     if ( opres < 0 ) { fprintf(stderr,"Error %u while SendPacketAndPassToMT \n",errno); retres.failed=1; mt->table[item_num].sent=0; pthread_mutex_unlock(&mt->lock); return retres; } else
     if ( opres != mt->table[item_num].header.payload_size ) { fprintf(stderr,"Payload was not fully transmitted only %u of %u bytes \n",opres,mt->table[item_num].header.payload_size);
                                                                retres.failed=1; mt->table[item_num].sent=0;  pthread_mutex_unlock(&mt->lock); return retres; }

   }



  pthread_mutex_unlock(&mt->lock); // LOCK PROTECTED OPERATION -------------------------------------------

  retres.value=item_num;
  retres.failed=0;
  return retres;
}

struct failint RecvMessageFromSocket(int clientsock,struct MessageTable * mt)
{
  struct failint retres={0};
  retres.value=0;
  retres.failed=0;

  struct PacketHeader header={0};
  int opres=recv(clientsock,&header,sizeof(struct PacketHeader),MSG_WAITALL);
  if (opres<0) { fprintf(stderr,"Error RecvPacketAndPassToMT recv error %u \n",errno); retres.failed=1; return retres; } else
  if (opres!=sizeof(struct PacketHeader)) { fprintf(stderr,"Error RecvPacketAndPassToMT incorrect number of bytes recieved %u \n",opres); retres.failed=1; return retres; }

  if ( header.payload_size > 0 )
   {
    void * payload = (void * ) malloc(header.payload_size);
    if (payload==0) { fprintf(stderr,"Error mallocing %u bytes \n ",header.payload_size); }
    opres=recv(clientsock,payload,header.payload_size,MSG_WAITALL);
    if ( opres < 0 ) { fprintf(stderr,"Error %u while SendPacketAndPassToMT \n",errno); retres.failed=1; return retres; } else
    if ( opres != header.payload_size ) { fprintf(stderr,"Payload was not fully received only %u of %u bytes \n",opres,header.payload_size); retres.failed=1; return retres; }

    retres = AddMessage(mt,INCOMING_MSG,1,&header,payload);
    if(sockadap_msg())
     {
        unsigned int * payload_val=(unsigned int * ) payload;
        fprintf(stderr,"RECEIVED %s - GROUP %u - With %u bytes of payload %u ----------------\n",ReturnPrintMessageTypeVal(header.operation_type),header.incremental_value,header.payload_size,*payload_val);
     }
    return retres;
   } else
   {
     if(sockadap_msg())
      {
        fprintf(stderr,"RECEIVED %s - GROUP %u - with NO payload ----------------\n",ReturnPrintMessageTypeVal(header.operation_type),header.incremental_value);
      }
   }

  retres = AddMessage(mt,INCOMING_MSG,0,&header,0);
  return retres;
}

void * SocketAdapterToMessageTable_Thread(void * ptr)
{
  struct SocketAdapterToMessageTablesContext * thread_context=(struct SocketAdapterToMessageTablesContext *) ptr;
  struct VariableShare * vsh = thread_context->vsh;
  unsigned int peer_id =  thread_context->peer_id;
  int peersock = thread_context->peersock;

  //We permit the var to be deallocated from the thread generator stack
  thread_context->keep_var_on_stack=0;

  struct PacketHeader incoming_packet;
  struct MessageTable * mt = &vsh->peer_list[peer_id].messages;
  struct failint res;
  unsigned int table_iterator=0;

  mt->message_queue_current_length=0;


  fprintf(stderr,"SocketAdapterToMessageTable_Thread started , peer_id = %u , peer socket = %d , master = %u \n",peer_id,peersock,vsh->this_address_space_is_master);
  fprintf(stderr,"Message Pool %u/%u \n",mt->message_queue_current_length,mt->message_queue_total_length);

  if ( vsh->this_address_space_is_master )
   {
      if (Start_Version_Handshake(vsh,peersock)) { vsh->peer_list[peer_id].peer_state=VSS_NORMAL; } else
                                                 { fprintf(stderr,"SocketAdapterToMessageTable_Thread Thread : Could not accept handshake for peer %u",peer_id);
                                                   vsh->peer_list[peer_id].peer_state=VSS_UNITIALIZED;
                                                   RemPeerBySock(vsh,peersock);
                                                   close(peersock);
                                                   return 0;}
   } else
   {
      if (Accept_Version_Handshake(vsh,peersock)) { vsh->peer_list[peer_id].peer_state=VSS_NORMAL; } else
                                                  { fprintf(stderr,"SocketAdapterToMessageTable_Thread Thread : Could not accept handshake for peer %u",peer_id);
                                                    vsh->peer_list[peer_id].peer_state=VSS_UNITIALIZED;
                                                    RemPeerBySock(vsh,peersock);
                                                    close(peersock);
                                                    return 0;}
   }

  fprintf(stderr,"SocketAdapterToMessageTable_Thread done with handshakes , starting recv/send loop with a %u pause val\n\n\n\n\n",vsh->peer_list[peer_id].messages.pause_sendrecv_thread);


  // Set socket to nonblocking mode..!
  int rest_perms = fcntl(peersock,F_GETFL,0);

  //Allow MessageProc Threads to wake up!
  mt->pause_messageproc_thread=0;

  int data_received = 0;

  while (! mt->stop_sendrecv_thread)
  {

//   pthread_mutex_lock (&mt->lock); // LOCK PROTECTED OPERATION -------------------------------------------

   /* ------------------------------------------------- RECEIVE PART ------------------------------------------------- */
   incoming_packet.incremental_value=0;
   incoming_packet.operation_type=0;
   incoming_packet.payload_size=0;
   incoming_packet.var_id=0;

   fcntl(peersock,F_SETFL,rest_perms | O_NONBLOCK);
     data_received = recv(peersock,&incoming_packet,sizeof(incoming_packet), MSG_DONTWAIT |MSG_PEEK);
   fcntl(peersock,F_SETFL,rest_perms); // restore sockets to prior blocking state ( TODO: add de init code afterwards )

   if (data_received==0) { /* RADIO :P SILENCE */ }
      else
   if (data_received==sizeof(incoming_packet))
   {
    res=RecvMessageFromSocket(peersock,mt);
    if (res.failed) { fprintf(stderr,"Failed passing socket recv to message table\n"); }
   }
     else
   if (data_received<0)
   {
      data_received = errno;
      if (data_received == EWOULDBLOCK)
        {/*This actually isnt an error , if no messages are available at the socket and O_NONBLOCK is set on the socket's file descriptor, recv() shall fail and set errno to [EAGAIN] or [EWOULDBLOCK]*/ }
         else
        { fprintf(stderr,"Receive (recv) thread encountered error , "); PrintError(data_received); break; }
   } else
   {
        fprintf(stderr,"A part of a incoming packet just arrived , sized %u \n",data_received);
   }



 /*------------------------------------------------- SEND PART -------------------------------------------------*/
   if (mt->message_queue_current_length>0)
   {
    for ( table_iterator=0; table_iterator< mt->message_queue_current_length; table_iterator++)
    {
        if ( (!mt->table[table_iterator].remove)&&(mt->table[table_iterator].direction==OUTGOING_MSG)&&(!mt->table[table_iterator].sent) )
        {
          res=SendMessageToSocket(peersock,mt,table_iterator);
          if (res.failed) { fprintf(stderr,"Could not SendMessageToSocket for table num %u and socket %u \n",table_iterator,peersock); } else
                          { /*fprintf(stderr,"Success SendPacketPassedToMT for table num %u and socket %u \n",table_iterator,peersock); */ }
        }
    }

   }

 //   pthread_mutex_unlock (&mt->lock); // LOCK PROTECTED OPERATION -------------------------------------------
   usleep(100);
  }

  RemPeerBySock(vsh,peersock);
  return 0;
}



void * JobAndMessageTableExecutor_Thread(void * ptr)
{
  struct SocketAdapterToMessageTablesContext * thread_context=(struct SocketAdapterToMessageTablesContext *) ptr;
  struct VariableShare * vsh = thread_context->vsh;
  unsigned int peer_id =  thread_context->peer_id;
//  int peersock = thread_context->peersock;
  unsigned int internal_loop = thread_context->type_of_thread;
  struct MessageTable * mt = &vsh->peer_list[peer_id].messages;
  if ((mt==0)||(mt->table==0)) { fprintf(stderr," Serious Error MessageTable doesnt seem to be allocated\n"); return 0; }

  unsigned int * sendrcv_pause_switch=&vsh->peer_list[peer_id].messages.pause_sendrecv_thread;

  unsigned int * stop_switch=0;
  unsigned int * pause_switch=0;

  stop_switch=&mt->stop_messageproc_thread;
  pause_switch=&mt->pause_messageproc_thread;

  //We permit the var to be deallocated from the thread generator stack
  thread_context->keep_var_on_stack=0;

  fprintf(stderr,"Started JobAndMessageTableExecutor_Thread for peer %u , internal_loop = %u",peer_id,internal_loop);


  while (*pause_switch) { fprintf(stderr,".INIT."); usleep(100); }



  unsigned char * protocol_progress=0;
  unsigned int * last_protocol_id=0;
  unsigned char * groupid=0;

  unsigned int mt_id=0;
  while (! (*stop_switch) )
  {
   if (*sendrcv_pause_switch==0)
   {
    for (mt_id=0; mt_id<mt->message_queue_current_length; mt_id++)
    {

   /* -----------------------------------------------------------
                        OUTGOING MESSAGE HANDLER
      -----------------------------------------------------------  */
      if ( (!mt->table[mt_id].executed) &&
            (mt->table[mt_id].direction==OUTGOING_MSG) &&
            (!mt->table[mt_id].remove) )
      {
       protocol_progress = &mt->table[mt_id].protocol_progress;
       last_protocol_id = &mt->table[mt_id].last_protocol_id;
       groupid = &mt->table[mt_id].header.incremental_value;

       switch ( mt->table[mt_id].header.operation_type )
         {
          case WRITETO :
            if (mt->table[mt_id].sent)
             {  mt->table[mt_id].remove=1; mt->table[mt_id].executed=1; }
          break;
         };
      }
        else
   /* -----------------------------------------------------------
                        INCOMING MESSAGE HANDLER
      -----------------------------------------------------------  */

      if ( (!mt->table[mt_id].executed) &&
            (mt->table[mt_id].direction==INCOMING_MSG) &&
            (!mt->table[mt_id].remove) )
      {

       protocol_progress = &mt->table[mt_id].protocol_progress;
       last_protocol_id = &mt->table[mt_id].last_protocol_id;
       groupid = &mt->table[mt_id].header.incremental_value;

       switch ( mt->table[mt_id].header.operation_type )
         {
          case NOACTION :
            fprintf(stderr,"NOACTION ( id %u ) doesnt trigger @  Message Processing Thread\n",mt_id);
            fprintf(stderr,"ERROR : There should be no NOACTION messages .. \n");
            mt->table[mt_id].remove=1;
            mt->table[mt_id].executed=1;
          break;

          case INTERNAL_START_SIGNALCHANGED :
             if ( Request_SignalChangeVariable(vsh,peer_id,mt->table[mt_id].header.var_id,mt_id,groupid,protocol_progress,last_protocol_id))
              { mt->table[mt_id].remove=1;  mt->table[mt_id].executed=1; } /*Internal messages must me marked remove here*/
          break;
          case INTERNAL_START_READFROM :
             if ( Request_ReadVariable(vsh,peer_id,mt->table[mt_id].header.var_id,mt_id,groupid,protocol_progress,last_protocol_id))
              { mt->table[mt_id].remove=1;  mt->table[mt_id].executed=1; } /*Internal messages must me marked remove here*/
          break;

          case INTERNAL_START_WRITETO :
             if ( Request_WriteVariable(vsh,peer_id,mt->table[mt_id].header.var_id,mt_id,groupid,protocol_progress,last_protocol_id))
              { mt->table[mt_id].remove=1;  mt->table[mt_id].executed=1; } /*Internal messages must me marked remove here*/

          case WRITETO: // INCOMING WRITETO REQUEST
             if ( AcceptRequest_WriteVariable(vsh,peer_id,mt,mt_id,groupid,protocol_progress,last_protocol_id))
             { mt->table[mt_id].executed=1; mt->table[mt_id].remove=1; }
          break;
          case READFROM: // INCOMING READFROM REQUEST
             if ( AcceptRequest_ReadVariable(vsh,peer_id,mt,mt_id,groupid,protocol_progress,last_protocol_id))
             { mt->table[mt_id].executed=1; }
          break;
          case SIGNALCHANGED : // INCOMING SIGNALCHANGED REQUEST
             if ( AcceptRequest_SignalChangeVariable(vsh,peer_id,mt,mt_id,groupid,protocol_progress,last_protocol_id) )
             { mt->table[mt_id].executed=1; }
          break;

          case SYNC : fprintf(stderr,"SYNC received packet doesnt trigger New Protocol Request\n"); break;
         };

       }
      }


    if (mt->GUARD_BYTE1 != RVS_GUARD_VALUE ) { fprintf(stderr,"GUARD VALUE 1 CORRUPTED \n"); }
    if (mt->GUARD_BYTE2 != RVS_GUARD_VALUE ) { fprintf(stderr,"GUARD VALUE 2 CORRUPTED \n"); }

    //First delete from message table messages pending removal

         // We will also pause the SEND/RECV thread in order to keep them from accessing the messagetable structure

         //Right now we are the only thread that has access to the MessageTable Structure
          //This call locks internally using the table mutex..
      if (mt->message_queue_current_length >90 )
       {
           RemFromMessageTableWhereRemoveFlagExists(mt);
       }

      //Ok , we have removed the trash , now to resume functionality..


   }
    usleep(500);
  }
  return 0;
}
