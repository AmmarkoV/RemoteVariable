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
#ifndef REMOTEVARIABLESUPPORT_H_INCLUDED
#define REMOTEVARIABLESUPPORT_H_INCLUDED


#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include <unistd.h>

#define MAX_JOBS_PENDING 100
#define DEFAULT_AUTO_REFRESH_OF_SHARE 1000

extern char byte_order; // 0 = intel ,  network
extern unsigned int central_timer;

enum jobactions
{
    NOACTION = 0 ,
    WRITETO,
    READFROM,
    SYNC
};


struct ShareListItem
{

    char ptr_name[32];

    unsigned int last_write_inc;
    unsigned int permissions;

    unsigned char lock_refresh;
    unsigned char flag_needs_refresh;

    unsigned long hash;

    unsigned int size_of_ptr;
    void * ptr;
};



struct ShareList
{
    unsigned int auto_refresh_every_msec; // 0 = disable auto refresh

    unsigned int total_variables_memory;
    unsigned int total_variables_shared;
    struct ShareListItem * variables;
};


struct SharePeer
{
    /*TODO ADD MORE DATA*/
    char IP[10];
    unsigned int port;

    int socket_to_client;

    unsigned int ping;
    unsigned int last_transaction;


    struct ShareList share;
};

struct ShareJob
{
     unsigned char status;

     unsigned int our_sharelist_id;
     unsigned int remote_peer_id;

     char action; // 0 no action , 1 = write to , 2 = read from , 3 = check if it is the same on the other end

     unsigned int time;
};


enum VariableShareStates
{
    VSS_UNITIALIZED=0,
    VSS_NORMAL,
    VSS_INCOMING_TRANSACTIONS_DISABLED,
    VSS_OUTGOING_TRANSACTIONS_DISABLED,
    VSS_ALL_TRANSACTIONS_DISABLED,
    VSS_CLOSING
};

enum VariableSharePolicies
{
    VSP_AUTOMATIC=0,
    VSP_LOWFOOTPRINT,
    VSP_MANUAL
};





struct VariableShare
{
    char sharename[32];
    char this_address_space_is_master;

    unsigned int global_state; /* enum VariableShareStates */
    unsigned int global_policy; /* enum VariableSharePolicies */

    char ip[32];
    unsigned int port;

    struct ShareJob job_list[MAX_JOBS_PENDING];
    unsigned int jobs_loaded;

    struct SharePeer master;
    struct SharePeer client_list[100];
    unsigned int clients_loaded;



    pthread_t refresh_thread;
    unsigned int stop_refresh_thread;

    pthread_t client_thread;
    unsigned int stop_client_thread;

    pthread_t server_thread;
    unsigned int stop_server_thread;



    struct ShareList share;

};



struct VariableShare * Start_VariableSharing(char * sharename,char * bindaddress,unsigned int port,char * password);
struct VariableShare * ConnectToRemote_VariableSharing(char * IP,unsigned int port,char * password);
int Stop_VariableSharing(struct VariableShare * vsh);
int Add_VariableToSharingList(struct VariableShare * vsh,char * variable_name,unsigned int permissions,void * ptr,unsigned int ptr_size);
int Delete_VariableFromSharingList(struct VariableShare * vsh,char * variable_name);
int Refresh_LocalVariable(struct VariableShare * vsh,char * variable_name);
int Refresh_RemoteVariable(struct VariableShare * vsh,char * variable_name);
int IsUptodate_RemoteVariable(struct VariableShare * vsh,char * variable_name);
#ifdef __cplusplus
}
#endif


#endif // REMOTEVARIABLESUPPORT_H_INCLUDED
