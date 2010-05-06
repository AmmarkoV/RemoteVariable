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

  vsh->share.total_variables_memory=newsize;
  vsh->share.total_variables_shared=0;
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
 if ( VariableShareOk(vsh) == 0 ) return -1;

 int i;
 for ( i=0; i<vsh->share.total_variables_shared; i++)
  {
     if ( strcmp(vsh->share.variables[i].ptr_name,var_name)==0 )
      {
        return (signed int ) i;
      }
  }
 return -1;
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

int FindJobsFrom_VariableDatabase(struct VariableShare * vsh)
{
 return -1;
}

int RefreshLocalVariable_VariableDatabase(struct VariableShare * vsh,char * variable_name)
{
   debug_say(" RefreshLocalVariable_VariableDatabase , not implemented ");
   int var_id = FindVariable_Database(vsh,variable_name);
   if ( var_id == -1 )
    {
      fprintf(stderr,"Variable %s not found , cannot be refreshed to local\n",variable_name);
      return -1;
    } else
    {
      //vsh->share.variables[var_id].
      //UpdateLocalVariable(vsh,  our_varid, peer_varid);
    }
   return -1;
}

int RefreshRemoteVariable_VariableDatabase(struct VariableShare * vsh,char * variable_name)
{
   debug_say(" RefreshRemoteVariable_VariableDatabase , not implemented ");
      int var_id = FindVariable_Database(vsh,variable_name);
   if ( var_id == -1 )
    {
      fprintf(stderr,"Variable %s not found , cannot be refreshed to local\n",variable_name);
      return -1;
    } else
    {
      //vsh->share.variables[var_id].
      //UpdateRemoteVariable(vsh,  our_varid, peer_varid);
    }
   return -1;
}


