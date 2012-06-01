#ifndef MESSAGETABLES_H_INCLUDED
#define MESSAGETABLES_H_INCLUDED

#include "RemoteVariableSupport.h"

int AllocateMessageQueue(struct MessageTable *  mt,unsigned int total_messages);
int FreeMessageQueue(struct MessageTable * mt);

struct failint AddToMessageTable(struct MessageTable * mt,unsigned int incoming,unsigned int free_malloc_at_disposal,struct PacketHeader * header,void * payload);
int RemFromMessageTable(struct MessageTable * mt,unsigned int mt_id);
int DeleteRemovedFromMessageTable(struct MessageTable * mt);

#endif // MESSAGETABLES_H_INCLUDED
