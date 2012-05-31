#ifndef PROTOCOL_H_INCLUDED
#define PROTOCOL_H_INCLUDED


#include "RemoteVariableSupport.h"
int Start_Version_Handshake(struct VariableShare * vsh,int peersock);
int Accept_Version_Handshake(struct VariableShare * vsh,int peersock);


int SignalChange_Variable(struct VariableShare * vsh,unsigned int peer_id,unsigned int var_id,int peersock);
int Request_Variable(struct VariableShare * vsh,unsigned int peer_id,unsigned int var_id,int peersock);

#endif // PROTOCOL_H_INCLUDED
