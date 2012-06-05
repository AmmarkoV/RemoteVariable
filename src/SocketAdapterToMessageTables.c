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

void PrintMessageType(struct PacketHeader *header)
{
      switch (header->operation_type)
      {
         case NOACTION : fprintf(stderr,"NOACTION"); break;

         case INTERNAL_START_SIGNALCHANGED : fprintf(stderr,"INTERNAL_START_SIGNALCHANGED"); break;
         case INTERNAL_START_READFROM : fprintf(stderr,"INTERNAL_START_READFROM"); break;
         case INTERNAL_START_WRITETO : fprintf(stderr,"INTERNAL_START_WRITETO"); break;

         case RAW_MESSAGE : fprintf(stderr,"RAW_MESSAGE"); break;


         case RESP_WRITETO : fprintf(stderr,"RESP_WRITETO"); break;
         case WRITETO : fprintf(stderr,"WRITETO"); break;
         case READFROM : fprintf(stderr,"READFROM"); break;
         case SIGNALCHANGED : fprintf(stderr,"SIGNALCHANGED"); break;
         case SIGNALMSGSUCCESS : fprintf(stderr,"SIGNALMSGSUCCESS"); break;
         case SIGNALMSGFAILURE : fprintf(stderr,"SIGNALMSGFAILURE"); break;
         case SYNC : fprintf(stderr,"SYNC"); break;
         default :
             fprintf(stderr," Unknown message type %u \n",header->operation_type);
             break;
      };
  fprintf(stderr," ");
}


struct failint SendPacketPassedToMT(int clientsock,struct MessageTable * mt,unsigned int item_num)
{
  struct failint retres={0};
  retres.value=0;
  retres.failed=0;
  if(sockadap_msg()) fprintf(stderr,"Trying SendPacketPassedToMT message number %u , inc value %u >>>>>>>>>>>>>>>>>>>>>>",item_num,mt->table[item_num].header.incremental_value);
  PrintMessageType(&mt->table[item_num].header);
  fprintf(stderr,"\n");

  if (mt->table[item_num].incoming)
   {
      fprintf(stderr,"Asked SendPacketAndPassToMT to send an incoming message ( %u ) , skipping operation\n",item_num);
      retres.failed=1;
      return retres;
   }

  if(sockadap_msg())
  {
    fprintf(stderr,"About to send the following : \n");
    PrintMessageTableItem(&mt->table[item_num],item_num);
  }

  int opres=send(clientsock,&mt->table[item_num].header,sizeof(mt->table[item_num].header),MSG_WAITALL);
  if ( opres < 0 ) { fprintf(stderr,"Error %u while SendPacketAndPassToMT \n",errno); retres.failed=1; return retres; }

  if ( mt->table[item_num].header.payload_size > 0 )
   {
     unsigned int * payload_val = (unsigned int *) mt->table[item_num].payload;
     fprintf(stderr,"SendPacketPassedToMT sending payload %p , payload_val %u , payload size %u \n", mt->table[item_num].payload,*payload_val,mt->table[item_num].header.payload_size);

     opres=send(clientsock,mt->table[item_num].payload,mt->table[item_num].header.payload_size,MSG_WAITALL);
     if ( opres < 0 ) { fprintf(stderr,"Error %u while SendPacketAndPassToMT \n",errno); retres.failed=1; return retres; } else
     if ( opres != mt->table[item_num].header.payload_size ) { fprintf(stderr,"Payload was not fully transmitted only %u of %u bytes \n",opres,mt->table[item_num].header.payload_size);
                                                                retres.failed=1; return retres; }

   }

  //This message will trigger will travel to the peer with the send operation but its role is not finished here
  //It will start a protocol action so we set it NOT for removal and NOT executed
  mt->table[item_num].executed=0;
  mt->table[item_num].remove=0;

  //Rest of initialization..
  mt->table[item_num].sent=1;


  if(sockadap_msg()) fprintf(stderr,"SendPacketPassedToMT complete ----------------\n");

  retres.value=item_num;
  retres.failed=0;
  return retres;
}

struct failint RecvPacketAndPassToMT(int clientsock,struct MessageTable * mt,unsigned int msg_timer)
{
  struct failint retres={0};
  retres.value=0;
  retres.failed=0;

  if(sockadap_msg()) fprintf(stderr,"Trying RecvPacketAndPassToMT <<<<<<<<<<<<<<<<<<<<<<< ");

  struct PacketHeader header={0};
  int opres=recv(clientsock,&header,sizeof(struct PacketHeader),MSG_WAITALL);
  if (opres<0) { fprintf(stderr,"Error RecvPacketAndPassToMT recv error %u \n",errno); retres.failed=1; return retres; } else
  if (opres!=sizeof(struct PacketHeader)) { fprintf(stderr,"Error RecvPacketAndPassToMT incorrect number of bytes recieved %u \n",opres); retres.failed=1; return retres; }

  if(sockadap_msg())
  {
    PrintMessageType(&header);
    fprintf(stderr," inc value %u \n",header.incremental_value);
  }


  if ( header.payload_size > 0 )
   {
    if(sockadap_msg())fprintf(stderr,"RecvPacketAndPassToMT waiting and mallocing for payload size %u \n",header.payload_size);
    void * payload = (void * ) malloc(header.payload_size);
    if (payload==0) { fprintf(stderr,"Error mallocing %u bytes \n ",header.payload_size); }
    opres=recv(clientsock,payload,header.payload_size,MSG_WAITALL);
    if ( opres < 0 ) { fprintf(stderr,"Error %u while SendPacketAndPassToMT \n",errno); retres.failed=1; return retres; } else
    if ( opres != header.payload_size ) { fprintf(stderr,"Payload was not fully received only %u of %u bytes \n",opres,header.payload_size); retres.failed=1; return retres; }

    unsigned int * payload_val=(unsigned int * ) payload;
    if(sockadap_msg())fprintf(stderr,"received payload seems to be carrying value %u \n",*payload_val);

    retres = AddToMessageTable(mt,1,1,&header,payload,msg_timer);
    if(sockadap_msg())fprintf(stderr,"RecvPacketAndPassToMT complete with payload----------------\n");
    return retres;
   } else
   {
     if(sockadap_msg())fprintf(stderr,"RecvPacketAndPassToMT with no payload\n");
   }

  retres = AddToMessageTable(mt,1,0,&header,0,msg_timer);

  if(sockadap_msg()) fprintf(stderr,"RecvPacketAndPassToMT complete ----------------\n");
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

  fprintf(stderr,"SocketAdapterToMessageTable_Thread started , peer_id = %u , peer socket = %d , master = %u \n",peer_id,peersock,vsh->this_address_space_is_master);

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

  fprintf(stderr,"SocketAdapterToMessageTable_Thread done with handshakes , starting recv/send loop\n\n\n\n\n");


  // Set socket to nonblocking mode..!
  int rest_perms = fcntl(peersock,F_GETFL,0);

  //Allow MessageProc Threads to wake up!
  mt->pause_internal_messageproc_thread=0;
  mt->pause_external_messageproc_thread=0;

  int data_received = 0;
  while (! mt->stop_sendrecv_thread)
  {



     /* PAUSE PART ------------------------------------------------------------------------*/
      if ( vsh->peer_list[peer_id].messages.pause_sendrecv_thread == 1 )
       { //We just received a Pause request..
         vsh->peer_list[peer_id].messages.pause_sendrecv_thread=2; // We signal we got it
         unsigned int wait_time=0;
         while( vsh->peer_list[peer_id].messages.pause_sendrecv_thread > 0 )
           {
             usleep(100);
             if (wait_time>100) { fprintf(stderr,".RS."); }
             ++wait_time;
           }
       }
     /* PAUSE PART ------------------------------------------------------------------------*/






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
    ++vsh->central_timer;
    res=RecvPacketAndPassToMT(peersock,mt,vsh->central_timer);
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
    //if (mt->message_queue_current_length!=0) { fprintf(stderr,"Table iterator scanning %u messages \n",mt->message_queue_current_length); }

    for ( table_iterator=0; table_iterator< mt->message_queue_current_length; table_iterator++)
    {
        if ( (!mt->table[table_iterator].remove)&&(!mt->table[table_iterator].incoming)&&(!mt->table[table_iterator].sent) )
        {
          ++vsh->central_timer;
          res=SendPacketPassedToMT(peersock,mt,table_iterator);
          if (res.failed) { fprintf(stderr,"Could not SendPacketPassedToMT for table num %u and socket %u \n",table_iterator,peersock); } else
                          { fprintf(stderr,"Success SendPacketPassedToMT for table num %u and socket %u \n",table_iterator,peersock); }
        }
    }

   }

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
  if ((mt==0)||(mt->table==0)) { fprintf(stderr," Serious Error MessageTable doesnt seem to be allocated\n\ "); return 0; }

  unsigned int * stop_switch=0;
  unsigned int * pause_switch=0;
  if (internal_loop==1) {stop_switch=&mt->stop_internal_messageproc_thread;
                         pause_switch=&mt->pause_internal_messageproc_thread; }
                          else
                        {stop_switch=&mt->stop_external_messageproc_thread;
                         pause_switch=&mt->pause_external_messageproc_thread; }

  //We permit the var to be deallocated from the thread generator stack
  thread_context->keep_var_on_stack=0;

  fprintf(stderr,"Started JobAndMessageTableExecutor_Thread for peer %u , internal_loop = %u",peer_id,internal_loop);

  while (*pause_switch) { fprintf(stderr,".INIT."); usleep(100); }


  if (internal_loop==2) {return 0; }
  if (internal_loop==1) { internal_loop=0; } // Use only one global process loop


  unsigned int mt_id=0;
  while (! *stop_switch)
  {

    for (mt_id=0; mt_id<mt->message_queue_current_length; mt_id++)
    {
      if ( (!mt->table[mt_id].executed) &&
            (mt->table[mt_id].incoming) &&
            (!mt->table[mt_id].remove)
          )
      {
       //INTERNAL MESSAGING THREAD WORK
       if ( (internal_loop==0) || (internal_loop==1) ) // This is an internal message only processing thread
       {
          switch ( mt->table[mt_id].header.operation_type )
         {
          case NOACTION : fprintf(stderr,"NOACTION doesnt trigger @ Internal Message Processing Thread\n"); break;
          case INTERNAL_START_SIGNALCHANGED : Request_SignalChangeVariable(vsh,peer_id,mt->table[mt_id].header.var_id,peersock); mt->table[mt_id].remove=1;  mt->table[mt_id].executed=1; /*Internal messages must me marked remove here*/ break;
          case INTERNAL_START_READFROM : Request_ReadVariable(vsh,peer_id,mt->table[mt_id].header.var_id,peersock); mt->table[mt_id].remove=1;   mt->table[mt_id].executed=1;/*Internal messages must me marked remove here*/ break;
          case INTERNAL_START_WRITETO : Request_WriteVariable(vsh,peer_id,mt->table[mt_id].header.var_id,peersock); mt->table[mt_id].remove=1;  mt->table[mt_id].executed=1; /*Internal messages must me marked remove here*/ break;
         };

       }

       //EXTERNAL MESSAGING THREAD WORK
       if ( (internal_loop==0) || (internal_loop==2) ) // This is an external message only processing thread
       {

         switch ( mt->table[mt_id].header.operation_type )
         {
          case NOACTION : fprintf(stderr,"NOACTION doesnt trigger @ External Message Processing Thread\n"); break;
          case RESP_WRITETO : fprintf(stderr,"RESP_WRITETO received packet doesnt trigger New Protocol Request\n"); break;
          case WRITETO:  AcceptRequest_WriteVariable(vsh,peer_id,mt,mt_id,peersock); mt->table[mt_id].executed=1; break;
          case READFROM: AcceptRequest_ReadVariable(vsh,peer_id,mt,mt_id,peersock); mt->table[mt_id].executed=1; break;
          case SIGNALCHANGED : AcceptRequest_SignalChangeVariable(vsh,peer_id,mt,mt_id,peersock); mt->table[mt_id].executed=1; break;
          case SIGNALMSGSUCCESS : fprintf(stderr,"SIGNALMSGSUCCESS received packet doesnt trigger New Protocol Request\n"); break;
          case SIGNALMSGFAILURE : fprintf(stderr,"SIGNALMSGFAILURE received packet doesnt trigger New Protocol Request\n"); break;
          case SYNC : fprintf(stderr,"SYNC received packet doesnt trigger New Protocol Request\n"); break;
         };

       }
      }
    }


    //First delete from message table messages pending removal
    if ( (internal_loop==0) || (internal_loop==2) )
       { // Only external or global thread removes messages to make things simpler..

         // We will also pause the SEND/RECV thread in order to keep them from accessing the messagetable structure
         vsh->peer_list[peer_id].messages.pause_sendrecv_thread=1;
         unsigned int wait_time=0;
         while (vsh->peer_list[peer_id].messages.pause_sendrecv_thread!=2)
         {
           usleep(100);
           if (wait_time>100) fprintf(stderr,".REM.");
           ++wait_time;
         }

         //Right now we are the only thread that has access to the MessageTable Structure
         RemFromMessageTableWhereRemoveFlagExists(mt);
         //Ok , we have removed the trash , now to resume functionality..

         vsh->peer_list[peer_id].messages.pause_sendrecv_thread=0;
       }

    usleep(500);
  }
  return 0;
}
