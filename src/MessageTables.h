#ifndef MESSAGETABLES_H_INCLUDED
#define MESSAGETABLES_H_INCLUDED

#include "RemoteVariableSupport.h"

int AllocateMessageQueue(struct MessageTable *  mt,unsigned int total_messages);
int FreeMessageQueue(struct MessageTable * mt);

int AddToMessageTable(struct MessageTable * mt,unsigned int incoming,struct PacketHeader * header,void * payload);
int RemFromMessageTable(struct MessageTable * mt,unsigned int mt_id);

#endif // MESSAGETABLES_H_INCLUDED
