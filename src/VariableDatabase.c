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
#include "JobTables.h"

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

/*
  unsigned int i=0;
  for (i=0; i<RVS_MAX_JOBS_PENDING; i++)
   {
      vsh->jobs_list[i]
   }*/

  vsh->jobs_loaded=0;
  vsh->total_jobs_done=0;

  vsh->job_thread=0;
  vsh->refresh_thread=0;
  vsh->client_thread=0;
  vsh->server_thread=0;

  vsh->central_timer=0;
  vsh->byte_order=0;
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
    vsh->pause_job_thread=0;
    vsh->stop_job_thread=1;

    vsh->pause_refresh_thread=0;
    vsh->stop_refresh_thread=1;

    vsh->pause_client_thread=0;
    vsh->stop_client_thread=1;

    vsh->pause_server_thread=0;
    vsh->stop_server_thread=1;

  vsh->share.total_variables_memory=0;
  vsh->share.total_variables_shared=0;

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

unsigned long GetVariableHash(struct VariableShare * vsh,unsigned int var_id)
{
  if (vsh->share.variables[var_id].size_of_ptr<sizeof(unsigned long)) {
                                                                           /*The whole variable fits inside the unsigned long so no hash is required*/
                                                                            unsigned long * stacklongptr = vsh->share.variables[var_id].ptr;
                                                                            unsigned long stacklong = *stacklongptr;
                                                                           /*fprintf(stderr,"GetVariableHash for var %u returning %u\n",var_id,stacklong);*/
                                                                            return stacklong;
                                                                         }
  debug_say("Todo ADD code that produces hash on variables that do not fit unsigned long!\n");
  /*hash(unsigned char *str);*/
  return 0;
}

int AddVariable_Database(struct VariableShare * vsh,char * var_name,unsigned int permissions,void * ptr,unsigned int ptr_size)
{
 if ( VariableShareOk(vsh) == 0 ) { fprintf(stderr,"Error could not add %s var to database \n",var_name); return 0; }

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
    if ( var_length>=32 ) { fprintf(stderr,"Var %s is too big , trimming it \n",var_name); var_length=32; }
    strncpy(vsh->share.variables[spot_to_take].ptr_name,var_name,var_length);


    vsh->share.variables[spot_to_take].ptr=ptr;
    vsh->share.variables[spot_to_take].size_of_ptr=ptr_size;
    vsh->share.variables[spot_to_take].hash=GetVariableHash(vsh,spot_to_take);

    vsh->share.variables[spot_to_take].last_write_inc=0;
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

unsigned int FindVariable_Database(struct VariableShare * vsh,char * var_name)
{
 if ( VariableShareOk(vsh) == 0 ) return 0;

 int i;
 for ( i=0; i<vsh->share.total_variables_shared; i++)
  {
     if ( strcmp(vsh->share.variables[i].ptr_name,var_name)==0 )
      {
        return i+1;
      }
  }
 return 0;
}

/*
int CanWriteTo_VariableDatabase(struct VariableShare * vsh,unsigned int var_spot)
{
 if ( VariableShareOk(vsh) == 0 ) return 0;
 if ( var_spot>=vsh->share.total_variables_shared ) return 0;

 debug_say(" AND code for write permissions not sure , skipping for now");
 return 1;
 if ( vsh->share.variables[var_spot].permissions && 2 ) return 1;
 return 0;
}

int CanReadFrom_VariableDatabase(struct VariableShare * vsh,unsigned int var_spot)
{
 if ( VariableShareOk(vsh) == 0 ) return 0;
 if ( var_spot>=vsh->share.total_variables_shared ) return 0;

 debug_say(" AND code for read permissions not sure , skipping for now");
 return 1;
 if ( vsh->share.variables[var_spot].permissions && 4 ) return 1;
 return 0;
}

int RefreshLocalVariable_VariableDatabase(struct VariableShare * vsh,char * variable_name)
{
   debug_say(" RefreshLocalVariable_VariableDatabase , not implemented ");
   int var_id = FindVariable_Database(vsh,variable_name);
   if ( var_id == 0 )
    {
      fprintf(stderr,"Variable %s not found , cannot be refreshed to local\n",variable_name);
      return 0;
    } else
    {
      --var_id; // FindVariableDatabase returns +1 results ( to signal failure with 0 )
      //TODO
      //vsh->share.variables[var_id].
      //UpdateLocalVariable(vsh,  our_varid, peer_varid);
    }
   return 0;
}

int RefreshRemoteVariable_VariableDatabase(struct VariableShare * vsh,char * variable_name)
{
   debug_say(" RefreshRemoteVariable_VariableDatabase , not implemented ");
      int var_id = FindVariable_Database(vsh,variable_name);
   if ( var_id == 0 )
    {
      fprintf(stderr,"Variable %s not found , cannot be refreshed to local\n",variable_name);
      return 0;
    } else
    {
      --var_id; // FindVariableDatabase returns +1 results ( to signal failure with 0 )
      //TODO
      //vsh->share.variables[var_id].
      //UpdateRemoteVariable(vsh,  our_varid, peer_varid);
    }
   return 0;
}
*/
int VariableIdExists(struct VariableShare * vsh,unsigned int var_id)
{
  if (vsh->share.total_variables_shared<=var_id) { return 0; }
  return 1;
}

int MarkVariableAsNeedsRefresh_VariableDatabase(struct VariableShare * vsh,char * variable_name,int clientsock)
{
   int var_id = FindVariable_Database(vsh,variable_name);
   if ( var_id == 0 )
    {
      fprintf(stderr,"Variable %s not found , cannot be refreshed to local\n",variable_name);
      return 0;
    } else
    {
      --var_id; // FindVariableDatabase returns +1 results ( to signal failure with 0 )
      fprintf(stderr,"Variable %s has been marked , as \"needs refresh\" \n",variable_name);
      vsh->share.variables[var_id].flag_needs_refresh_from_sock = clientsock;
    }
   return 0;
}

int IfLocalVariableChanged_SignalUpdateToJoblist(struct VariableShare * vsh,unsigned int var_id)
{
  if (!VariableIdExists(vsh,var_id)) { fprintf(stderr,"Variable addressed ( %u ) by IfLocalVariableChanged_SignalUpdateToJoblist does not exist \n",var_id); return 0; }
  unsigned long newhash=GetVariableHash(vsh,var_id);
  if (newhash!=vsh->share.variables[var_id].hash )
    {
      fprintf(stderr,"Variable Changed Hash for variable %u , values %u to %u !\n",var_id,(unsigned int) newhash,(unsigned int) vsh->share.variables[var_id].hash );
      vsh->share.variables[var_id].hash=newhash; /*We keep the new hash as the current hash :)*/
      Job_SingalLocalVariableChanged(vsh,var_id); // <- This call creates a job to signal the change..!

      return 1;
    }
  return 0;
}

int SignalUpdatesForAllLocalVariablesThatNeedIt(struct VariableShare * vsh)
{
 if ( vsh->share.total_variables_shared == 0 ) { return 0; /* NO VARIABLES TO SHARE OR UPDATE!*/}
 int retres=0;
 unsigned int i=0;
 //fprintf(stderr,"Refreshing %u variables!\n",vsh->share.total_variables_shared);
 for ( i=0; i<vsh->share.total_variables_shared; i++)
  {
     if ( IfLocalVariableChanged_SignalUpdateToJoblist(vsh,i) ) { ++retres; }
  }

  return retres;
}



int RefreshAllVariablesThatNeedIt(struct VariableShare *vsh)
{
   unsigned int added_jobs=0;
   unsigned int i=0;

    for ( i=0; i<vsh->share.total_variables_shared; i++)
   {
      if ( vsh->share.variables[i].flag_needs_refresh_from_sock > 0 )
        {
           fprintf(stderr,"Detected that a variable (%u) needs refresh , and automatically adding a job to receive it\n",i);
           if ( AddJob(vsh,i,vsh->share.variables[i].flag_needs_refresh_from_sock ,READFROM) )
             {
               ++added_jobs;
               fprintf(stderr,"Carrying on after job adding\n");
               vsh->share.variables[i].flag_needs_refresh_from_sock=0; // We trust that the "add job" will do its job :P
             } else
             {
               fprintf(stderr,"Could not add job\n");
             }
        }
   }


  return added_jobs;
}

int MakeSureVarReachedPeers(struct VariableShare *vsh,char * varname)
{
  fprintf(stderr,"MakeSureVarReachedPeers waiting for var %s \n",varname);

  unsigned int var_id = FindVariable_Database(vsh,varname);
  if (!var_id) { fprintf(stderr,"MakeSureVarReachedPeers called for non existing variable ( %s ) \n",varname); return 0; }
  --var_id;

  while ( vsh->share.auto_refresh_every_msec==0)
   {
     usleep(100);
   }

  if ( SignalUpdatesForAllLocalVariablesThatNeedIt(vsh) ) // <- this will make any variables that have just changed be set as flag_needs_refres_from_sock
   {
       fprintf(stderr,"Found changed variables..!\n");
   }

  unsigned int wait_time=10000;
  while (wait_time>0)
  {
      if ( (vsh->share.variables[var_id].flag_needs_refresh_from_sock==0) && ( vsh->jobs_loaded ==0 )) { return 1; }

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
   unsigned int variables_changed=0,variables_refreshed=0;
   while ( (vsh->global_policy==VSP_AUTOMATIC) && (vsh->stop_refresh_thread==0) )
   {
       usleep(vsh->share.auto_refresh_every_msec);

       if (vsh->pause_refresh_thread)
          {
            // This means that auto refresh is disabled so we dont do anything until is re-enabled
          } else
          {
             variables_changed=SignalUpdatesForAllLocalVariablesThatNeedIt(vsh);
             total_variables_changed+=variables_changed;

             variables_refreshed=RefreshAllVariablesThatNeedIt(vsh);

             if ( (variables_changed==0 ) && ( variables_refreshed == 0) ) { fprintf(stderr,"."); } else
                                                                           { fprintf(stderr,"AutoRefresh Thread: %u variables changed and %u variables refreshed\n",variables_changed,variables_refreshed); }
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
