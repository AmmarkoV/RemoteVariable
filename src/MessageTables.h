#ifndef MESSAGETABLES_H_INCLUDED
#define MESSAGETABLES_H_INCLUDED

#include "RemoteVariableSupport.h"

void PrintMessageTableItem(struct MessageTableItem * mti,unsigned int val);

int AllocateMessageQueue(struct MessageTable *  mt,unsigned int total_messages);
int FreeMessageQueue(struct MessageTable * mt);

struct failint AddToMessageTable(struct MessageTable * mt,unsigned int incoming,unsigned int free_malloc_at_disposal,struct PacketHeader * header,void * payload);
int RemFromMessageTable(struct MessageTable * mt,unsigned int mt_id);
int RemFromMessageTableByIncrementalValue(struct MessageTable * mt,unsigned int inc_val);

int SetMessageTableItemForRemoval(struct MessageTableItem * mti);

int DeleteRemovedFromMessageTable(struct MessageTable * mt);

struct failint  WaitForSuccessIndicatorAtMessageTableItem(struct MessageTable *mt , unsigned int mt_id);
struct failint  WaitForVariableAndCopyItAtMessageTableItem(struct MessageTable *mt , unsigned int mt_id,struct VariableShare *vsh ,unsigned int var_id);

#endif // MESSAGETABLES_H_INCLUDED
