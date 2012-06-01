#include "SocketAdapterToMessageTables.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "helper.h"

#include "MessageTables.h"
#include "VariableDatabase.h"

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
         default :
             fprintf(stderr," ERROR CODE %d \n",errornum);
             break;
      };
}


struct failint SendPacketPassedToMT(int clientsock,struct MessageTable * mt,unsigned int item_num)
{
  struct failint retres={0};

  if (mt->table[item_num].incoming)
   {
      fprintf(stderr,"Asked SendPacketAndPassToMT to send an incoming message ( %u ) , skipping operation\n");
      retres.failed=1;
      return retres;
   }

  int opres=send(clientsock,&mt->table[item_num].header,sizeof(mt->table[item_num].header),0);
  if ( opres < 0 ) { fprintf(stderr,"Error %u while SendPacketAndPassToMT \n",errno); retres.failed=1; return retres; }

  if ( mt->table[item_num].header.payload_size > 0 )
   {
     opres=send(clientsock,&mt->table[item_num].payload,mt->table[item_num].header.payload_size,0);
     if ( opres < 0 ) { fprintf(stderr,"Error %u while SendPacketAndPassToMT \n",errno); retres.failed=1; return retres; }
   }


  mt->table[item_num].sent=1;
  retres.value=item_num;
  return retres;
}

struct failint RecvPacketAndPassToMT(int clientsock,struct MessageTable * mt)
{
  struct failint retres={0};

  fprintf(stderr,"Trying RecvPacketAndPassToMT\n");

  struct PacketHeader header;
  int opres=recv(clientsock,&header,sizeof(header),MSG_WAITALL);
  if (opres<0) { fprintf(stderr,"Error complain %u \n",errno); } else
  if (opres!=sizeof(header)) { fprintf(stderr,"Error complain incorrect number of bytes recieved %u \n",opres); }



  if ( header.payload_size > 0 )
   {
    void * payload = (void * ) malloc(header.payload_size);
    opres=recv(clientsock,&payload,header.payload_size,MSG_WAITALL);
    if ( opres < 0 ) { fprintf(stderr,"Error %u while SendPacketAndPassToMT \n",errno); retres.failed=1; return retres; }
    return AddToMessageTable(mt,1,1,&header,payload);
   }

  fprintf(stderr,"RecvPacketAndPassToMT FAILED\n");
  retres.failed=1;
  return retres;
}



void * SocketAdapterToMessageTable_Thread(void * ptr)
{
  struct SocketAdapterToMessageTablesContext * thread_context=(struct SocketAdapterToMessageTablesContext *) ptr;
  struct VariableShare * vsh = thread_context->vsh;
  unsigned int peer_id =  thread_context->peer_id;
  int peersock = thread_context->peersock;
  thread_context->keep_var_on_stack=0;

  struct PacketHeader incoming_packet;
  struct MessageTable * mt = &vsh->peer_list[peer_id].message_queue;
  struct failint res;
  unsigned int mt_id=0;

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


  // Set socket to nonblocking mode..!
  int rest_perms = fcntl(peersock,F_GETFL,0);
  fcntl(peersock,F_SETFL,rest_perms | O_NONBLOCK);


  unsigned int table_iterator=0;
  int data_received = 0;
  while (1)
  {
   /* ------------------------------------------------- RECEIVE PART ------------------------------------------------- */
   data_received = recv(peersock,&incoming_packet,sizeof(incoming_packet), MSG_DONTWAIT |MSG_PEEK);

   if (data_received==sizeof(incoming_packet))
   {
    res=RecvPacketAndPassToMT(peersock,mt);
    if (!res.failed)
    {
    mt_id=res.value;
    switch ( incoming_packet.operation_type )
    {
      case READFROM: AcceptRequest_Variable(vsh,peer_id,mt,mt_id,peersock); break;
      case WRITETO: break;
      case SIGNALCHANGED: AcceptSignalChange_Variable(vsh,peer_id,mt,mt_id,peersock);  break;
      default : fprintf(stderr,"Unhandled incoming packet operation ( %u ) \n",incoming_packet.operation_type);
    };

    }
   } else
   if (data_received<0)
    {
      data_received = errno;
      if (data_received == EWOULDBLOCK)
        {/*This actually isnt an error , if no messages are available at the socket and O_NONBLOCK is set on the socket's file descriptor, recv() shall fail and set errno to [EAGAIN] or [EWOULDBLOCK]*/ }
         else
        { PrintError(data_received); break; }
    }

    //First delete from message table messages pending removal
    DeleteRemovedFromMessageTable(mt);

    for ( table_iterator=0; table_iterator<mt->message_queue_current_length; table_iterator++)
    {
        if ( (!mt->table[table_iterator].remove)&&(!mt->table[table_iterator].incoming)&&(!mt->table[table_iterator].sent) )
        {
          res=SendPacketPassedToMT(peersock,mt,table_iterator);
          if (res.failed) { fprintf(stderr,"Could not SendPacketPassedToMT for table num %u and socket %u \n",table_iterator,peersock); } else
                          { fprintf(stderr,"Success SendPacketPassedToMT for table num %u and socket %u \n",table_iterator,peersock); }
        }
    }

   /*------------------------------------------------- SEND PART -------------------------------------------------*/
  }

  RemPeerBySock(vsh,peersock);

 // fcntl(peersock,F_SETFL,rest_perms);
}
