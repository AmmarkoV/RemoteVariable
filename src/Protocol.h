#ifndef PROTOCOL_H_INCLUDED
#define PROTOCOL_H_INCLUDED


#include "RemoteVariableSupport.h"
int Start_Version_Handshake(struct VariableShare * vsh,int peersock);
int Accept_Version_Handshake(struct VariableShare * vsh,int peersock);


int Request_SignalChangeVariable(struct VariableShare * vsh,unsigned int peer_id,unsigned int var_id,int peersock, unsigned int mt_id, unsigned char * groupid , unsigned char * protocol_progress,unsigned int * last_protocol_mid);
int AcceptRequest_SignalChangeVariable(struct VariableShare * vsh,unsigned int peer_id,struct MessageTable * mt,unsigned int mt_id,int peersock, unsigned char * groupid , unsigned char * protocol_progress,unsigned int * last_protocol_mid);

int Request_WriteVariable(struct VariableShare * vsh,unsigned int peer_id,unsigned int var_id,int peersock, unsigned int mt_id, unsigned char * groupid , unsigned char * protocol_progress,unsigned int * last_protocol_mid);
int AcceptRequest_WriteVariable(struct VariableShare * vsh,unsigned int peer_id,struct MessageTable * mt,unsigned int mt_id,int peersock, unsigned char * groupid , unsigned char * protocol_progress,unsigned int * last_protocol_mid);

int Request_ReadVariable(struct VariableShare * vsh,unsigned int peer_id,unsigned int var_id,int peersock, unsigned int mt_id, unsigned char * groupid , unsigned char * protocol_progress,unsigned int * last_protocol_mid);
int AcceptRequest_ReadVariable(struct VariableShare * vsh,unsigned int peer_id,struct MessageTable * mt,unsigned int mt_id,int peersock, unsigned char * groupid , unsigned char * protocol_progress,unsigned int * last_protocol_mid);



#endif // PROTOCOL_H_INCLUDED
