#include "Peers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int AddMaster(struct VariableShare * vsh,char * name,unsigned int port , int clientsock)
{
  unsigned int name_len = strlen(name);
  if ( name_len > RVS_MAX_SHARE_IP_CHARS ) { name_len = RVS_MAX_SHARE_IP_CHARS; }
 // strncpy(vsh->master.IP,name,name_len);
  vsh->master.port=port;
  vsh->master.socket_to_client=clientsock;
  return 1;
}

int AddPeer(struct VariableShare * vsh,char * name,unsigned int port , int clientsock)
{
  if (vsh->peers_active < RVS_MAX_PEERS)
   {
       unsigned int pos = vsh->peers_active;

       strncpy(vsh->peer_list[pos].IP,"TODO",4);
       //strncpy(vsh->peer_list[pos].IP,name,RVS_MAX_SHARE_IP_CHARS);
       vsh->peer_list[pos].port=port;
       vsh->peer_list[pos].socket_to_client = clientsock;
       vsh->peer_list[pos].socket_locked=0;
       ++vsh->peers_active;
       return 1;
   }
  return 0;
}

int SwapPeers(struct VariableShare * vsh,int peer_id1,int peer_id2)
{
 if ( (peer_id1<vsh->peers_active)&&(peer_id2<vsh->peers_active) )
  {
    struct SharePeer temp=vsh->peer_list[peer_id1];
    vsh->peer_list[peer_id1]=vsh->peer_list[peer_id2];
    vsh->peer_list[peer_id2]=temp;
  } else
  {
    fprintf(stderr,"SwapPeers called for peers %u and %u which are out of bounds ( %u ) \n",peer_id1,peer_id2,vsh->peers_active);
    return 0;
  }
 return 1;
}


int RemPeer(struct VariableShare * vsh,int peer_id)
{
 if ( (vsh->peers_active==1)&&(peer_id==0)) { vsh->peers_active=0; return 1; }
 if ( (vsh->peers_active>1) )
  {
    unsigned int last_peer = vsh->peers_active-1;
    SwapPeers(vsh,peer_id,last_peer);
    --vsh->peers_active;
    return 1;
  }
 return 0;
}

int GetPeerIdBySock(struct VariableShare * vsh,int clientsock)
{
  unsigned int i=0;
  while ( i < vsh->peers_active )
   {
      if (vsh->peer_list[i].socket_to_client == clientsock )
       {
           return i;
       }
      ++i;
   }
  return 0;
}



int RemPeerBySock(struct VariableShare * vsh,int clientsock)
{
 return RemPeer(vsh,GetPeerIdBySock(vsh,clientsock));
}
