#ifndef MESSAGETABLES_H_INCLUDED
#define MESSAGETABLES_H_INCLUDED

#include "RemoteVariableSupport.h"

void PrintMessageTableItem(struct MessageTableItem * mti,unsigned int val);

int AllocateMessageQueue(struct MessageTable *  mt,unsigned int total_messages);
int FreeMessageQueue(struct MessageTable * mt);

unsigned char GenNewMessageGroupID( struct MessageTable * mt);
int UpdateGroupIDWithIncoming(struct MessageTable * mt,unsigned char incoming_incremental_value);

int MessageExists(struct MessageTable * mt , unsigned int varid , unsigned char optype, unsigned char direction);
int MessagePendingSend(struct MessageTable * mt , unsigned int varid , unsigned char optype, unsigned char direction);

struct failint AddMessage(struct MessageTable * mt,unsigned int direction,unsigned int free_malloc_at_disposal,struct PacketHeader * header,void * payload);
int RemMessage(struct MessageTable * mt,unsigned int mt_id);
int RemFromMessageTableWhereRemoveFlagExists(struct MessageTable * mt);


int SetMessage_Flag_ForRemoval(struct MessageTableItem * mti);
int SetAllMessagesOfGroup_Flag_ForRemoval(struct MessageTable * mt,unsigned int groupid);

int DeleteRemovedFromMessageTable(struct MessageTable * mt);


struct failint WaitForMessage(struct MessageTable *mt , unsigned char optype1 , unsigned char optype2 , unsigned int groupid , unsigned int incoming , unsigned int wait_forever);
unsigned int WaitForMessageTableItemToBeSent(struct MessageTableItem * mti);
struct failint  WaitForSuccessIndicatorAtMessageTableItem(struct MessageTable *mt , unsigned char groupid, unsigned int wait_forever);

struct failint WaitForVariableAndCopyItAtMessageTableItem(struct MessageTable *mt ,unsigned int peer_id,unsigned char waitingop,unsigned char groupid,struct VariableShare *vsh , unsigned int wait_forever);
#endif // MESSAGETABLES_H_INCLUDED
