#include "SocketAdapterToMessageTables.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "helper.h"

#include "MessageTables.h"
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




int SendPacketAndPassToMT(int clientsock,struct MessageTable * mt,unsigned int item_num)
{
  int opres=send(clientsock,&mt->table[item_num].header,sizeof(mt->table[item_num].header),0);
  if ( opres < 0 ) { fprintf(stderr,"Error %u while SendPacketAndPassToMT \n",errno); return opres; }

  opres=send(clientsock,&mt->table[item_num].payload,mt->table[item_num].header.payload_size,0);
  if ( opres < 0 ) { fprintf(stderr,"Error %u while SendPacketAndPassToMT \n",errno); return opres; }

  mt->table[item_num].sent=1;
  return opres;
}

int RecvPacketAndPassToMT(int clientsock,struct MessageTable * mt)
{
  fprintf(stderr,"Trying RecvPacketAndPassToMT\n");

  struct PacketHeader header;
  int opres=recv(clientsock,&header,sizeof(header),MSG_WAITALL);
  if (opres<0) { fprintf(stderr,"Error complain %u \n",errno); } else
  if (opres!=sizeof(header)) { fprintf(stderr,"Error complain incorrect number of bytes recieved %u \n",opres); } else

  opres=recv(clientsock,&header,sizeof(header),MSG_WAITALL);


  void * payload=(void*) malloc(header.payload_size);
  return AddToMessageTable(mt,1,&header,payload);
}



void * SocketAdapterToMessageTable_Thread(void * ptr)
{
  struct PacketHeader incoming_packet;
  int rest_perms = fcntl(peersock,F_GETFL,0);
  fcntl(peersock,F_SETFL,rest_perms | O_NONBLOCK);

  int data_received = recv(peersock,&incoming_packet,sizeof(incoming_packet), MSG_DONTWAIT |MSG_PEEK);


   if (data_received==sizeof(incoming_packet))
   {
    //  RecvPacketAndPassToMT(peersock,mt)
   } else
   //if (data_received>0)
   //{  } else
   if (data_received<0)
    {
      data_received = errno;

      if (data_received == EWOULDBLOCK)
        {
          //This actually isnt an error!
          //If no messages are available at the socket and O_NONBLOCK is set on the socket's file descriptor, recv() shall fail and set errno to [EAGAIN] or [EWOULDBLOCK].
          return 1;
        }


    }


  fcntl(peersock,F_SETFL,rest_perms);
  // --------------- LOCK PROTECTED OPERATION  ---------------------



}
