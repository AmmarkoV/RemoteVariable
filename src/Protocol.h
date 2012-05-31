#ifndef PROTOCOL_H_INCLUDED
#define PROTOCOL_H_INCLUDED


#include "RemoteVariableSupport.h"
int Start_Version_Handshake(struct VariableShare * vsh,int peersock);
int Accept_Version_Handshake(struct VariableShare * vsh,int peersock);

#endif // PROTOCOL_H_INCLUDED
