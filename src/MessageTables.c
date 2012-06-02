#include "RemoteVariableSupport.h"
#include "MessageTables.h"
#include "helper.h"
#include <sys/errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "SocketAdapterToMessageTables.h"


int EmptyMTItem(struct MessageTableItem * mti)
{
  mti->header.incremental_value=0;
  mti->header.operation_type=0;
  mti->header.var_id=0;
  mti->header.payload_size=0;
  mti->remove=0;
  mti->executed=0;
  mti->incoming=0;
  mti->sent=0;
  mti->payload=0;
  mti->payload_local_malloc=0;
}


void PrintMessageTableItem(struct MessageTableItem * mti)
{
  fprintf(stderr,"MessageTableItem head.incval= %u , head.optype= %u , head.var_id= %u , head.pay_size=%u )",mti->header.incremental_value,mti->header.operation_type,mti->header.var_id,mti->header.payload_size);
   PrintMessageType(&mti->header);
   fprintf(stderr,"remove= %u , exec= %u , incoming= %u , sent=%u payload_localmalloc=%u , payload=%p)\n",
             mti->remove,mti->executed,mti->incoming,mti->sent,mti->payload_local_malloc,mti->payload);
}


int AllocateMessageQueue(struct MessageTable *  mt,unsigned int total_messages)
{
   if (mt==0) { error("complain"); return 0; }

   mt->locked=0;
   mt->message_queue_current_length=0;
   mt->table = (struct MessageTableItem *) malloc( total_messages * sizeof(struct MessageTableItem) );
   if (mt->table==0) { error("Could not allocate memory for new Message Table!"); return 0; }


   unsigned int i=0;
   for (i=0; i<total_messages; i++)
    {
      EmptyMTItem(&mt->table[i]);
    }

   mt->message_queue_total_length=total_messages;

  return 1;
}

int FreeMessageQueue(struct MessageTable * mt)
{
    int i=0;
    for (i=0; i<mt->message_queue_current_length; i++)
     {
        if (mt->table[i].payload_local_malloc)
         {
           mt->table[i].payload_local_malloc=0;
           free(mt->table[i].payload);
           mt->table[i].payload=0;
         }
     }
    return 1;
}

struct failint AddToMessageTable(struct MessageTable * mt,unsigned int incoming,unsigned int free_malloc_at_disposal,struct PacketHeader * header,void * payload)
{
  fprintf(stderr,"AddToMessageTable incoming = %u , freemalloc_atdispoal = %u , payload = %p , payload_size %u\n",incoming,free_malloc_at_disposal,payload,header->payload_size);
  PrintMessageType(header);

  struct failint retres;
  retres.failed=0;
  retres.value=0;

  if (mt->message_queue_current_length >= mt->message_queue_total_length) { error("AddToMessageTable complain 1\n"); retres.failed=1; return retres; }

  unsigned int mt_pos = mt->message_queue_current_length;
  ++mt->message_queue_current_length;

  mt->table[mt_pos].header=*header;
  mt->table[mt_pos].remove=0;
  mt->table[mt_pos].incoming=incoming;
  mt->table[mt_pos].sent=0;

  if (mt->table[mt_pos].payload!=0)
  {
    fprintf(stderr,"Found uninitialized pointer , this shouldnt be here ..\n");
    if (mt->table[mt_pos].payload_local_malloc)
     {
       fprintf(stderr,"It is a local malloc so freeing it..\n");
       free(mt->table[mt_pos].payload);
       mt->table[mt_pos].payload_local_malloc=0;
       mt->table[mt_pos].payload=0;
     }
  }

  mt->table[mt_pos].payload_local_malloc=free_malloc_at_disposal;
  mt->table[mt_pos].payload=payload;

  if (mt->table[mt_pos].payload!=0)
   {
     fprintf(stderr,"Address of current payload is %p , size = %u ..\n",mt->table[mt_pos].payload,mt->table[mt_pos].header.payload_size);
   }

  fprintf(stderr,"AddToMessageTable ended , new message pos %u\n",mt_pos);
  retres.value=mt_pos;
  return retres;
}


int RemFromMessageTable(struct MessageTable * mt,unsigned int mt_id)
{
  if (mt->message_queue_current_length <= mt_id ) { error("complain here\n"); return 0; }
  fprintf(stderr,"RemFromMessageTable -> mt_id %u \n",mt_id);
  PrintMessageTableItem(&mt->table[mt_id]);


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

  int i=0;
  for (i=mt_id; i<mt->message_queue_current_length; i++)
   {
     mt->table[mt_id]=mt->table[mt_id+1];
   }
   --mt->message_queue_current_length;
   EmptyMTItem(&mt->table[mt->message_queue_current_length]);

  return 1;
}


int RemFromMessageTableByIncrementalValue(struct MessageTable * mt,unsigned int inc_val)
{
  unsigned int mt_id=0;
  unsigned int messages_removed=0;

  for (mt_id=0; mt_id<mt->message_queue_current_length; mt_id++)
   {
     if ( mt->table[mt_id].header.incremental_value == inc_val )
         {
            if ( RemFromMessageTable(mt,mt_id) ) { ++messages_removed; }
         }
   }

  return messages_removed;
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

int WaitForSuccessIndicatorAtMessageTableItem(struct MessageTable *mt , unsigned int mt_id)
{
  if (mt==0) { fprintf(stderr,"WaitForSuccessIndicatorAtMessageTableItem Called with zero MessageTable\n"); return 0;}
  if (mt->message_queue_current_length <= mt_id ) { error("WaitForSuccessIndicatorAtMessageTableItem mt_id out of bounds \n"); return 0; }


  unsigned int our_incremental_value=mt->table[mt_id].header.incremental_value;
  unsigned int done_waiting=0;
  unsigned int mt_traverse=0;
  fprintf(stderr,"WaitForSuccessIndicatorAtMessageTableItem ( waiting for SIGNALMSGSUCCESS or SIGNALMSGFAILURE ) mt_id=%u , mt_length = %u , our_inc_value = %u\n",mt_id,mt->message_queue_current_length,our_incremental_value);
while (!done_waiting)
   {
     if(mt->message_queue_current_length>0)
     {
       if (our_incremental_value==mt->table[mt_traverse].header.incremental_value)
       {
         if (SIGNALMSGSUCCESS==mt->table[mt_traverse].header.operation_type)
            {
             fprintf(stderr,"FOUND SIGNALMSGSUCCESS!\n");
             return 1;
            }  else
         if (SIGNALMSGFAILURE==mt->table[mt_traverse].header.operation_type)
            {
             fprintf(stderr,"FOUND SIGNALMSGFAILURE!\n");
             return 0;
            } else
            {
            //  fprintf(stderr," Encountered : "); PrintMessageType(&mt->table[mt_traverse].header);
            }
       } else
       {
          // fprintf(stderr,"Dismissing group value %u , we have %u , total %u queued messages \n",mt->table[mt_traverse].header.incremental_value,our_incremental_value,mt->message_queue_current_length);
       }

       ++mt_traverse;
     }
       if (mt_traverse>=mt->message_queue_current_length) { mt_traverse=0; }
       usleep(1000);
   }

  fprintf(stderr,"WaitForSuccessIndicatorAtMessageTableItem success\n");
  return 1;
}

int WaitForVariableAndCopyItAtMessageTableItem(struct MessageTable *mt , unsigned int mt_id,struct VariableShare *vsh ,unsigned int var_id)
{
  if (mt==0) { fprintf(stderr,"WaitForVariableAndCopyItAtMessageTableItem Called with zero MessageTable\n"); return 0;}
  if (mt->message_queue_current_length <= mt_id ) { error("WaitForVariableAndCopyItAtMessageTableItem mt_id out of bounds \n"); return 0; }



  unsigned int our_incremental_value=mt->table[mt_id].header.incremental_value;
  unsigned int done_waiting=0;
  unsigned int mt_traverse=0;
  fprintf(stderr,"WaitForVariableAndCopyItAtMessageTableItem ( waiting for RESP_WRITETO ) mt_id=%u , mt_length = %u , our_inc_value = %u , var_id = %u\n",mt_id,mt->message_queue_current_length,our_incremental_value,var_id);
  while (!done_waiting)
   {

     if(mt->message_queue_current_length>0)
     {
       if  (RESP_WRITETO==mt->table[mt_traverse].header.operation_type)
       {
         fprintf(stderr,"Found RESP_WRITETO , our inc_value %u header inc_value %u\n",our_incremental_value,mt->table[mt_id].header.incremental_value);
         if (our_incremental_value==mt->table[mt_traverse].header.incremental_value)
            {
              fprintf(stderr,"Found candidate inc value %u \n",our_incremental_value);
              if ( (var_id==mt->table[mt_traverse].header.var_id) || (vsh->share.variables[var_id].size_of_ptr!=mt->table[mt_traverse].header.payload_size) )
              {

                unsigned int * old_val = (unsigned int *) vsh->share.variables[var_id].ptr;
                fprintf(stderr,"Copying a fresh value for variable %u , was %u now will become %u ( size %u ) \n",var_id,*old_val,mt->table[mt_traverse].header.var_id,vsh->share.variables[var_id].size_of_ptr);
                memcpy(vsh->share.variables[var_id].ptr,mt->table[mt_traverse].payload,vsh->share.variables[var_id].size_of_ptr);
                return 1;
              } else
              {
                fprintf(stderr,"Mixed or tampered RESP_WRITETO for var_id %u while waiting for var_id %u\n",mt->table[mt_traverse].header.var_id,var_id);
                return 0;
              }

            }
       }

       ++mt_traverse;
     }
       if (mt_traverse>=mt->message_queue_current_length) { mt_traverse=0; }
       usleep(1000);
   }
  fprintf(stderr,"WaitForVariableAndCopyItAtMessageTableItem success\n");
  return 1;
}

