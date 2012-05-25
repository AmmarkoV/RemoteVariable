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
#include "JobTables.h"


/* #Start_VariableSharing#
   This function is supposed to allocate a VariableShare structure ( vsh )
   and prepare the Variable Share Server in our machine as a master share
*/
struct VariableShare * Start_VariableSharing(char * sharename,char * bindaddress,unsigned int port,char * password)
{
    under_construction_msg();

    fprintf(stderr,"Starting Variable Sharing!\n");
    struct VariableShare *  vsh = Create_VariableDatabase(sharename,bindaddress,port,password,128);
    if ( vsh == 0 ) return 0;

    if ( vsh->global_state == 0 )
                            {
                              vsh->global_state=VSS_WAITING_FOR_HANDSHAKE;
                              vsh->global_policy=VSP_AUTOMATIC;
                              vsh->this_address_space_is_master=1;
                            } else
                            {
                              error("Variable Shared already initialized , stop it before starting it again as server!");
                            }
    StartRemoteVariableServer(vsh);
    StartAutoRefreshVariable(vsh);
    StartJobExecutioner(vsh);
    return vsh;
}

/* #ConnectToRemote_VariableSharing#
   This function is supposed to allocate a VariableShare structure ( vsh )
   and connect to a remote Share and replicate its data in our structures
*/
struct VariableShare * ConnectToRemote_VariableSharing(char * sharename,char * IP,unsigned int port,char * password)
{
    under_construction_msg();

    struct VariableShare *  vsh = Create_VariableDatabase(sharename,IP,port,password,128);
    if ( vsh == 0 ) { fprintf(stderr,"Could not create Remote variable database ( Name : %s , IP %s:%u )",sharename,IP,port); return 0; }


    vsh->global_state=VSS_WAITING_FOR_HANDSHAKE;
    vsh->global_policy=VSP_AUTOMATIC;
    vsh->this_address_space_is_master=0;


    StartRemoteVariableConnection(vsh);
    StartAutoRefreshVariable(vsh);
    StartJobExecutioner(vsh);
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
    vsh->stop_refresh_thread=1;

    return Destroy_VariableDatabase(vsh);
}




int PeersActive_VariableShare(struct VariableShare * vsh)
{
  if (vsh==0) { return 0; }
  return vsh->peers_active;
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


/* #LockForLocalUse_LocalVariable#
   The variable_name is locked and will not be writeable , or readable from a network peer
*/
int LockForLocalUse_LocalVariable(struct VariableShare * vsh,char * variable_name)
{
    /*Todo Implement*/
    return 0;
}


/* #Unlock_LocalVariable#
   The variable_name is unlocked and will be writeable , or readable from a network peer according to the permissions
   specified when it was added to the share
*/
int Unlock_LocalVariable(struct VariableShare * vsh,char * variable_name)
{
    /*Todo Implement*/
    return 0;
}


/* #Refresh_LocalVariable#
   If the share policy is automatic updates , it forces an update
   If the share policy is manual updates , it forces an update
   This function BLOCKS until the variable is refreshed or the connection dropped
*/
int Refresh_LocalVariable(struct VariableShare * vsh,char * variable_name)
{
  return 0;
 //    return RefreshLocalVariable_VariableDatabase(vsh,variable_name);
}

/* #Refresh_RemoteVariable#
   If the share policy is automatic updates , it forces an update
   If the share policy is manual updates , it forces an update
   This function BLOCKS until the variable is refreshed or the connection dropped
*/
int Refresh_RemoteVariable(struct VariableShare * vsh,char * variable_name)
{
  return 0;
  //  return RefreshRemoteVariable_VariableDatabase(vsh,variable_name);
}


int IsUptodate_RemoteVariable(struct VariableShare * vsh,char * variable_name)
{
  fprintf(stderr,"IsUptodate_RemoteVariable not implemented yet ,returning false!");
  return 0;
}


int MakeSureVarReachedPeers_RemoteVariable(struct VariableShare * vsh,char * variable_name)
{
  return MakeSureVarReachedPeers(vsh,variable_name);
}



/*
   TODO ADD POLICY SWITCHES
*/
