#ifndef MESSAGETABLES_H_INCLUDED
#define MESSAGETABLES_H_INCLUDED

#include "RemoteVariableSupport.h"

int AllocateMessageQueue(struct MessageTable *  mt,unsigned int total_messages);
int FreeMessageQueue(struct MessageTable * mt);

struct failint AddToMessageTable(struct MessageTable * mt,unsigned int incoming,unsigned int free_malloc_at_disposal,struct PacketHeader * header,void * payload);
int RemFromMessageTable(struct MessageTable * mt,unsigned int mt_id);
int RemFromMessageTableByIncrementalValue(struct MessageTable * mt,unsigned int inc_val);

int DeleteRemovedFromMessageTable(struct MessageTable * mt);

int WaitForSuccessIndicatorAtMessageTableItem(struct MessageTable *mt , unsigned int mt_id);

#endif // MESSAGETABLES_H_INCLUDED
