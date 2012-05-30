#ifndef MESSAGETABLES_H_INCLUDED
#define MESSAGETABLES_H_INCLUDED

struct PacketHeader
{
   unsigned char IncrementalValue;
   unsigned char OperationType;
   unsigned int  PayloadSize;
};

struct MessageTableItem
{
   struct PacketHeader * header;
   unsigned char incoming;
   void * payload;
};

struct MessageTableItem * AllocateMessageQueue(unsigned int total_messages);
int FreeMessageQueue(struct MessageTableItem * queue);


#endif // MESSAGETABLES_H_INCLUDED
