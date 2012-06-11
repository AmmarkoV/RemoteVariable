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

#define RVS_GUARD_VALUE 123
#define RVS_VARIABLEDATABASE_THREAD_TIME 500

#define RVS_MAX_PEERS 100
#define RVS_MAX_JOBS_PENDING 100

#define RVS_MAX_RAW_HANDSHAKE_MESSAGE 128
#define RVS_MAX_SHARE_NAME_CHARS 64
#define RVS_MAX_SHARE_IP_CHARS 64


enum jobactions
{
    NOACTION = 0 ,

    INTERNAL_START_SIGNALCHANGED,
    INTERNAL_START_READFROM,
    INTERNAL_START_WRITETO,

    RAW_MESSAGE,

    RESP_WRITETO,
    WRITETO,
    READFROM,
    SIGNALCHANGED,
    SIGNALMSGSUCCESS,
    SIGNALMSGFAILURE,
    SYNC
};

enum msgdir
{
   NO_DIRECTION = 0 ,
   INCOMING_MSG ,
   OUTGOING_MSG
};

struct failint
{
  unsigned char failed;
  int value;
};

struct PacketHeader
{
   unsigned char incremental_value;
   unsigned char operation_type;
   unsigned int  var_id;
   unsigned int  payload_size;
};



struct MessageTableItem
{
   unsigned int time;
   unsigned char protocol_progress;
   unsigned int last_protocol_id;
   unsigned char executed;
   unsigned char remove;
   unsigned char direction; // see enum msgdir
   unsigned char sent;
   struct PacketHeader header;
   unsigned char payload_local_malloc;
   void * payload;
};


struct MessageTable
{
    pthread_mutex_t lock;
    pthread_mutex_t remlock;

    unsigned char time;
    unsigned char groupid;

    unsigned int GUARD_BYTE1;

    pthread_t sendrecv_thread;
    unsigned int pause_sendrecv_thread;
    unsigned int stop_sendrecv_thread;

    pthread_t messageproc_thread;
    unsigned int pause_messageproc_thread;
    unsigned int stop_messageproc_thread;

    unsigned int GUARD_BYTE2;

    unsigned int message_queue_total_length;

    unsigned int message_queue_current_length;

    struct MessageTableItem * table;

};



struct ShareListItem
{

    char ptr_name[32];
    unsigned int ptr_name_length;

    unsigned char GUARD_BYTE1;

    unsigned int last_write_time;
    unsigned int permissions;

    unsigned long last_signaled_hash[RVS_MAX_PEERS];
    unsigned long hash;
    unsigned int this_hash_transmission_count;


    int flag_needs_refresh_from_sock;
    // TODO : NOTE THAT THIS SHOULD ACTUALLY BE AN ARRAY AS LONG AS THE POSSIBLE PEERS


    unsigned int size_of_ptr;
    void * ptr;

    unsigned char GUARD_BYTE2;
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
    char IP[RVS_MAX_SHARE_IP_CHARS];
    unsigned int port;

    int socket_to_client;

    unsigned int ping_in_microseconds;
    unsigned int last_transaction;

    unsigned int peer_state; /* enum VariableShareStates */

    pthread_t peer_thread;
    unsigned int pause_peer_thread;
    unsigned int stop_peer_thread;

    struct MessageTable messages;

    struct ShareList share;
};


enum VariableShareStates
{
    VSS_UNITIALIZED=0,
    VSS_WAITING_FOR_HANDSHAKE=0,
    VSS_CONNECTION_FAILED,
    VSS_HANDSHAKE_FAILED,
    VSS_ALL_TRANSACTIONS_DISABLED,
    VSS_INCOMING_TRANSACTIONS_DISABLED,
    VSS_OUTGOING_TRANSACTIONS_DISABLED,
    VSS_CLOSING,
    VSS_SECURITY_ALERT,
    VSS_NORMAL
};

enum VariableSharePolicies
{
    VSP_AUTOMATIC=0,
    VSP_LOWFOOTPRINT,
    VSP_MANUAL
};





struct VariableShare
{
    char sharename[RVS_MAX_SHARE_NAME_CHARS];
    char this_address_space_is_master;

    unsigned int global_state; /* enum VariableShareStates */
    unsigned int global_policy; /* enum VariableSharePolicies */

    char ip[RVS_MAX_SHARE_IP_CHARS];
    unsigned int port;

    struct SharePeer master;
    struct SharePeer peer_list[RVS_MAX_PEERS];
    unsigned int total_peers;


    pthread_mutex_t refresh_lock;
    pthread_t refresh_thread;
    unsigned int pause_refresh_thread;
    unsigned int stop_refresh_thread;

    pthread_t client_thread;
    unsigned int pause_client_thread;
    unsigned int stop_client_thread;

    pthread_t server_thread;
    unsigned int pause_server_thread;
    unsigned int stop_server_thread;


    char byte_order; // 0 = intel ,  network
    unsigned int central_timer;

    struct ShareList share;

};


struct PeerServerContext
{
   struct VariableShare * vsh;
   unsigned int peer_id;
   int peersock;
   unsigned int keep_var_on_stack;
};

struct SocketAdapterToMessageTablesContext
{
   struct VariableShare * vsh;
   unsigned int peer_id;
   int peersock;
   unsigned int type_of_thread;
   unsigned int keep_var_on_stack;
   unsigned int * pause_switch;
   unsigned int * stop_switch;
};


struct VariableShare * Start_VariableSharing(char * sharename,char * bindaddress,unsigned int port,char * password);
struct VariableShare * ConnectToRemote_VariableSharing(char * sharename,char * IP,unsigned int port,char * password);
int Stop_VariableSharing(struct VariableShare * vsh);
int PeersActive_VariableShare(struct VariableShare * vsh);
int RemoteVariableSupport_InternalTest();
int Add_VariableToSharingList(struct VariableShare * vsh,char * variable_name,unsigned int permissions,volatile void * ptr,unsigned int ptr_size);
int Delete_VariableFromSharingList(struct VariableShare * vsh,char * variable_name);
int Refresh_LocalVariable(struct VariableShare * vsh,char * variable_name);
int Refresh_RemoteVariable(struct VariableShare * vsh,char * variable_name);
int IsUptodate_RemoteVariable(struct VariableShare * vsh,char * variable_name);
int MakeSureVarReachedPeers_RemoteVariable(struct VariableShare * vsh,char * variable_name,unsigned int wait_time_ms);


#ifdef __cplusplus
}
#endif


#endif // REMOTEVARIABLESUPPORT_H_INCLUDED
