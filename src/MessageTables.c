#include "RemoteVariableSupport.h"
#include "MessageTables.h"
#include "helper.h"
#include <sys/errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "SocketAdapterToMessageTables.h"


int WaitForMessageLockToClear(struct MessageTable * mt)
{
  if (mt==0) { fprintf(stderr,"WaitForMessageLockToClear skipped by null pointer \n"); return 0; }
  unsigned int waitloops=0;
  while (mt->locked)
   {
     usleep(1000);
     ++waitloops;
   }

  if (waitloops>0) { fprintf(stderr,"WaitForMessageLockToClear waited for %u ms ( loops )\n",waitloops); }
  return 1;
}

int LockMessageTable(struct MessageTable * mt)
{
  if (mt==0) { fprintf(stderr,"LockMessageTable skipped by null pointer \n"); return 0; }
  if (mt->locked!=0) {
                      fprintf(stderr,"LockMessageTable found locked variable ! , waiting for it to clear out\n");
                      WaitForMessageLockToClear(mt);
                    }
  mt->locked=1;
  return 1;
}

int UnlockMessageTable(struct MessageTable * mt)
{
  if (mt==0) { fprintf(stderr,"UnlockSocket skipped by null pointer \n"); return 0; }
  mt->locked=0;
  return 1;
}



void EmptyMTItem(struct MessageTableItem * mti,unsigned int ignore_payload)
{
  if (mti==0) {fprintf(stderr,"EmptyMTIItem called with null parameter \n "); }

if (!ignore_payload)
{
 if (mti->payload!=0)
  {
    if (mti->payload_local_malloc)
     {
       fprintf(stderr,"EmptyMTIItem found a local malloc so freeing it..\n");
       free(mti->payload);
     }
  }
}

  mti->header.incremental_value=0;
  mti->header.operation_type=0;
  mti->header.var_id=0;
  mti->header.payload_size=0;
  mti->time=0;
  mti->remove=0;
  mti->executed=0;
  mti->incoming=0;
  mti->sent=0;
  mti->payload=0;
  mti->payload_local_malloc=0;
}


void PrintMessageTableItem(struct MessageTableItem * mti,unsigned int val)
{
   return;
   fprintf(stderr,"MessageTableItem Printout (mti index %u) \n",val);
   fprintf(stderr,"mti->header.incremental_value = %u\n",mti->header.incremental_value);
   fprintf(stderr,"mti->header.operation_type = %u ",mti->header.operation_type); PrintMessageType(&mti->header);  fprintf(stderr,"\n");
   fprintf(stderr,"mti->header.var_id = %u\n",mti->header.var_id);
   fprintf(stderr,"mti->time = %u\n",mti->time);
   fprintf(stderr,"mti->remove = %u\n",mti->remove);
   fprintf(stderr,"mti->executed = %u\n",mti->executed);
   fprintf(stderr,"mti->incoming = %u\n",mti->incoming);
   fprintf(stderr,"mti->sent = %u\n",mti->sent);
   fprintf(stderr,"mti->payload = %p\n",mti->payload);
   if (mti->payload!=0)
     {
       unsigned int * payload_val = mti->payload;
       fprintf(stderr,"mti->payload_val = %u\n",*payload_val);
     }
   fprintf(stderr,"mti->payload_size = %u\n",mti->header.payload_size);
   fprintf(stderr,"mti->payload_local_malloc = %u\n",mti->payload_local_malloc);
}


int AllocateMessageQueue(struct MessageTable *  mt,unsigned int total_messages)
{
   if (mt==0) { error("complain"); return 0; }

   mt->locked=0; //New allocation so cleaning it up
   LockMessageTable(mt); // LOCK PROTECTED OPERATION -------------------------------------------


   mt->message_queue_current_length=0;
   mt->table = (struct MessageTableItem *) malloc( total_messages * sizeof(struct MessageTableItem) );
   if (mt->table==0) { error("Could not allocate memory for new Message Table!"); return 0; }


   unsigned int i=0;
   for (i=0; i<total_messages; i++)
    {
      mt->table[i].payload=0;
      mt->table[i].payload_local_malloc=0;
      EmptyMTItem(&mt->table[i],1); // we ignore the payload because it has random garbage initially
    }

   mt->message_queue_total_length=total_messages;

   UnlockMessageTable(mt); // LOCK PROTECTED OPERATION -------------------------------------------

  return 1;
}

int FreeMessageQueue(struct MessageTable * mt)
{
    LockMessageTable(mt); // LOCK PROTECTED OPERATION will never get unlocked :P

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

   free(mt->table);

   return 1;
}

struct failint AddToMessageTable(struct MessageTable * mt,unsigned int incoming,unsigned int free_malloc_at_disposal,struct PacketHeader * header,void * payload,unsigned int msg_timer)
{

  struct failint retres={0};
  retres.failed=0;
  retres.value=0;
  if (mt->message_queue_current_length >= mt->message_queue_total_length) { error("AddToMessageTable complain 1\n"); retres.failed=1; return retres; }


  LockMessageTable(mt); // LOCK PROTECTED OPERATION -------------------------------------------

  fprintf(stderr,"AddToMessageTable incoming = %u , freemalloc_atdispoal = %u , payload = %p , payload_size %u ",incoming,free_malloc_at_disposal,payload,header->payload_size);
  PrintMessageType(header); fprintf(stderr," group = %u time = %u \n",header->incremental_value,msg_timer);
  //usleep(200);

  //TODO APPARENTLY THE TIMING ON THIS CALL HAS SOME IMPORTANCE ( RACE CONDITION )!
  unsigned int mt_pos = mt->message_queue_current_length;
  ++mt->message_queue_current_length;



  EmptyMTItem(&mt->table[mt_pos],0); //<- completely clean spot


  mt->table[mt_pos].header=*header;
  mt->table[mt_pos].incoming=incoming;
  mt->table[mt_pos].time=msg_timer;

  mt->table[mt_pos].payload_local_malloc=free_malloc_at_disposal;
  mt->table[mt_pos].payload=payload;

  PrintMessageTableItem(&mt->table[mt_pos],mt_pos);

  retres.value=mt_pos;

  UnlockMessageTable(mt); // LOCK PROTECTED OPERATION -------------------------------------------

  return retres;
}


int RemFromMessageTable(struct MessageTable * mt,unsigned int mt_id)
{

  if (mt->message_queue_current_length <= mt_id ) { error("RemFromMessageTable called for out of bounds mt_id \n"); return 0; }

  LockMessageTable(mt); // LOCK PROTECTED OPERATION -------------------------------------------


  fprintf(stderr,"RemFromMessageTable -> mt_id %u of %u - type ",mt_id,mt->message_queue_current_length);
  PrintMessageType(&mt->table[mt_id].header);
  fprintf(stderr," group %u \n",mt->table[mt_id].header.incremental_value);


  EmptyMTItem(&mt->table[mt_id],0); // we want to deallocate any mallocs that happen to be local ( 0 second arg )

  if (mt->message_queue_current_length>1)
  {
   int i=0;
   for (i=mt_id; i<mt->message_queue_current_length; i++)
   {
     mt->table[mt_id]=mt->table[mt_id+1];
   }
   EmptyMTItem(&mt->table[mt->message_queue_current_length-1],1); //ignore malloc cause it is a copy
  }


   --mt->message_queue_current_length;

  UnlockMessageTable(mt); // LOCK PROTECTED OPERATION -------------------------------------------
  return 1;
}

int SetMessageTableItemForRemoval(struct MessageTableItem * mti)
{
  if (mti==0) { fprintf(stderr,"SetMessageTableItemForRemoval called with a zero MTI \n"); return 0; }
  mti->remove=1;
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


int RemFromMessageTableWhereRemoveFlagExists(struct MessageTable * mt)
{
  if (mt->message_queue_current_length==0) {return 0;}

  unsigned int mt_id=0;
  unsigned int messages_removed=0;

  // First message hardcoded removal
  if (mt->message_queue_current_length==1)
   {
     if ( mt->table[0].remove ) { if ( RemFromMessageTable(mt,0) ) { ++messages_removed; } }
     if (messages_removed) { fprintf(stderr,"Hardcoded single message removed %u msgs",messages_removed); }
     return messages_removed;
   }



  // If we have more than one messages The rest of the messages starting from the last till the second one!
  for (mt_id=mt->message_queue_current_length-1; mt_id>0; mt_id--)
   {
     if ( mt->table[mt_id].remove )
         {
            if ( RemFromMessageTable(mt,mt_id) ) { ++messages_removed; }
         }
   }
  //Hardcoded last check
  if ( mt->table[0].remove ) { if ( RemFromMessageTable(mt,0) ) { ++messages_removed; } }


  if (messages_removed>0) fprintf(stderr,"RemFromMessageTableWhereRemoveFlagExists removed %u messages \n",messages_removed);

  return messages_removed;
}



struct failint WaitForSuccessIndicatorAtMessageTableItem(struct MessageTable *mt , unsigned int mt_id)
{
  struct failint retres;
  retres.failed=1; retres.value=1;

  if (mt==0) { fprintf(stderr,"WaitForSuccessIndicatorAtMessageTableItem Called with zero MessageTable\n"); return retres;}
  if (mt->message_queue_current_length <= mt_id ) { error("WaitForSuccessIndicatorAtMessageTableItem mt_id out of bounds \n"); return retres; }


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
              retres.failed=0;
              retres.value=mt_traverse;

              return retres;
            }  else
         if (SIGNALMSGFAILURE==mt->table[mt_traverse].header.operation_type)
            {
             fprintf(stderr,"FOUND SIGNALMSGFAILURE!\n");
             return retres;
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
       fprintf(stderr,".SI.");
       usleep(5000);
   }

  fprintf(stderr,"WaitForSuccessIndicatorAtMessageTableItem failed after timeout\n");
  return retres;
}

struct failint WaitForVariableAndCopyItAtMessageTableItem(struct MessageTable *mt , unsigned int mt_id,struct VariableShare *vsh ,unsigned int var_id)
{
  struct failint retres;
  retres.failed=1; retres.value=1;

  if (mt==0) { fprintf(stderr,"WaitForVariableAndCopyItAtMessageTableItem Called with zero MessageTable\n"); return retres;}
  if (mt->message_queue_current_length <= mt_id ) { error("WaitForVariableAndCopyItAtMessageTableItem mt_id out of bounds \n"); return retres; }



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
        // fprintf(stderr,"Found RESP_WRITETO , our inc_value %u header inc_value %u\n",our_incremental_value,mt->table[mt_id].header.incremental_value);
         if (our_incremental_value==mt->table[mt_traverse].header.incremental_value)
            {
              //fprintf(stderr,"Found candidate inc value %u \n",our_incremental_value);
              if ( (var_id==mt->table[mt_traverse].header.var_id) || (vsh->share.variables[var_id].size_of_ptr!=mt->table[mt_traverse].header.payload_size) )
              {
                unsigned int * old_val = (unsigned int *) vsh->share.variables[var_id].ptr;
                unsigned int * new_val = (unsigned int *) mt->table[mt_traverse].payload;
                unsigned int ptr_size = vsh->share.variables[var_id].size_of_ptr;
                fprintf(stderr,"!!!!!!!!!!--> Copying a fresh value for variable %u , was %u now will become %u ( size %u ) \n", var_id,  *old_val,  *new_val, ptr_size);
                memcpy(old_val,new_val,ptr_size);

                /*
                Memory can be deallocated at this point since the message has been copied , this however is not very stable :S
                if (mt->table[mt_traverse].payload_local_malloc)
                 {
                   free(mt->table[mt_traverse].payload);
                 }
                   mt->table[mt_traverse].payload=0;
                   mt->table[mt_traverse].payload_local_malloc=0; */


                retres.failed=0;
                retres.value=mt_traverse;

                return retres;
              } else
              {
                fprintf(stderr,"Mixed or tampered RESP_WRITETO for var_id %u while waiting for var_id %u\n",mt->table[mt_traverse].header.var_id,var_id);
                return retres;
              }

            }
       }

       ++mt_traverse;
     }
       if (mt_traverse>=mt->message_queue_current_length) { mt_traverse=0; }
       fprintf(stderr,".VC.");
       usleep(5000);
   }

  fprintf(stderr,"WaitForVariableAndCopyItAtMessageTableItem failed after timeout\n");
  return retres;
}


struct failint WaitForMessageTableItemToBeSent(struct MessageTable *mt , unsigned int mt_id)
{
  struct failint retres;
  retres.failed=1; retres.value=1;

  if (mt==0) { fprintf(stderr,"WaitForMessageTableItemToBeSent Called with zero MessageTable\n"); return retres;}
  if (mt->message_queue_current_length <= mt_id ) { error("WaitForMessageTableItemToBeSent mt_id out of bounds \n"); return retres; }
 unsigned int done_waiting=0;
  fprintf(stderr,"WaitForMessageTableItemToBeSent  for table item %u , waiting for sent flag \n",mt_id);
  while (!done_waiting)
   {
       if ( mt->table[mt_id].sent )
        {
           retres.failed=0;
           retres.value=mt_id;
           return retres;
        }

       fprintf(stderr,".WS.");
       usleep(1000);
   }

  return retres;
}
