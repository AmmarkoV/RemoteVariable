/***************************************************************************
* Copyright (C) 2010-2012 by Ammar Qammaz *
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
#include "Peers.h"
#include "InternalTester.h"
#include "NetworkFramework.h"
#include "VariableDatabase.h"


/* #Start_VariableSharing#
   This function is supposed to allocate a VariableShare structure ( vsh )
   and prepare the Variable Share Server in our machine as a master share
*/
struct VariableShare * RVS_HostVariableShare(char * sharename,char * bindaddress,unsigned int port,char * password)
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
    return vsh;
}

/* #ConnectToRemote_VariableSharing#
   This function is supposed to allocate a VariableShare structure ( vsh )
   and connect to a remote Share and replicate its data in our structures
*/
struct VariableShare * RVS_ConnectToVariableShare(char * sharename,char * IP,unsigned int port,char * password)
{
    under_construction_msg();

    struct VariableShare *  vsh = Create_VariableDatabase(sharename,IP,port,password,128);
    if ( vsh == 0 ) { fprintf(stderr,"Could not create Remote variable database ( Name : %s , IP %s:%u )",sharename,IP,port); return 0; }


    vsh->global_state=VSS_WAITING_FOR_HANDSHAKE;
    vsh->global_policy=VSP_AUTOMATIC;
    vsh->this_address_space_is_master=0;


    StartRemoteVariableConnection(vsh);
    StartAutoRefreshVariable(vsh);
    return vsh;
}


/* #Stop_VariableSharing#
   Stops all threads used for variable sharing and after they are safely destroyed it frees all the memory
   of the structure vsh
*/
int RVS_StopVariableShare(struct VariableShare * vsh)
{

    unsigned int loaded_peers=vsh->total_peers,i=0;
    for (i=0; i<loaded_peers; i++)
     {
       RemPeer(vsh,i);
     }

    vsh->global_state=VSS_UNITIALIZED;
    vsh->stop_server_thread=1;
    vsh->stop_client_thread=1;
    vsh->stop_refresh_thread=1;

    usleep(1000);


    return Destroy_VariableDatabase(vsh);
}



int RVS_InternalTest()
{
  return PerformInternalTest();
}

int RVS_PeersActive(struct VariableShare * vsh)
{
  if (vsh==0) { return 0; }

  unsigned int count_peers_active = 0;
  if (vsh->total_peers > 0)
    {
        unsigned int i=0;
        for (i=0; i<vsh->total_peers; i++)
         {
           if ( vsh->peer_list[i].peer_state==VSS_NORMAL ) { ++count_peers_active; }
         }
    }

  return count_peers_active;
}


void RVS_SetPolicy(struct VariableShare * vsh,unsigned int new_policy)
{
    return ;// disabled for now
    vsh->global_policy = new_policy;

}


/* #Add_VariableToSharingList#
   Adds a new variable to the Variable Share with chosen permissions
   If the Variable Share policy is set to automatically synchronize this variable it will start doing it after it is added with this command
*/
int RVS_AddVariable(struct VariableShare * vsh,char * variable_name,unsigned int permissions,volatile void * ptr,unsigned int ptr_size)
{
    return AddVariable_Database(vsh,variable_name,permissions,ptr,ptr_size);
}

/* #Add_VariableToSharingList#
   Removes an existing variable from the Variable Share
*/
int RVS_RemoveVariable(struct VariableShare * vsh,char * variable_name)
{
    fprintf(stderr,"Delete_VariableFromSharingList not implemented yet!");
    return 0;
}


int RVS_GetVarId(struct VariableShare * vsh , char * var_name , char * exists)
{
   struct failint retres = FindVariable_Database(vsh,var_name);
   if (retres.failed==0) { *exists=1; } else
                         { *exists=0; }
   return retres.value;
}


int RVS_GetVarLastUpdateTimestamp(struct VariableShare * vsh , unsigned int var_id)
{
   return 0;
}

int RVS_WaitForTimestamp(struct VariableShare * vsh , unsigned int var_id,unsigned int timestamp_to_wait_for)
{
   return 0;
}


/* #LockForLocalUse_LocalVariable#
   The variable_name is locked and will not be writeable , or readable from a network peer
*/
int RVS_LockVariableLocalOnly(struct VariableShare * vsh,unsigned int var_id)
{
    /*Todo Implement*/
    return 0;
}


/* #Unlock_LocalVariable#
   The variable_name is unlocked and will be writeable , or readable from a network peer according to the permissions
   specified when it was added to the share
*/
int RVS_UnlockVariable(struct VariableShare * vsh,unsigned int var_id)
{
    /*Todo Implement*/
    return 0;
}


/* #Refresh_LocalVariable#
   If the share policy is automatic updates , it forces an update
   If the share policy is manual updates , it forces an update
   This function BLOCKS until the variable is refreshed or the connection dropped
*/
int RVS_Refresh_AllVariables(struct VariableShare * vsh)
{
  return 1;// disabled for now

  pthread_mutex_lock (&vsh->refresh_lock); // LOCK PROTECTED OPERATION -------------------------------------------

    unsigned int variables_changed=1 , variables_signaled=1 , variables_refreshed=1;

  while ((variables_changed+variables_signaled+variables_refreshed)!=0)
  {
   variables_changed=CheckForChangedVariables(vsh);

   variables_signaled=SignalUpdatesForAllLocalVariablesThatNeedIt(vsh);

   variables_refreshed=RefreshAllVariablesThatNeedIt(vsh);
   printf("AutoRefresh Thread: %u vars changed | %u vars signaled | %u vars refreshed\n",variables_changed,variables_signaled,variables_refreshed);
  }
  pthread_mutex_unlock (&vsh->refresh_lock); // LOCK PROTECTED OPERATION -------------------------------------------

  return 1;
 //    return RefreshLocalVariable_VariableDatabase(vsh,variable_name);
}

/* #RVS_Refresh_Variable#
   If the share policy is automatic updates , it forces an update
   If the share policy is manual updates , it forces an update
   This function BLOCKS until the variable is refreshed or the connection dropped
*/
int RVS_Refresh_Variable(struct VariableShare * vsh,unsigned int var_id)
{
  return 0;
  //  return RefreshRemoteVariable_VariableDatabase(vsh,variable_name);
}


int RVS_LocalVariableChanged(struct VariableShare * vsh,unsigned int var_id)
{
  return 1;
}

int RVS_LocalVariableIsUptodate(struct VariableShare * vsh,unsigned int var_id)
{
  return (!RVS_LocalVariableChanged(vsh,var_id));
}






int RVS_MakeSureVarReachedPeers(struct VariableShare * vsh,char * variable_name,unsigned int wait_time_ms)
{
  return MakeSureVarReachedPeers(vsh,variable_name,wait_time_ms);
}



/*
   TODO ADD POLICY SWITCHES
*/
