#include "RemoteVariableSupport.h"
#include "MessageTables.h"
#include "helper.h"
#include <sys/errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "SocketAdapterToMessageTables.h"



#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/uio.h>
#include <fcntl.h>


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
  mti->protocol_progress=0;
  mti->last_protocol_id=0;
  mti->time=0;
  mti->remove=0;
  mti->executed=0;
  mti->direction=0;
  mti->sent=0;
  mti->payload=0;
  mti->payload_local_malloc=0;
}


void PrintMessageTableItem(struct MessageTableItem * mti,unsigned int val)
{  return;
   fprintf(stderr,"MessageTableItem Printout (mti index %u) \n",val);
   fprintf(stderr,"mti->header.incremental_value = %u\n",mti->header.incremental_value);
   fprintf(stderr,"mti->header.operation_type = %u ",mti->header.operation_type); PrintMessageType(&mti->header);  fprintf(stderr,"\n");
   fprintf(stderr,"mti->header.var_id = %u\n",mti->header.var_id);
   fprintf(stderr,"mti->protocol_progress = %u\n",mti->protocol_progress);
   fprintf(stderr,"mti->last_protocol_id = %u\n",mti->last_protocol_id);
   fprintf(stderr,"mti->time = %u\n",mti->time);
   fprintf(stderr,"mti->remove = %u\n",mti->remove);
   fprintf(stderr,"mti->executed = %u\n",mti->executed);
   fprintf(stderr,"mti->direction = %u\n",mti->direction);
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
   pthread_mutex_lock (&mt->lock); // LOCK PROTECTED OPERATION -------------------------------------------

   if (mt==0) { error("AllocateMessageQueue called with a zero message table"); return 0; }

    mt->sendrecv_thread=0;
    mt->pause_sendrecv_thread=0;
    mt->stop_sendrecv_thread=0;

    mt->internal_messageproc_thread=0;
    mt->pause_internal_messageproc_thread=1; //We want it to start paused , sendrecv thread will initiate it when needed
    mt->stop_internal_messageproc_thread=0;

    mt->external_messageproc_thread=0;
    mt->pause_external_messageproc_thread=1; //We want it to start paused , sendrecv thread will initiate it when needed
    mt->stop_external_messageproc_thread=0;


   mt->message_queue_current_length=0;
   mt->message_queue_total_length=0;
   mt->table = (struct MessageTableItem *) malloc( (total_messages+1) * sizeof(struct MessageTableItem) );
   if (mt->table==0) { error("Could not allocate memory for new Message Table!"); return 0; }


   unsigned int i=0;
   for (i=0; i<total_messages; i++)
    {
      mt->table[i].header.payload_size=0;
      mt->table[i].payload=0;
      mt->table[i].payload_local_malloc=0;
      EmptyMTItem(&mt->table[i],1); // we ignore the payload because it has random garbage initially
    }

   mt->message_queue_total_length=total_messages;

   pthread_mutex_unlock (&mt->lock); // LOCK PROTECTED OPERATION -------------------------------------------

  return 1;
}

int FreeMessageQueue(struct MessageTable * mt)
{
    //LockMessageTable(mt); // LOCK PROTECTED OPERATION will never get unlocked :P


    mt->pause_sendrecv_thread=0;
    mt->stop_sendrecv_thread=1;

    mt->pause_internal_messageproc_thread=0;
    mt->stop_internal_messageproc_thread=1;

    mt->pause_external_messageproc_thread=0;
    mt->stop_external_messageproc_thread=1;

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

struct failint AddMessage(struct MessageTable * mt,unsigned int direction,unsigned int free_malloc_at_disposal,struct PacketHeader * header,void * payload,unsigned int msg_timer)
{
   pthread_mutex_lock (&mt->lock); // LOCK PROTECTED OPERATION -------------------------------------------

   struct failint retres={0};
   retres.failed=0;
   retres.value=0;


  if (mt->message_queue_current_length >= mt->message_queue_total_length)
    {
      error("AddToMessageTable cannot add new Message to table 1\n");
      fprintf(stderr,"Message table can accomodate %u messages and it has already %u/%u\n",mt->message_queue_total_length,mt->message_queue_current_length,mt->message_queue_total_length);
      retres.failed=1;
      return retres;
    }



  unsigned int mt_pos = mt->message_queue_current_length;
  ++mt->message_queue_current_length;

  fprintf(stderr,"ADDING  %s direction(%u) , freemalloc(%u) , payload = %p , size %u group = %u time = %u , mt_id = %u \n",ReturnPrintMessageTypeVal(header->operation_type),direction,free_malloc_at_disposal,payload,header->payload_size,header->incremental_value,msg_timer,mt_pos);

  //usleep(200);

  //TODO APPARENTLY THE TIMING ON THIS CALL HAS SOME IMPORTANCE ( RACE CONDITION )!




  EmptyMTItem(&mt->table[mt_pos],0); //<- completely clean spot


  mt->table[mt_pos].header=*header;
  mt->table[mt_pos].direction=direction;
  mt->table[mt_pos].time=msg_timer;

  mt->table[mt_pos].payload_local_malloc=free_malloc_at_disposal;
  mt->table[mt_pos].payload=payload;


  retres.value=mt_pos;

   pthread_mutex_unlock (&mt->lock); // LOCK PROTECTED OPERATION -------------------------------------------

  return retres;
}

int SwapMessages(struct MessageTable * mt,unsigned int mt_id1,unsigned int mt_id2)
{
  if ( (mt->message_queue_current_length <= mt_id1 )||
        (mt->message_queue_current_length <= mt_id2 ) )
         { error("SwapItemsMessageTable called for out of bounds mt_id1 or mt_id2 \n"); return 0; }

   if (mt_id1==mt_id2) { return 1;}

   struct MessageTableItem temp;
   temp=mt->table[mt_id1];
   mt->table[mt_id1]=mt->table[mt_id2];
   mt->table[mt_id2]=temp;

   return 1;
}

/*!


        -------------------------------------------------------------------------
        -------------------------------------------------------------------------

        -------------------------------------------------------------------------
        -------------------------------------------------------------------------


     SEND/RECEIVE OPERATIONS

          |
          |
         \ /
          -
!*/



struct failint SendMessageToSocket(int clientsock,struct MessageTable * mt,unsigned int item_num)
{
  struct failint retres={0};
  retres.value=0;
  retres.failed=0;
  //if(sockadap_msg()) fprintf(stderr,"Trying SendPacketPassedToMT message number %u , inc value %u >>>>>>>>>>>>>>>>>>>>>>",item_num,mt->table[item_num].header.incremental_value);
  PrintMessageType(&mt->table[item_num].header);
  fprintf(stderr,"\n");

  if (mt->table[item_num].direction != OUTGOING_MSG)
   {
      fprintf(stderr,"ERROR : Asked SendPacketAndPassToMT to send a non Outgoing message ( %u ) , skipping operation\n",item_num);
      retres.failed=1;
      return retres;
   }


  if(sockadap_msg())
  {
    fprintf(stderr,"SENDING %s mt_id=%u , GROUP %u \n",ReturnPrintMessageTypeVal(mt->table[item_num].header.operation_type), item_num,mt->table[item_num].header.incremental_value);
  }

  int opres=send(clientsock,&mt->table[item_num].header,sizeof(mt->table[item_num].header),MSG_WAITALL);
  if ( opres < 0 ) { fprintf(stderr,"Error %u while SendPacketAndPassToMT \n",errno); retres.failed=1; return retres; }

  if ( mt->table[item_num].header.payload_size > 0 )
   {
     unsigned int * payload_val = (unsigned int *) mt->table[item_num].payload;

     fprintf(stderr,"SENDING %s payload %p , payload_val %u , payload size %u GROUP %u \n",ReturnPrintMessageTypeVal(mt->table[item_num].header.operation_type),mt->table[item_num].payload,*payload_val,mt->table[item_num].header.payload_size,mt->table[item_num].header.incremental_value);

     opres=send(clientsock,mt->table[item_num].payload,mt->table[item_num].header.payload_size,MSG_WAITALL);
     if ( opres < 0 ) { fprintf(stderr,"Error %u while SendPacketAndPassToMT \n",errno); retres.failed=1; return retres; } else
     if ( opres != mt->table[item_num].header.payload_size ) { fprintf(stderr,"Payload was not fully transmitted only %u of %u bytes \n",opres,mt->table[item_num].header.payload_size);
                                                                retres.failed=1; return retres; }

   }

  //This message will trigger will travel to the peer with the send operation but its role is not finished here
  //It will start a protocol action so we set it NOT for removal and NOT executed
  mt->table[item_num].executed=0;
  mt->table[item_num].remove=0;

  //Rest of initialization..
  mt->table[item_num].sent=1;



  retres.value=item_num;
  retres.failed=0;
  return retres;
}

struct failint RecvMessageFromSocket(int clientsock,struct MessageTable * mt,unsigned int msg_timer)
{
  struct failint retres={0};
  retres.value=0;
  retres.failed=0;

  struct PacketHeader header={0};
  int opres=recv(clientsock,&header,sizeof(struct PacketHeader),MSG_WAITALL);
  if (opres<0) { fprintf(stderr,"Error RecvPacketAndPassToMT recv error %u \n",errno); retres.failed=1; return retres; } else
  if (opres!=sizeof(struct PacketHeader)) { fprintf(stderr,"Error RecvPacketAndPassToMT incorrect number of bytes recieved %u \n",opres); retres.failed=1; return retres; }

  if ( header.payload_size > 0 )
   {
    void * payload = (void * ) malloc(header.payload_size);
    if (payload==0) { fprintf(stderr,"Error mallocing %u bytes \n ",header.payload_size); }
    opres=recv(clientsock,payload,header.payload_size,MSG_WAITALL);
    if ( opres < 0 ) { fprintf(stderr,"Error %u while SendPacketAndPassToMT \n",errno); retres.failed=1; return retres; } else
    if ( opres != header.payload_size ) { fprintf(stderr,"Payload was not fully received only %u of %u bytes \n",opres,header.payload_size); retres.failed=1; return retres; }

    unsigned int * payload_val=(unsigned int * ) payload;

    retres = AddMessage(mt,INCOMING_MSG,1,&header,payload,msg_timer);
    if(sockadap_msg())
     {
        fprintf(stderr,"RECEIVED %s - GROUP %u - With %u bytes of payload ----------------\n",ReturnPrintMessageTypeVal(header.operation_type),header.incremental_value,header.payload_size);
     }
    return retres;
   } else
   {
     if(sockadap_msg())
      {
        fprintf(stderr,"RECEIVED %s - GROUP %u - with NO payload ----------------\n",ReturnPrintMessageTypeVal(header.operation_type),header.incremental_value);
      }
   }

  retres = AddMessage(mt,INCOMING_MSG,0,&header,0,msg_timer);
  return retres;
}

/*!


        -------------------------------------------------------------------------
        -------------------------------------------------------------------------

        -------------------------------------------------------------------------
        -------------------------------------------------------------------------


     REMOVE / FLAG SET  OPERATIONS

          |
          |
         \ /
          -
!*/



int RemMessageINTERNAL_MUST_BE_LOCKED(struct MessageTable * mt,unsigned int mt_id)
{
  if (mt->message_queue_current_length <= mt_id ) { error("RemFromMessageTableKeepOrder called for out of bounds mt_id \n"); return 0; }
  if (mt->message_queue_current_length==0) { fprintf(stderr,"Erroneous call at RemFromMessageTableKeepOrder for item %u , it must have been removed by another thread\n",mt_id); }



  // PRINT THE REM MESSAGE TABLE OPERATION
  fprintf(stderr,"REMOVING %s GROUP %u  mt_id %u/%u \n",ReturnPrintMessageTypeVal(mt->table[mt_id].header.operation_type),mt->table[mt_id].header.incremental_value,mt_id,mt->message_queue_current_length-1);


  if (mt->message_queue_current_length==1)
   {
      EmptyMTItem(&mt->table[mt_id],0);
      mt->message_queue_current_length=0;
   } else
  if (mt->message_queue_current_length>1)
   {
    if (mt_id<mt->message_queue_current_length-1)
     {
       //We are not in the last position , we need to move some things around...
       int i=0;
       for (i=mt_id; i<mt->message_queue_current_length; i++) { mt->table[i]=mt->table[i+1]; } // This "bug" took me 2 days to find out :P , this line was mt->table[mt_id]=mt->table[mt_id+1];

       EmptyMTItem(&mt->table[mt->message_queue_current_length-1],1); //ignore malloc cause it is a copy
       --mt->message_queue_current_length;
     } else
     {
       //We are in the last position..
       EmptyMTItem(&mt->table[mt_id],0);
        --mt->message_queue_current_length;
     }
   }



  return 1;
}



int SetMessage_Flag_ForRemoval(struct MessageTableItem * mti)
{
  if (mti==0) { fprintf(stderr,"SetMessageTableItemForRemoval called with a zero MTI \n"); return 0; }
  mti->remove=1;
  return 1;
}

int SetAllMessagesOfGroup_Flag_ForRemoval(struct MessageTable * mt,unsigned int groupid)
{
  if (mt==0) {return 0;}
  if (mt->message_queue_total_length==0) {return 0;}
  if (mt->message_queue_current_length==0) {return 0;}

  unsigned int mt_id=0;
  for (mt_id=0; mt_id < mt->message_queue_current_length; mt_id++)
     {
       if (mt->table[mt_id].header.incremental_value==groupid)
                              {
                                mt->table[mt_id].remove=1;
                                fprintf(stderr,"MARKED REMOVE %u/%u %s \n",mt_id,mt->message_queue_current_length-1,ReturnPrintMessageTypeVal(mt->table[mt_id].header.operation_type));
                              }
     }

  return 1;
}


int RemFromMessageTableWhereRemoveFlagExists(struct MessageTable * mt)
{
  if (mt==0) {return 0;}
  if (mt->message_queue_total_length==0) {return 0;}
  if (mt->message_queue_current_length==0) {return 0;}

  pthread_mutex_lock (&mt->lock);  // LOCK PROTECTED OPERATION -------------------------------------------

  unsigned int mt_id=0;
  unsigned int messages_removed=0;

   for (mt_id=0; mt_id < mt->message_queue_current_length; mt_id++)
     {
       if ( mt->table[mt_id].remove )
          {
             if ( RemMessageINTERNAL_MUST_BE_LOCKED(mt,mt_id) ) { ++messages_removed; }
          }
     }

   pthread_mutex_unlock (&mt->lock); // LOCK PROTECTED OPERATION -------------------------------------------
  return messages_removed;
}


/*!


        -------------------------------------------------------------------------
        -------------------------------------------------------------------------

        -------------------------------------------------------------------------
        -------------------------------------------------------------------------


     WAIT OPERATIONS

          |
          |
         \ /
          -
!*/



unsigned int WaitForMessageTableItemToBeSent(struct MessageTableItem * mti)
{
  while (!mti->sent)
   {
     usleep(10); fprintf(stderr,".WS.");
   }
   return 1;
}


struct failint WaitForMessage(struct MessageTable *mt , unsigned char optype1 , unsigned char optype2 , unsigned int inc_value , unsigned int direction , unsigned int wait_forever)
{
    struct failint retres; retres.failed=1; retres.value=0;
    if (mt==0) { fprintf(stderr,"WaitForMessage Called with zero MessageTable\n"); return retres;}
    if ((mt->message_queue_current_length == 0)&&(!wait_forever)) { /*No messages , wont wait*/ return retres; }



    unsigned int done_waiting=0;
    unsigned int mt_traverse=0;

   while (!done_waiting)
   {
     if ( !wait_forever ) { done_waiting=1; } // Wait forever ( until message comes .. ) :P

  //   pthread_mutex_lock (&mt->lock); // LOCK PROTECTED OPERATION -------------------------------------------
     for (mt_traverse=0; mt_traverse<mt->message_queue_current_length; mt_traverse++)
     {
       if ( (optype1==mt->table[mt_traverse].header.operation_type) &&
            (inc_value==mt->table[mt_traverse].header.incremental_value)&&
            (direction==mt->table[mt_traverse].direction)  )
       {
         fprintf(stderr,"\nFOUND1 %s , group %u !\n",ReturnPrintMessageTypeVal(mt->table[mt_traverse].header.operation_type),inc_value);
         retres.failed=0; retres.value=mt_traverse; return retres;
       }
        else //Todo 2 loops one when optype1==optype2 , one which does both..!
       if ( (optype2==mt->table[mt_traverse].header.operation_type) &&
            (inc_value==mt->table[mt_traverse].header.incremental_value) &&
            (direction==mt->table[mt_traverse].direction) )
       {
         fprintf(stderr,"\nFOUND2 %s , group %u !\n",ReturnPrintMessageTypeVal(mt->table[mt_traverse].header.operation_type),inc_value);
         retres.failed=2; retres.value=mt_traverse; return retres;
       }
     }
   //  pthread_mutex_unlock (&mt->lock); // LOCK PROTECTED OPERATION -------------------------------------------

     usleep(500);
     fprintf(stderr,"*");
   }

   return retres;
}

struct failint WaitForSuccessIndicatorAtMessageTableItem(struct MessageTable *mt , unsigned int mt_id, unsigned int wait_forever)
{
  unsigned int group=mt->table[mt_id].header.incremental_value;
  fprintf(stderr,"WAITING for SIGNALMSGSUCCESS or SIGNALMSGFAILURE  GROUP %u , mt_id=%u , mt_length = %u \n",group,mt_id,mt->message_queue_current_length);
  struct failint retres=  WaitForMessage(mt ,SIGNALMSGSUCCESS,SIGNALMSGFAILURE,group,1 /*INCOMING*/,wait_forever);
  if (wait_forever) fprintf(stderr,"DONE WAITING for SIGNALMSGSUCCESS or SIGNALMSGFAILURE  GROUP %u , mt_id=%u , mt_length = %u \n",group,mt_id,mt->message_queue_current_length);

  return retres;
}

struct failint WaitForVariableAndCopyItAtMessageTableItem(struct MessageTable *mt , unsigned int mt_id,struct VariableShare *vsh ,unsigned int var_id, unsigned int wait_forever)
{
  unsigned int group=mt->table[mt_id].header.incremental_value;
  fprintf(stderr,"WAITING for RESP_WRITETO  GROUP %u , mt_id=%u , mt_length = %u \n",group,mt_id,mt->message_queue_current_length);
  struct failint retres= WaitForMessage(mt,RESP_WRITETO,RESP_WRITETO,group,1 /*INCOMING*/,wait_forever);
  if (wait_forever) fprintf(stderr,"DONE WAITING for RESP_WRITETO  GROUP %u , mt_id=%u , mt_length = %u \n",group,mt_id,mt->message_queue_current_length);

  if (!retres.failed)
   {
     unsigned int mt_respwriteto = retres.value;
     if ( (var_id==mt->table[mt_respwriteto].header.var_id) || (vsh->share.variables[var_id].size_of_ptr!=mt->table[mt_respwriteto].header.payload_size) )
     {
         unsigned int * old_val = (unsigned int *) vsh->share.variables[var_id].ptr;
         unsigned int * new_val = (unsigned int *) mt->table[mt_respwriteto].payload;

         if (old_val==0) { fprintf(stderr,"\nERROR : WaitForVariableAndCopyItAtMessageTableItem old_value memory points to zero \n");  retres.failed=1; return retres; }
         else if (new_val==0) { fprintf(stderr,"\nERROR : WaitForVariableAndCopyItAtMessageTableItem new_value memory points to zero \n");  retres.failed=1; return retres; }
         else if (old_val==new_val) {fprintf(stderr,"\nERROR : WaitForVariableAndCopyItAtMessageTableItem copy target memory the same with source\n");  retres.failed=1; return retres;}
         else {
               unsigned int ptr_size = vsh->share.variables[var_id].size_of_ptr;
               fprintf(stderr,"\n!!!!!!!!!!--> Copying a fresh value for variable %u , was %u now will become %u ( size %u ) \n", var_id,  *old_val,  *new_val, ptr_size);
               memcpy(old_val,new_val,ptr_size);
               // Memory can be deallocated at this point since the message has been copied , this however is not very stable :S
               //    if (mt->table[mt_respwriteto].payload_local_malloc) { free(mt->table[mt_respwriteto].payload); }
               //    mt->table[mt_respwriteto].payload=0;
               //    mt->table[mt_respwriteto].payload_local_malloc=0;
               return retres;
              }
     }
       else
     {
        fprintf(stderr,"\nMixed or tampered RESP_WRITETO for var_id %u while waiting for var_id %u\n",mt->table[mt_respwriteto].header.var_id,var_id);
        retres.failed=1;
        return retres;
     }
   } else
   {
       if (wait_forever) fprintf(stderr,"ERROR ,FAILED WAITING for RESP_WRITETO  GROUP %u , mt_id=%u , mt_length = %u \n",group,mt_id,mt->message_queue_current_length);
   }

  return retres;
}
