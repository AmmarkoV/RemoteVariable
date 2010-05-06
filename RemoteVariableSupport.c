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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "helper.h"
#include "RemoteVariableSupport.h"
#include "NetworkFramework.h"
#include "VariableDatabase.h"



extern char byte_order=0; // 0 = intel ,  network
extern unsigned int central_timer=0;


/* #Start_VariableSharing#
   This function is supposed to allocate a VariableShare structure ( vsh )
   and prepare the Variable Share Server in our machine as a master share
*/
struct VariableShare * Start_VariableSharing(char * sharename,char * password)
{
    fprintf(stderr,"Starting Variable Sharing!\n");
    struct VariableShare *  vsh = Create_VariableDatabase(sharename,"127.0.0.1",12345,password,128);
    if ( vsh == 0 ) return 0;

    if ( vsh->global_state == 0 )  vsh->global_state=VSS_NORMAL; else
                            {
                              error("Variable Shared already initialized , stop it before starting it again as server!");
                            }
    StartRemoteVariableServer(vsh);
    return vsh;
}

/* #ConnectToRemote_VariableSharing#
   This function is supposed to allocate a VariableShare structure ( vsh )
   and connect to a remote Share and replicate its data in our structures
*/
struct VariableShare * ConnectToRemote_VariableSharing(char * IP,unsigned int port,char * password)
{
    struct VariableShare *  vsh = Create_VariableDatabase("RemoteShare",IP,port,password,128);
    if ( vsh == 0 ) return 0;
    if ( vsh->global_state == 0 )  vsh->global_state=VSS_NORMAL; else
                            {
                              error("Variable Shared already initialized , stop it before starting it again as connect to remote!");
                            }

    StartRemoteVariableConnection(vsh);
    return vsh;
}


/* #Stop_VariableSharing#
   Stops all threads used for variable sharing and after they are safely destroyed it frees all the memory
   of the structure vsh
*/
int Stop_VariableSharing(struct VariableShare * vsh)
{
    vsh->global_state=VSS_UNITIALIZED;
    vsh->stop_server_thread=1;
    vsh->stop_client_thread=1;


    Destroy_VariableDatabase(vsh);
    return 0;
}

/* #Add_VariableToSharingList#
   Adds a new variable to the Variable Share with chosen permissions
   If the Variable Share policy is set to automatically synchronize this variable it will start doing it after it is added with this command
*/
int Add_VariableToSharingList(struct VariableShare * vsh,char * variable_name,unsigned int permissions,void * ptr,unsigned int ptr_size)
{
    return AddVariable_Database(vsh,variable_name,permissions,ptr,ptr_size);
}

/* #Add_VariableToSharingList#
   Removes an existing variable from the Variable Share
*/
int Delete_VariableFromSharingList(struct VariableShare * vsh,char * variable_name)
{
    fprintf(stderr,"Delete_VariableFromSharingList not implemented yet!");
    return 0;
}

/* #Refresh_LocalVariable#
   If the share policy is automatic updates , it forces an update
   If the share policy is manual updates , it forces an update
   This function BLOCKS until the variable is refreshed or the connection dropped
*/
int Refresh_LocalVariable(struct VariableShare * vsh,char * variable_name)
{
    return RefreshLocalVariable_VariableDatabase(vsh,variable_name);
}

/* #Refresh_RemoteVariable#
   If the share policy is automatic updates , it forces an update
   If the share policy is manual updates , it forces an update
   This function BLOCKS until the variable is refreshed or the connection dropped
*/
int Refresh_RemoteVariable(struct VariableShare * vsh,char * variable_name)
{
    return RefreshRemoteVariable_VariableDatabase(vsh,variable_name);
}


int IsUptodate_RemoteVariable(struct VariableShare * vsh,char * variable_name)
{
  fprintf(stderr,"IsUptodate_RemoteVariable not implemented yet ,returning false!");
  return 0;
}

/*
   TODO ADD POLICY SWITCHES
*/
