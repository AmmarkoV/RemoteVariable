#include "Peers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int AddPeer(struct VariableShare * vsh,char * name,unsigned int port , int clientsock)
{
  if (vsh->total_peers < RVS_MAX_PEERS)
   {
       unsigned int pos = vsh->total_peers;

       memset(vsh->peer_list[pos].IP,0,RVS_MAX_SHARE_IP_CHARS);
       strncpy(vsh->peer_list[pos].IP,name,RVS_MAX_SHARE_IP_CHARS);
       vsh->peer_list[pos].port=port;
       vsh->peer_list[pos].socket_to_client = clientsock;
       vsh->peer_list[pos].peer_state=VSS_WAITING_FOR_HANDSHAKE;
       vsh->peer_list[pos].incremental_value=0;

       AllocateMessageQueue(&vsh->peer_list[pos].message_queue);

       ++vsh->total_peers;
       return pos+1;
   }
  return 0;
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
  FreeMessageQueue(&vsh->peer_list[peer_id].message_queue);

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

int GetPeerIdBySock(struct VariableShare * vsh,int clientsock)
{
  unsigned int i=0;
  while ( i < vsh->total_peers )
   {
      if (vsh->peer_list[i].socket_to_client == clientsock )
       {
           return 1+i;
       }
      ++i;
   }
  return 0;
}



int RemPeerBySock(struct VariableShare * vsh,int clientsock)
{
 return RemPeer(vsh,GetPeerIdBySock(vsh,clientsock));
}



int PeerNewPingValue(struct VariableShare * vsh,unsigned int peer_id,long ping_in_microseconds)
{
  if (vsh==0) { return 0; }
  if (vsh->total_peers<=peer_id) { return 0; }
  vsh->peer_list[peer_id].ping_in_microseconds=ping_in_microseconds;
  fprintf(stderr,"New ping value for peer %u = %u milliseconds \n",peer_id,(unsigned int) ping_in_microseconds/1000);
  return 1;
}

