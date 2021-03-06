#ifndef PEERS_H_INCLUDED
#define PEERS_H_INCLUDED

#include "RemoteVariableSupport.h"

struct failint AddPeer(struct VariableShare * vsh,char * name,unsigned int port , int clientsock) ;
int SwapPeers(struct VariableShare * vsh,int peer_id1,int peer_id2) ;
int RemPeer(struct VariableShare * vsh,int peer_id) ;
struct failint GetPeerIdBySock(struct VariableShare * vsh,int clientsock);
int RemPeerBySock(struct VariableShare * vsh,int clientsock) ;
int PeerNewPingValue(struct VariableShare * vsh,unsigned int peer_id,long ping_in_microseconds);


#endif // PEERS_H_INCLUDED
