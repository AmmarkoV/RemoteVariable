#ifndef NETWORKFRAMEWORKLOWLEVEL_H_INCLUDED
#define NETWORKFRAMEWORKLOWLEVEL_H_INCLUDED



int SendRAWTo(int clientsock,char * message,unsigned int length);
int RecvRAWFrom(int clientsock,char * message,unsigned int length);

int RecvVariableFrom(struct VariableShare * vsh,int clientsock,unsigned int variable_id);
int SendVariableTo(struct VariableShare * vsh,int clientsock,unsigned int variable_id);

int RecvFileFrom(struct VariableShare * vsh,int clientsock,unsigned int variable_id);
int SendFileTo(struct VariableShare * vsh,int clientsock,unsigned int variable_id);

int WaitForSocketLockToClear(int peersock,unsigned int * peerlock);
int LockSocket(int peersock,unsigned int * peerlock);
int UnlockSocket(int peersock,unsigned int * peerlock);


#endif // NETWORKFRAMEWORKLOWLEVEL_H_INCLUDED
