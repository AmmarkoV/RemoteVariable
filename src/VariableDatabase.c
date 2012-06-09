/***************************************************************************
* Copyright (C) 2010 by Ammar Qammaz *
* ammarkov@gmail.com *
* *
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or *
* (at your option) any later version. *
* *
* This program is distributed in the hope that it will be useful, *
* but WITHOUT ANY WARRANTY; without even the implied warranty of *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the *
* GNU General Public License for more details. *
* *
* You should have received a copy of the GNU General Public License *
* along with this program; if not, write to the *
* Free Software Foundation, Inc., *
* 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. *
***************************************************************************/
#include "VariableDatabase.h"
#include "helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "HashFunctions.h"
#include "MessageTables.h"
#include "Peers.h"

int VariableShareOk(struct VariableShare * vsh)
{
 if ( vsh == 0 ) { error("Variable Share not ok!"); return 0; }
 if ( vsh->share.variables == 0 ) { error("Variable Share not ok , no initialized space for variables!"); return 0; }
 return 1;
}

struct VariableShare * Create_VariableDatabase(char * sharename,char * IP,unsigned int port,char * password,unsigned int newsize)
{
  struct VariableShare * vsh=0;

  vsh = (struct VariableShare *) malloc(sizeof(struct VariableShare));
  if (vsh==0) { error("Could not allocate memory for share!"); return 0; }

  vsh->share.variables = (struct ShareListItem * ) malloc( newsize * sizeof(struct ShareListItem) );
  if (vsh->share.variables==0) { free(vsh); // ROLL BACK
                                 error("Could not allocate memory for share variables!"); return 0; }

  strncpy(vsh->sharename,sharename,RVS_MAX_SHARE_NAME_CHARS);

  strncpy(vsh->ip,IP,RVS_MAX_SHARE_IP_CHARS);
  vsh->port=port;

  vsh->global_state=0;
  vsh->global_policy=0;

  vsh->share.auto_refresh_every_msec=RVS_VARIABLEDATABASE_THREAD_TIME;
  vsh->share.total_variables_memory=newsize;
  vsh->share.total_variables_shared=0;

  vsh->total_peers=0;

  vsh->refresh_thread=0;
  vsh->client_thread=0;
  vsh->server_thread=0;

  vsh->central_timer=0;
  vsh->byte_order=0;


  pthread_mutex_init(&vsh->refresh_lock, 0); //New allocation so cleaning up mutex

  return vsh;
}


int Destroy_VariableDatabase(struct VariableShare * vsh)
{
  if (vsh==0) { debug_say("Memory for share , already deallocated!"); return 1; }
  if ( vsh->share.variables == 0 ) { debug_say("Memory for shared variables , already deallocated!"); return 1; }

  int i=0;
  for (i=0; i<vsh->total_peers; i++)
   {
     close(vsh->peer_list[i].socket_to_client);
     vsh->peer_list[i].socket_to_client=0;
   }

   if (vsh->this_address_space_is_master) { close(vsh->master.socket_to_client); vsh->master.socket_to_client=0; }

  //TODO CLOSE SOCKETS ETC PROPERLY
    vsh->pause_refresh_thread=0;
    vsh->stop_refresh_thread=1;

    vsh->pause_client_thread=0;
    vsh->stop_client_thread=1;

    vsh->pause_server_thread=0;
    vsh->stop_server_thread=1;

  vsh->share.total_variables_memory=0;
  vsh->share.total_variables_shared=0;

  pthread_mutex_destroy(&vsh->refresh_lock);

  free(vsh->share.variables);
  free(vsh);

  usleep(10000);

  return 1;
}


int Resize_VariableDatabase(struct VariableShare * vsh , unsigned int newsize)
{
  // TODO TODO TODO
  error("The Variable Share Object has run out of space and there is no code implemented for resizing it (yet) !");
  return 0;
}


unsigned long GetVariableHash(struct VariableShare * vsh,void * ptr,unsigned int size_of_ptr)
{
  if (size_of_ptr<sizeof(unsigned long)) {
                                                                           /*The whole variable fits inside the unsigned long so no hash is required*/
                                                                            unsigned long * stacklongptr = ptr;
                                                                            unsigned long stacklong = *stacklongptr;
                                                                           /*fprintf(stderr,"GetVariableHash for var %u returning %u\n",var_id,stacklong);*/
                                                                            return stacklong;
                                                                         }
  debug_say("TODO: ADD code that produces hash on variables that do not fit unsigned long!\n");
  /*hash(unsigned char *str);*/
  return 0;
}
unsigned long GetVariableHashForVar(struct VariableShare * vsh,unsigned int var_id)
{
  return GetVariableHash(vsh,
                          vsh->share.variables[var_id].ptr,
                          vsh->share.variables[var_id].size_of_ptr);
}



int AddVariable_Database(struct VariableShare * vsh,char * var_name,unsigned int permissions,volatile void * ptr,unsigned int ptr_size)
{
 if ( VariableShareOk(vsh) == 0 ) { fprintf(stderr,"Error could not add %s var to database \n",var_name); return 0; }

 unsigned int * ptr_val = (unsigned int * ) ptr;
 fprintf(stderr,"AddVariable_Database , var_name = %s ( %u chars ) , ptr value = %p ptr addr = %p , ptr_size = %u \n",var_name,strlen(var_name),*ptr_val,ptr,ptr_size);

 unsigned int spot_to_take=vsh->share.total_variables_memory+1;
 if (vsh->share.total_variables_memory > vsh-> share.total_variables_shared )
  { // WE HAVE SPACE TO ADD A NEW SHARE DATA!
     spot_to_take=vsh->share.total_variables_shared;
  } else
  {
    // NO FREE SPACE RESIZE CODE HERE !
    if ( Resize_VariableDatabase(vsh,vsh->share.total_variables_memory+128) == 0 ) { return 0; }
  }

  if (spot_to_take < vsh->share.total_variables_memory)
  {
    unsigned int var_length = strlen(var_name);
    vsh->share.variables[spot_to_take].ptr_name_length  = var_length;
    if ( var_length>=32 ) { fprintf(stderr,"Var %s is too big , trimming it \n",var_name); var_length=32; }
    memset(vsh->share.variables[spot_to_take].ptr_name,0,32);
    strncpy(vsh->share.variables[spot_to_take].ptr_name,var_name,var_length);



    vsh->share.variables[spot_to_take].ptr=ptr;
    vsh->share.variables[spot_to_take].size_of_ptr=ptr_size;
    vsh->share.variables[spot_to_take].hash=GetVariableHashForVar(vsh,spot_to_take);
    vsh->share.variables[spot_to_take].last_signaled_hash=vsh->share.variables[spot_to_take].hash;

    vsh->share.variables[spot_to_take].last_write_time=0;
    vsh->share.variables[spot_to_take].permissions=permissions;


    vsh->share.variables[spot_to_take].flag_needs_refresh_from_sock=0;


    vsh->share.variables[spot_to_take].GUARD_BYTE1 = RVS_GUARD_VALUE ;
    vsh->share.variables[spot_to_take].GUARD_BYTE2 = RVS_GUARD_VALUE ;

    vsh->share.total_variables_shared+=1;
  }

 return 1;
}

int DeleteVariable_Database(struct VariableShare * vsh,char * var_name)
{
  // TODO TODO TODO
  error("The Variable Share Object wants to delete an item and there is no code implemented for deleting it (yet) !");
  return 0;
}

struct failint FindVariable_Database(struct VariableShare * vsh,char * var_name)
{
 struct failint retres={0};

 if ( VariableShareOk(vsh) == 0 ) { retres.failed=1; return retres; }

 int i;
 for ( i=0; i<vsh->share.total_variables_shared; i++)
  {
     if ( strcmp(vsh->share.variables[i].ptr_name,var_name)==0 )
      {
        retres.value=i;
        return retres;
      }
  }

 retres.failed=1;
 return retres;
}


int VariableIdExists(struct VariableShare * vsh,unsigned int var_id)
{
  if (vsh->share.total_variables_shared<=var_id) { return 0; }
  return 1;
}

int MarkVariableAsNeedsRefresh_VariableDatabase(struct VariableShare * vsh,unsigned int var_id,int clientsock)
{
   if (!VariableIdExists(vsh,var_id)) { fprintf(stderr,"Variable addressed ( %u ) by MarkVariableAsNeedsRefresh_VariableDatabase does not exist \n",var_id); return 0; }
   fprintf(stderr,"Variable %u has been marked , as \"needs refresh\" \n",var_id);
   vsh->share.variables[var_id].flag_needs_refresh_from_sock = clientsock;
   return 1;
}

int IfLocalVariableChanged_SignalUpdateToJoblist(struct VariableShare * vsh,unsigned int var_id)
{
  if (!VariableIdExists(vsh,var_id)) { fprintf(stderr,"Variable addressed ( %u ) by IfLocalVariableChanged_SignalUpdateToJoblist does not exist \n",var_id); return 0; }
  unsigned long current_hash=vsh->share.variables[var_id].hash;//GetVariableHashForVar(vsh,var_id);

  if (current_hash!=vsh->share.variables[var_id].last_signaled_hash)
    {
         printf("Variable Changed Hash for variable %u , last hash change transmitted %u , now we have %u !\n",var_id,(unsigned int) vsh->share.variables[var_id].last_signaled_hash , (unsigned int) current_hash);
         fprintf(stderr,"Variable Changed Hash for variable %u , last hash change transmitted %u , now we have %u !\n",var_id,(unsigned int) vsh->share.variables[var_id].last_signaled_hash , (unsigned int) current_hash);
         //vsh->share.variables[var_id].last_signaled_hash=current_hash;


         unsigned int failed_transmissions=0;
         unsigned int successfull_transmissions=0;
         unsigned int i=0;
         struct failint internal_msg={0};

           struct PacketHeader header={0};
           header.incremental_value=0;
           header.operation_type=INTERNAL_START_SIGNALCHANGED;
           header.var_id=var_id;
           header.payload_size=0;

         for (i=0; i< vsh->total_peers; i++)
         {
           fprintf(stderr,"Singaling LocalVariableChanged broadcasting to peer number %u \n",i);

           if ( MessageExists(&vsh->peer_list[i].messages,var_id,INTERNAL_START_READFROM,INCOMING_MSG) )
             {
               fprintf(stderr,"We have a pending READFROM for var %u so no point in resignaling var change\n",var_id);
               printf("We have a pending READFROM for var %u so no point in resignaling var change\n",var_id);
               ++failed_transmissions;
             } else
          if (! MessageExists(&vsh->peer_list[i].messages,var_id,INTERNAL_START_SIGNALCHANGED,INCOMING_MSG) )
             {
               unsigned int * val =  vsh->share.variables[var_id].ptr;
               fprintf(stderr,"Creating INTERNAL_START_SIGNALCHANGED for var %u ( val %u ) , broadcasted to peer %u \n",var_id,*val,i);
                //vsh->share.variables[var_id].hash=current_hash; /*We keep the new hash as the current hash :)*/
                vsh->share.variables[var_id].last_signaled_hash=current_hash;
                vsh->share.variables[var_id].this_hash_transmission_count=0;
                internal_msg=AddMessage(&vsh->peer_list[i].messages,INCOMING_MSG,0,&header,0);
               if (!internal_msg.failed) { ++successfull_transmissions;  ++vsh->share.variables[var_id].this_hash_transmission_count; } else
                                         { ++failed_transmissions; }
             }
         }
    }
  return 0;
}


int CheckForChangedVariables(struct VariableShare * vsh)
{
 if ( vsh->share.total_variables_shared == 0 ) { return 0; /* NO VARIABLES TO SHARE OR UPDATE!*/}
 int retres=0;
 unsigned int var_id=0;
 unsigned long live_hash=0;
 //fprintf(stderr,"Refreshing %u variables!\n",vsh->share.total_variables_shared);
 for ( var_id=0; var_id<vsh->share.total_variables_shared; var_id++)
  {
     live_hash = GetVariableHashForVar(vsh,var_id);
     if (live_hash != vsh->share.variables[var_id].hash )
      {
        fprintf(stderr,"Variable %u changed!\n");
        vsh->share.variables[var_id].hash=live_hash;
        ++retres;
      }
  }

  return retres;
}



int SignalUpdatesForAllLocalVariablesThatNeedIt(struct VariableShare * vsh)
{
 if ( vsh->share.total_variables_shared == 0 ) { return 0; /* NO VARIABLES TO SHARE OR UPDATE!*/}
 int retres=0;
 unsigned int i=0;
 //fprintf(stderr,"Refreshing %u variables!\n",vsh->share.total_variables_shared);
 for ( i=0; i<vsh->share.total_variables_shared; i++)
  {
     if ( IfLocalVariableChanged_SignalUpdateToJoblist(vsh,i) ) { ++retres; /*usleep(100);*/ }
  }

  return retres;
}

int NewValueForVariable(struct VariableShare * vsh,unsigned int var_id,void * new_val,unsigned int new_val_size,unsigned int time)
{
  unsigned int * old_val = (unsigned int *) vsh->share.variables[var_id].ptr;
 //TODO: Add locking for variable ..! :)
  if (old_val==0) { fprintf(stderr,"\nERROR : NewValueForVariable old_value memory points to zero \n");  return 0; }
  else if (new_val==0) { fprintf(stderr,"\nERROR : NewValueForVariable new_value memory points to zero \n");  return 0; }
  else if (old_val==new_val) {fprintf(stderr,"\nERROR : NewValueForVariable copy target memory the same with source\n");  return 0;}
  else if (new_val_size!=vsh->share.variables[var_id].size_of_ptr) { fprintf(stderr,"\nERROR : NewValueForVariable new_value memory points to zero \n");  return 0; }
  else {

         /*unsigned long newhash=GetVariableHashForVar(vsh,var_id);
         if (newhash!=vsh->share.variables[var_id].hash )
         {
           printf("The value changed locally before delivery,new value is irrelevant!\n");
           fprintf(stderr,"The value changed locally before delivery,new value is irrelevant!\n");
           IfLocalVariableChanged_SignalUpdateToJoblist(vsh,var_id);
           return 0;
         }*/

         unsigned int ptr_size = vsh->share.variables[var_id].size_of_ptr;
         unsigned int * new_val_int = (unsigned int * ) new_val;
         fprintf(stderr,"Var %s ( id %u ) %u now becomes %u time %u\n",vsh->share.variables[var_id].ptr_name , var_id ,*old_val,  *new_val_int , vsh->share.variables[var_id].last_write_time);
         printf("Var %s ( id %u ) %u now becomes %u time %u\n",vsh->share.variables[var_id].ptr_name , var_id ,*old_val,  *new_val_int,vsh->share.variables[var_id].last_write_time);

         if (mutex_msg()) fprintf(stderr,"Waiting for mutex NewValueForVariable\n");
         pthread_mutex_lock (&vsh->refresh_lock); // LOCK PROTECTED OPERATION -------------------------------------------
         if (mutex_msg()) fprintf(stderr,"Entered mutex NewValueForVariable\n");

         memcpy(old_val,new_val,ptr_size);
         vsh->share.variables[var_id].hash=GetVariableHashForVar(vsh,var_id);
         vsh->share.variables[var_id].last_write_time = time;

         pthread_mutex_unlock (&vsh->refresh_lock); // LOCK PROTECTED OPERATION -------------------------------------------

         fprintf(stderr,"Updated hash value for new payload ( %u ) , hash = %u \n",*new_val_int,(unsigned int ) vsh->share.variables[var_id].hash);
         return 1;
        }


   return 0;
}


int RefreshAllVariablesThatNeedIt(struct VariableShare *vsh)
{
   unsigned int added_jobs=0;
   unsigned int var_id=0;

    for ( var_id=0; var_id<vsh->share.total_variables_shared; var_id++)
   {
      if ( vsh->share.variables[var_id].flag_needs_refresh_from_sock > 0 )
        {
           fprintf(stderr,"Detected that a variable (%u) needs refresh , and automatically adding a job to receive it\n",var_id);

           struct failint internal_msg={0};
           struct failint peer_find={0};

           struct PacketHeader header={0};
           header.operation_type=INTERNAL_START_READFROM;
           header.var_id=var_id;
           header.incremental_value=0;
           header.payload_size=0;

           peer_find = GetPeerIdBySock(vsh,vsh->share.variables[var_id].flag_needs_refresh_from_sock);
           if (peer_find.failed)
           {
            fprintf(stderr,"ERROR Resolving from socket for peer id!\n");
           } else
           if (! MessageExists(&vsh->peer_list[peer_find.value].messages,var_id,INTERNAL_START_READFROM,INCOMING_MSG) )
             {
               unsigned int * val =  vsh->share.variables[var_id].ptr;
               //printf("Creating INTERNAL_START_READFROM for var %u ( val %u ) broadcasted to peer %u\n",var_id,*val,peer_find.value);
               fprintf(stderr,"Creating INTERNAL_START_READFROM for var %u ( val %u ) broadcasted to peer %u\n",var_id,*val,peer_find.value);
               internal_msg=AddMessage(&vsh->peer_list[peer_find.value].messages,INCOMING_MSG,0,&header,0);

               if (!internal_msg.failed) { vsh->share.variables[var_id].flag_needs_refresh_from_sock=0; /* We trust that the "add job" will do its job :P*/ }
             }
        }
   }


  return added_jobs;
}

int MakeSureVarReachedPeers(struct VariableShare *vsh,char * varname,unsigned int wait_time)
{
  fprintf(stderr,"MakeSureVarReachedPeers waiting for var %s \n",varname);

  struct failint res = FindVariable_Database(vsh,varname);
  if (res.failed) { fprintf(stderr,"MakeSureVarReachedPeers called for non existing variable ( %s ) \n",varname); return 0; }
  unsigned int var_id  = res.value;


  if ( SignalUpdatesForAllLocalVariablesThatNeedIt(vsh) ) // <- this will make any variables that have just changed be set as flag_needs_refres_from_sock
   {
       fprintf(stderr,"Found changed variables..!\n");
   }

  while (wait_time>0)
  {

      if ( (vsh->share.variables[var_id].this_hash_transmission_count == vsh->total_peers ) &&
           (vsh->share.variables[var_id].flag_needs_refresh_from_sock==0)  /*add message table counter here maybe*/)

                          { return 1; }

      --wait_time;
      usleep (1000);
  }

  fprintf(stderr,"Timed out while MakeSureVarReachedPeers waiting for var to reach clients\n");
  return 0;
}


void *
AutoRefreshVariable_Thread(void * ptr)
{
  debug_say("AutoRefresh Thread: AutoRefresh Thread started..\n");
  struct VariableShare *vsh=0;
  vsh = (struct VariableShare *) ptr;

   unsigned int total_variables_changed=0;
   unsigned int variables_changed=0,variables_signaled=0,variables_refreshed=0;
   while ( (vsh->global_policy==VSP_AUTOMATIC) && (vsh->stop_refresh_thread==0) )
   {
       usleep(vsh->share.auto_refresh_every_msec);

       if (vsh->pause_refresh_thread)
          {
            // This means that auto refresh is disabled so we dont do anything until is re-enabled
          } else
          {
             if (mutex_msg()) fprintf(stderr,"Waiting for mutex AutoRefreshVariable_Thread\n");
             pthread_mutex_lock (&vsh->refresh_lock); // LOCK PROTECTED OPERATION -------------------------------------------
             if (mutex_msg()) fprintf(stderr,"Entered mutex AutoRefreshVariable_Thread\n");

             variables_changed=CheckForChangedVariables(vsh);
             total_variables_changed+=variables_changed;


             variables_signaled=SignalUpdatesForAllLocalVariablesThatNeedIt(vsh);
             total_variables_changed+=variables_signaled;

             variables_refreshed=RefreshAllVariablesThatNeedIt(vsh);
             pthread_mutex_unlock (&vsh->refresh_lock); // LOCK PROTECTED OPERATION -------------------------------------------

             if ( (variables_changed==0 ) && ( variables_signaled == 0) && ( variables_refreshed == 0) ) { /*fprintf(stderr,".");*/ } else
                                                                           { fprintf(stderr,"AutoRefresh Thread: %u vars changed | %u vars signaled | %u vars refreshed\n",variables_changed,variables_signaled,variables_refreshed); }
          }
   }
   fprintf(stderr,"AutoRefresh Thread: %u total variables changed detected by autorefresh thread\n",total_variables_changed);

   return 0;
}

void AutoRefreshVariable_Thread_Pause(struct VariableShare * vsh)
{
  vsh->pause_refresh_thread=1;
}

void AutoRefreshVariable_Thread_Resume(struct VariableShare * vsh)
{
  vsh->pause_refresh_thread=0;
}

/* THREAD STARTER */
int StartAutoRefreshVariable(struct VariableShare * vsh)
{
  vsh->stop_refresh_thread=0;
  vsh->pause_refresh_thread=0;
  int retres = pthread_create( &vsh->refresh_thread, 0,  AutoRefreshVariable_Thread ,(void*) vsh);

  if (retres!=0) retres = 0; else retres = 1;
  return retres;
}
