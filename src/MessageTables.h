#ifndef MESSAGETABLES_H_INCLUDED
#define MESSAGETABLES_H_INCLUDED

int AllocateMessageQueue(struct MessageTable *  mt,unsigned int total_messages);
int FreeMessageQueue(struct MessageTable * mt);

int AddToMessageTable(struct MessageTable * queue,unsigned int incoming,struct PacketHeader * header,void * payload);
int RemFromMessageTable(struct MessageTable * queue,unsigned int mt_id);

#endif // MESSAGETABLES_H_INCLUDED
