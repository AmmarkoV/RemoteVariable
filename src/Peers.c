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

#include "Peers.h"
#include "MessageTables.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "helper.h"

struct failint AddPeer(struct VariableShare * vsh,char * name,unsigned int port , int clientsock)
{
  struct failint retres;
  retres.failed=0;
  retres.value=0;


  if (vsh->total_peers < RVS_MAX_PEERS)
   {
       unsigned int pos = vsh->total_peers;

       memset(vsh->peer_list[pos].IP,0,RVS_MAX_SHARE_IP_CHARS);
       strncpy(vsh->peer_list[pos].IP,name,RVS_MAX_SHARE_IP_CHARS);
       vsh->peer_list[pos].port=port;
       vsh->peer_list[pos].socket_to_client = clientsock;
       vsh->peer_list[pos].peer_state=VSS_WAITING_FOR_HANDSHAKE;

       pthread_mutex_init(&vsh->peer_list[pos].messages.lock, 0); //New allocation so cleaning up mutex
       pthread_mutex_init(&vsh->peer_list[pos].messages.remlock, 0); //New allocation so cleaning up mutex

       AllocateMessageQueue(&vsh->peer_list[pos].messages,100);
       vsh->peer_list[pos].messages.is_master = vsh->this_address_space_is_master;

       ++vsh->total_peers;
       retres.value=pos;
       return retres;
   }

  retres.failed=1;
  return retres;
}

int SwapPeers(struct VariableShare * vsh,int peer_id1,int peer_id2)
{
 if ( (peer_id1<vsh->total_peers)&&(peer_id2<vsh->total_peers) )
  {
    struct SharePeer temp=vsh->peer_list[peer_id1];
    vsh->peer_list[peer_id1]=vsh->peer_list[peer_id2];
    vsh->peer_list[peer_id2]=temp;
  } else
  {
    fprintf(stderr,"SwapPeers called for peers %u and %u which are out of bounds ( %u ) \n",peer_id1,peer_id2,vsh->total_peers);
    return 0;
  }
 return 1;
}


int RemPeer(struct VariableShare * vsh,int peer_id)
{
  FreeMessageQueue(&vsh->peer_list[peer_id].messages);

  pthread_mutex_destroy(&vsh->peer_list[peer_id].messages.lock);
  pthread_mutex_destroy(&vsh->peer_list[peer_id].messages.remlock);

 if ( (vsh->total_peers==1)&&(peer_id==0)) { vsh->total_peers=0; return 1; }
 if ( (vsh->total_peers>1) )
  {
    unsigned int last_peer = vsh->total_peers-1;
    SwapPeers(vsh,peer_id,last_peer);
    --vsh->total_peers;
    return 1;
  }
 return 0;
}

struct failint GetPeerIdBySock(struct VariableShare * vsh,int clientsock)
{
  struct failint retres={0};
  retres.failed=1;
  retres.value=0;

  unsigned int i=0;
  while ( i < vsh->total_peers )
   {
      if (vsh->peer_list[i].socket_to_client == clientsock )
       {
           retres.failed=0;
           retres.value=i;
           return retres;
       }
      ++i;
   }
  return retres;
}

int RemPeerBySock(struct VariableShare * vsh,int clientsock)
{
  struct failint peer_find=GetPeerIdBySock(vsh,clientsock);
  if (peer_find.failed) { error("Peerfind error at rempeerbysock"); }

 return RemPeer(vsh,peer_find.value);
}

int PeerNewPingValue(struct VariableShare * vsh,unsigned int peer_id,long ping_in_microseconds)
{
  if (vsh==0) { return 0; }
  if (vsh->total_peers<=peer_id) { return 0; }
  vsh->peer_list[peer_id].ping_in_microseconds=ping_in_microseconds;
  fprintf(stderr,"New ping value for peer %u = %u milliseconds \n",peer_id,(unsigned int) ping_in_microseconds/1000);
  return 1;
}

