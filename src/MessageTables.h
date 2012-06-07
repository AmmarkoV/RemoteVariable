#ifndef MESSAGETABLES_H_INCLUDED
#define MESSAGETABLES_H_INCLUDED

#include "RemoteVariableSupport.h"

void PrintMessageTableItem(struct MessageTableItem * mti,unsigned int val);

int AllocateMessageQueue(struct MessageTable *  mt,unsigned int total_messages);
int FreeMessageQueue(struct MessageTable * mt);

struct failint AddMessage(struct MessageTable * mt,unsigned int direction,unsigned int free_malloc_at_disposal,struct PacketHeader * header,void * payload,unsigned int msg_timer);
int RemMessage(struct MessageTable * mt,unsigned int mt_id);
int RemFromMessageTableWhereRemoveFlagExists(struct MessageTable * mt);


struct failint SendMessageToSocket(int clientsock,struct MessageTable * mt,unsigned int item_num);
struct failint RecvMessageFromSocket(int clientsock,struct MessageTable * mt,unsigned int msg_timer);

int SetMessage_Flag_ForRemoval(struct MessageTableItem * mti);
int SetAllMessagesOfGroup_Flag_ForRemoval(struct MessageTable * mt,unsigned int groupid);

int DeleteRemovedFromMessageTable(struct MessageTable * mt);


struct failint WaitForMessage(struct MessageTable *mt , unsigned char optype1 , unsigned char optype2 , unsigned int inc_value , unsigned int incoming , unsigned int wait_forever);
unsigned int WaitForMessageTableItemToBeSent(struct MessageTableItem * mti);
struct failint  WaitForSuccessIndicatorAtMessageTableItem(struct MessageTable *mt , unsigned int mt_id, unsigned int wait_forever);
struct failint  WaitForVariableAndCopyItAtMessageTableItem(struct MessageTable *mt , unsigned int mt_id,struct VariableShare *vsh ,unsigned int var_id, unsigned int wait_forever);
#endif // MESSAGETABLES_H_INCLUDED
