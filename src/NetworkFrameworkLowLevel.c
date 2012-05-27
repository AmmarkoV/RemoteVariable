#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "VariableDatabase.h"
#include "NetworkFramework.h"
#include "NetworkFrameworkLowLevel.h"

#include "JobTables.h"
#include "Peers.h"
#include "helper.h"
#include "ProtocolThreads.h"
#include "RemoteVariableSupport.h"


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/uio.h>

int SendRAWTo(int clientsock,char * message,unsigned int length)
{
  fprintf(stderr,"Trying to send `%s` to peer socket %d \n",message,clientsock);
  return send(clientsock,message,length, 0);
}

int RecvRAWFrom(int clientsock,char * message,unsigned int length)
{
  fprintf(stderr,"Trying RecvRAWFrom\n");
  memset (message,0,length);

  int retres=recv(clientsock,message,length, 0);

  fprintf(stderr,"Received `%s` from peer socket %d  \n",message,clientsock);
  return retres;
}

int RecvVariableFrom(struct VariableShare * vsh,int clientsock,unsigned int variable_id)
{
 struct NetworkRequestGeneralPacket data;
 data.RequestType = READFROM ; // This should remain readfrom after receiving the general packet..!
 data.data_size = vsh->share.variables[variable_id].size_of_ptr;


 fprintf(stderr,"Receiving GeneralPacket\n");
 int data_recv= recv(clientsock, (char*) & data, sizeof (data), 0); // SEND START FRAME!
 if (data_recv != sizeof (data) ) { fprintf(stderr,"Incorrect starting frame received ( %u instead of %u )\n",data_recv , sizeof (data) ); return 0; }
 if (data.data_size != vsh->share.variables[variable_id].size_of_ptr ) { fprintf(stderr,"Incorrect pointer size at frame received ( %u instead of %u )\n",data.data_size , vsh->share.variables[variable_id].size_of_ptr ); return 0; }


 //TODO BEEF UP SECURITY HERE :P
 fprintf(stderr,"Receiving Payload ( var was %d ",vsh->share.variables[variable_id].ptr);
 data_recv= recv(clientsock,vsh->share.variables[variable_id].ptr,data.data_size, 0); // GET VAR PAYLOAD!


 fprintf(stderr,"now %d  )\n",vsh->share.variables[variable_id].ptr);
 if ( data_recv != data.data_size ) { fprintf(stderr,"Incorrect payload received ( %u instead of %u )\n",data_recv , vsh->share.variables[variable_id].size_of_ptr ); return 0; }
 if ( vsh->share.variables[variable_id].GUARD_BYTE != RVS_GUARD_VALUE ) {  error("Buffer overflow attack @ RecvVariableFrom detected\n"); vsh->global_state=VSS_SECURITY_ALERT; return 0; }

 // TODO CHECK OPERATIONS ETC HERE!
 fprintf(stderr,"Updating hash value  \n");
 vsh->share.variables[variable_id].hash=GetVariableHash(vsh,variable_id);

 fprintf(stderr,"RecvVariableFrom , Great Success :) \n");
  return 1;
}

int SendVariableTo(struct VariableShare * vsh,int clientsock,unsigned int variable_id)
{
  struct NetworkRequestGeneralPacket data;
  data.RequestType=WRITEVAR;
  data.data_size = vsh->share.variables[variable_id].size_of_ptr;

  fprintf(stderr,"Sending GeneralPacket\n");
  int data_sent= send(clientsock, (char*) & data, sizeof (data), 0); // SEND START FRAME!
  if ( data_sent != sizeof (data) ) { fprintf(stderr,"Incorrect starting frame transmission ( %d instead of %u )\n",data_sent , sizeof (data) ); return 0; }

  fprintf(stderr,"Sending Payload\n");
  data_sent= send(clientsock,vsh->share.variables[variable_id].ptr,data.data_size , 0); // SEND VARIABLE!
  if ( data_sent != data.data_size ) { fprintf(stderr,"Incorrect payload transmission ( %d instead of %u )\n",data_sent , data.data_size ); return 0; }


  fprintf(stderr,"SendVariableTo , Great Success :) \n");
  return 1;
}


int RecvFileFrom(struct VariableShare * vsh,int clientsock,unsigned int variable_id)
{
  fprintf(stderr,"RecvFileFrom not implemented\n");
  return 0;
}

int SendFileTo(struct VariableShare * vsh,int clientsock,unsigned int variable_id)
{
  fprintf(stderr,"SendFileTo not implemented\n");
  return 0;
}


int WaitForSocketLockToClear(int peersock,unsigned int * peerlock)
{
  return 1;
  if (peerlock==0) { fprintf(stderr,"WaitForSocketLock skipped by null pointer \n"); return 0; }
  unsigned int waitloops=0;
  while (*peerlock)
   {
     usleep(1000);
     ++waitloops;
   }

  if (waitloops) { fprintf(stderr,"WaitForSocketLockToClear waited for %u ms ( loops )\n",waitloops); }
  return 1;
}

int LockSocket(int peersock,unsigned int * peerlock)
{
  return 1;
  if (peerlock==0) { fprintf(stderr,"LockSocket skipped by null pointer \n"); return 0; }
  if (*peerlock!=0) {
                      fprintf(stderr,"LockSocket found locked variable ! , waiting for it to clear out\n");
                      WaitForSocketLockToClear(peersock,peerlock);
                    }
  *peerlock=1;
  return 1;
}

int UnlockSocket(int peersock,unsigned int * peerlock)
{
  return 1;
  if (peerlock==0) { fprintf(stderr,"UnlockSocket skipped by null pointer \n"); return 0; }
  *peerlock=0;
  return 1;
}
