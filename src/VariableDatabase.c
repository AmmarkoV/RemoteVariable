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

  vsh->share.auto_refresh_every_msec=RVS_DEFAULT_AUTO_REFRESH_OF_SHARE;
  vsh->share.total_variables_memory=newsize;
  vsh->share.total_variables_shared=0;
  vsh->peers_active=0;
  return vsh;
}


int Destroy_VariableDatabase(struct VariableShare * vsh)
{
  if (vsh==0) { debug_say("Memory for share , already deallocated!"); return 1; }
  if ( vsh->share.variables == 0 ) { debug_say("Memory for shared variables , already deallocated!"); return 1; }

  free(vsh->share.variables);
  free(vsh);
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
  if (vsh->share.variables[var_id].size_of_ptr<sizeof(unsigned long)) { /*The whole variable fits inside the unsigned long so no hash is required*/
                                                                        unsigned long * stacklongptr = vsh->share.variables[var_id].ptr;
                                                                        unsigned long stacklong = *stacklongptr;
                                                                        /*fprintf(stderr,"GetVariableHash for var %u returning %u\n",var_id,stacklong);*/
                                                                        return stacklong; }
  debug_say("Todo ADD code that produces hash on variables that do not fit unsigned long!\n");
  /*hash(unsigned char *str);*/
  return 0;
}

int AddVariable_Database(struct VariableShare * vsh,char * var_name,unsigned int permissions,void * ptr,unsigned int ptr_size)
{
 if ( VariableShareOk(vsh) == 0 ) return 0;

 unsigned int spot_to_take=vsh->share.total_variables_memory+1;
 if (vsh->share.total_variables_memory > vsh-> share.total_variables_shared )
  { // WE HAVE SPACE TO ADD A NEW SHARE DATA!
     spot_to_take=vsh->share.total_variables_shared;
  } else
  {
    // NO FREE SPACE RESIZE CODE HERE !
    if ( Resize_VariableDatabase(vsh,vsh->share.total_variables_memory+128) == 0 ) return 0;
  }

  if (spot_to_take < vsh->share.total_variables_memory)
  {
    if ( strlen(var_name)>=128 ) { error("Buffer Overflow attempt , we are safe"); return 0; }
    strcpy(vsh->share.variables[spot_to_take].ptr_name,var_name);
    if ( strlen(var_name) > 31 ) { error("Buffer Overflow attempt originating from us ? "); return 0; }

    vsh->share.variables[spot_to_take].last_write_inc=0;
    vsh->share.variables[spot_to_take].permissions=permissions;

    vsh->share.variables[spot_to_take].ptr=ptr;
    vsh->share.variables[spot_to_take].size_of_ptr=ptr_size;

    ++vsh->share.total_variables_shared;
  }

 return 1;
}

int DeleteVariable_Database(struct VariableShare * vsh,char * var_name)
{
  // TODO TODO TODO
  error("The Variable Share Object wants to delete an item and there is no code implemented for deleting it (yet) !");
  return 0;
}

signed int FindVariable_Database(struct VariableShare * vsh,char * var_name)
{
 if ( VariableShareOk(vsh) == 0 ) return 0;

 int i;
 for ( i=0; i<vsh->share.total_variables_shared; i++)
  {
     if ( strcmp(vsh->share.variables[i].ptr_name,var_name)==0 )
      {
        return (signed int ) i;
      }
  }
 return 0;
}

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
      /*TODO*/
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
      /*TODO*/
      //vsh->share.variables[var_id].
      //UpdateRemoteVariable(vsh,  our_varid, peer_varid);
    }
   return 0;
}


int IfLocalVariableChanged_SignalUpdateToJoblist(struct VariableShare * vsh,unsigned int var_id)
{
  unsigned long newhash=GetVariableHash(vsh,var_id);
  if (newhash!=vsh->share.variables[var_id].hash )
    {
      debug_say("Variable Changed !");
      Job_UpdateLocalVariableToAllPeers(vsh,var_id);
      /*We keep the new hash as the current hash :)*/
      vsh->share.variables[var_id].hash=newhash;

      return 1;
    }
  return 0;
}

int RefreshAllLocalVariables(struct VariableShare * vsh)
{
 if ( vsh->share.total_variables_shared == 0 ) { return -1; /* NO VARIABLES TO SHARE OR UPDATE!*/}
 int retres=0;
 int i=0;
 //fprintf(stderr,"Refreshing %u variables!\n",vsh->share.total_variables_shared);
 for ( i=0; i<vsh->share.total_variables_shared; i++)
  {
     if ( IfLocalVariableChanged_SignalUpdateToJoblist(vsh,i) ) ++retres;
  }

 if ( retres>0 ) return retres;
 return 0;
}


void *
AutoRefreshVariable_Thread(void * ptr)
{
  debug_say("AutoRefresh Thread started..\n");
  struct VariableShare *vsh;
  vsh = (struct VariableShare *) ptr;

   unsigned int total_variables_changed=0;
   unsigned int variables_changed=0;
   while ( (vsh->global_policy==VSP_AUTOMATIC) && (vsh->stop_refresh_thread==0) )
   {
       usleep(vsh->share.auto_refresh_every_msec);
       if (vsh->share.auto_refresh_every_msec==0)
          {  // This means that auto refresh is disabled so we sleep things out till it is re-enabled
             usleep(10000);
          } else
          {
             variables_changed=RefreshAllLocalVariables(vsh);
             total_variables_changed+=variables_changed;
             fprintf(stderr,"%u variables changed in this loop\n",variables_changed);
          }
   }
   fprintf(stderr,"%u total variables changed detected by autorefresh thread\n",total_variables_changed);

   return 0;
}

/* THREAD STARTER */
int StartAutoRefreshVariable(struct VariableShare * vsh)
{
  vsh->stop_refresh_thread=0;
  int retres = pthread_create( &vsh->refresh_thread, NULL,  AutoRefreshVariable_Thread ,(void*) vsh);
  if (retres!=0) retres = 0; else
                 retres = 1;
  return retres;
}
