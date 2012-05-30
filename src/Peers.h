#ifndef PEERS_H_INCLUDED
#define PEERS_H_INCLUDED

#include "RemoteVariableSupport.h"

int AddMaster(struct VariableShare * vsh,char * name,unsigned int port , int clientsock);
int AddPeer(struct VariableShare * vsh,char * name,unsigned int port , int clientsock) ;
int SwapPeers(struct VariableShare * vsh,int peer_id1,int peer_id2) ;
int RemPeer(struct VariableShare * vsh,int peer_id) ;
int GetPeerIdBySock(struct VariableShare * vsh,int clientsock);
int RemPeerBySock(struct VariableShare * vsh,int clientsock) ;
int PeerNewPingValue(struct VariableShare * vsh,unsigned int peer_id,long ping_in_microseconds);

#endif // PEERS_H_INCLUDED
