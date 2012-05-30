#include "RemoteVariableSupport.h"
#include "MessageTables.h"
#include "helper.h"
#include <sys/errno.h>

int AllocateMessageQueue(struct MessageTable *  mt,unsigned int total_messages)
{
   if (mt==0) { error("complain"); return 0; }

   mt->locked=0;
   mt->message_queue_current_length=0;
   mt->table = (struct MessageTableItem *) malloc( total_messages * sizeof(struct MessageTableItem) );
   if (mt->table==0) { error("Could not allocate memory for new Message Table!"); return 0; }

   mt->message_queue_total_length=total_messages;

  return 1;
}

int FreeMessageQueue(struct MessageTable * mt)
{
    free(mt);
    return 0;
}

int AddToMessageTable(struct MessageTable * mt,unsigned int incoming,struct PacketHeader * header,void * payload)
{
  if (mt->message_queue_current_length >= mt->message_queue_total_length) { error("complain here\n"); return 0; }

  unsigned int mt_pos = mt->message_queue_current_length;
  ++mt->message_queue_current_length;

  mt->table[mt_pos].header=*header;
  mt->table[mt_pos].remove=0;
  mt->table[mt_pos].incoming=incoming;

  if (mt->table[mt_pos].payload!=0) { error("complain here\n"); return 0; }
  mt->table[mt_pos].payload = (void *) malloc(header->payload_size);
  memcpy(mt->table[mt_pos].payload,payload,mt->table[mt_pos].header.payload_size);

  return 1;
}

int RemFromMessageTable(struct MessageTable * mt,unsigned int mt_id)
{
  if (mt->message_queue_current_length <= mt_id ) { error("complain here\n"); return 0; }
  mt->table[mt_id].remove=0;

  return 1;
}
