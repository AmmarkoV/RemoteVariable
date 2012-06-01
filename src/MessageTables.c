#include "RemoteVariableSupport.h"
#include "MessageTables.h"
#include "helper.h"
#include <sys/errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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

struct failint AddToMessageTable(struct MessageTable * mt,unsigned int incoming,unsigned int free_malloc_at_disposal,struct PacketHeader * header,void * payload)
{
  struct failint retres;
  retres.failed=0;
  retres.value=0;

  if (mt->message_queue_current_length >= mt->message_queue_total_length) { error("complain here\n"); retres.failed=1; return retres; }

  unsigned int mt_pos = mt->message_queue_current_length;
  ++mt->message_queue_current_length;

  mt->table[mt_pos].header=*header;
  mt->table[mt_pos].remove=0;
  mt->table[mt_pos].incoming=incoming;

  if (mt->table[mt_pos].payload!=0) { error("complain here\n"); retres.failed=1; return retres; }


  mt->table[mt_pos].payload_local_malloc=free_malloc_at_disposal;
  mt->table[mt_pos].payload=payload;

  retres.value=mt_pos;
  return retres;
}

int RemFromMessageTable(struct MessageTable * mt,unsigned int mt_id)
{
  if (mt->message_queue_current_length <= mt_id ) { error("complain here\n"); return 0; }
  mt->table[mt_id].remove=0;

    mt->table[mt_id].payload_local_malloc=0;

  if (mt->table[mt_id].payload_local_malloc)
   {
     free(mt->table[mt_id].payload);
     mt->table[mt_id].payload=0;
   } else
   {
     mt->table[mt_id].payload=0;
   }

  return 1;
}

int DeleteRemovedFromMessageTable(struct MessageTable * mt)
{
   unsigned int mt_id=0;
   for (mt_id=0; mt_id<mt->message_queue_current_length; mt_id++)
   {
     if (mt->table[mt_id].remove)
     {
       if (mt->table[mt_id].payload_local_malloc)
        {
          free(mt->table[mt_id].payload);
          mt->table[mt_id].payload=0;
        } else
        {
          mt->table[mt_id].payload=0;
        }
       fprintf(stderr,"Todo proper delete , swap remove respecting ordering etc..\n");
       mt->table[mt_id].payload_local_malloc=0;
       mt->table[mt_id].header.payload_size=0;
     }
   }

  return 1;
}


