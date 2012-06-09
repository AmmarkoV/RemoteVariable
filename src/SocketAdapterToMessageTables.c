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
  int peersock = thread_context->peersock;
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

    for (mt_id=0; mt_id<mt->message_queue_current_length; mt_id++)
    {
      if ( (!mt->table[mt_id].executed) &&
            (mt->table[mt_id].direction==INCOMING_MSG) &&
            (!mt->table[mt_id].remove)
          )
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
             if ( Request_SignalChangeVariable(vsh,peer_id,mt->table[mt_id].header.var_id,peersock,mt_id,groupid,protocol_progress,last_protocol_id))
              { mt->table[mt_id].remove=1;  mt->table[mt_id].executed=1; } /*Internal messages must me marked remove here*/
          break;
          case INTERNAL_START_READFROM :
             if ( Request_ReadVariable(vsh,peer_id,mt->table[mt_id].header.var_id,peersock,mt_id,groupid,protocol_progress,last_protocol_id))
              { mt->table[mt_id].remove=1;  mt->table[mt_id].executed=1; } /*Internal messages must me marked remove here*/
          break;

          case INTERNAL_START_WRITETO :
             if ( Request_WriteVariable(vsh,peer_id,mt->table[mt_id].header.var_id,peersock,mt_id,groupid,protocol_progress,last_protocol_id))
              { mt->table[mt_id].remove=1;  mt->table[mt_id].executed=1; } /*Internal messages must me marked remove here*/

          case WRITETO:
             if ( AcceptRequest_WriteVariable(vsh,peer_id,mt,mt_id,peersock,groupid,protocol_progress,last_protocol_id))
             { mt->table[mt_id].executed=1; }
          break;
          case READFROM:
             if ( AcceptRequest_ReadVariable(vsh,peer_id,mt,mt_id,peersock,groupid,protocol_progress,last_protocol_id))
             { mt->table[mt_id].executed=1; }
          break;
          case SIGNALCHANGED :
             if ( AcceptRequest_SignalChangeVariable(vsh,peer_id,mt,mt_id,peersock,groupid,protocol_progress,last_protocol_id) )
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

    usleep(500);
  }
  return 0;
}
