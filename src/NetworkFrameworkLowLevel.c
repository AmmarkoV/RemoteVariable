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

 void * temp_recv_mem_to_avoid_race = (void *) malloc(data.data_size);
 if (temp_recv_mem_to_avoid_race==0)
  {
     fprintf(stderr,"Could not make safe memory allocation\n");
     //TODO BEEF UP SECURITY HERE :P
     unsigned int * ptr_val = (unsigned int * ) vsh->share.variables[variable_id].ptr;
     fprintf(stderr,"Receiving Payload ( var was %u ",*ptr_val);
     data_recv= recv(clientsock,vsh->share.variables[variable_id].ptr,data.data_size, 0); // GET DirectPAYLOAD!
     if ( data_recv != data.data_size ) { fprintf(stderr,"Incorrect payload received ( %u instead of %u )\n",data_recv , vsh->share.variables[variable_id].size_of_ptr ); return 0; }
     ptr_val = (unsigned int * ) vsh->share.variables[variable_id].ptr;
     fprintf(stderr,"now %u  )\n",*ptr_val);

     fprintf(stderr,"Updating hash value for new payload ( %u ) , ",*ptr_val);
     vsh->share.variables[variable_id].hash=GetVariableHashForVar(vsh,variable_id);
     fprintf(stderr,"new hash value is %u\n",vsh->share.variables[variable_id].hash);

   }   else
   {
     unsigned int * ptr_val = (unsigned int * ) vsh->share.variables[variable_id].ptr;
     fprintf(stderr,"Receiving Payload ( var was %u ",*ptr_val);
     data_recv= recv(clientsock,temp_recv_mem_to_avoid_race,data.data_size, 0); // GET VAR PAYLOAD! vsh->share.variables[variable_id].ptr
     if ( data_recv != data.data_size ) { fprintf(stderr,"Incorrect payload received ( %u instead of %u )\n",data_recv , vsh->share.variables[variable_id].size_of_ptr ); return 0; }
     ptr_val = (unsigned int * ) vsh->share.variables[variable_id].ptr;
     fprintf(stderr,"now %u  )\n",*ptr_val);

     fprintf(stderr,"Updating hash value for new payload ( %u ) , ",*ptr_val);
     vsh->share.variables[variable_id].hash=GetVariableHash(vsh,temp_recv_mem_to_avoid_race,data.data_size);
     memcpy(vsh->share.variables[variable_id].ptr,temp_recv_mem_to_avoid_race,data.data_size);
     fprintf(stderr,"new hash value is %u\n",vsh->share.variables[variable_id].hash);

     free(temp_recv_mem_to_avoid_race);
   }







 if ( vsh->share.variables[variable_id].GUARD_BYTE1 != RVS_GUARD_VALUE ) {  error("Buffer overflow attack @ RecvVariableFrom detected\n"); vsh->global_state=VSS_SECURITY_ALERT; return 0; }
 if ( vsh->share.variables[variable_id].GUARD_BYTE2 != RVS_GUARD_VALUE ) {  error("Buffer overflow attack @ RecvVariableFrom detected\n"); vsh->global_state=VSS_SECURITY_ALERT; return 0; }

 // TODO CHECK OPERATIONS ETC HERE!



 fprintf(stderr,"RecvVariableFrom , Great Success :) \n");
  return 1;
}

int SendVariableTo(struct VariableShare * vsh,int clientsock,unsigned int variable_id)
{
  fprintf(stderr,"SendVariableTo socket %d , var_id %u \n",clientsock,variable_id);
  struct NetworkRequestGeneralPacket data={0};
  data.RequestType=WRITEVAR;
  data.data_size = vsh->share.variables[variable_id].size_of_ptr;

  fprintf(stderr,"Sending GeneralPacket\n");
  int data_sent= send(clientsock, (char*) & data, sizeof (data), 0); // SEND START FRAME!
  if ( data_sent != sizeof (data) ) { fprintf(stderr,"Incorrect starting frame transmission ( %d instead of %u )\n",data_sent , (unsigned int) sizeof (data) ); return 0; }

  fprintf(stderr,"Sending Payload\n");
  data_sent= send(clientsock,vsh->share.variables[variable_id].ptr,data.data_size , 0); // SEND VARIABLE!
  if ( data_sent != data.data_size ) { fprintf(stderr,"Incorrect payload transmission ( %d instead of %u )\n",data_sent , data.data_size ); return 0; }

  ++vsh->share.variables[variable_id].this_hash_transmission_count;

  fprintf(stderr,"SendVariableTo , Great Success , the variable with this hash has been transmitted %u times :) \n",vsh->share.variables[variable_id].this_hash_transmission_count);
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
