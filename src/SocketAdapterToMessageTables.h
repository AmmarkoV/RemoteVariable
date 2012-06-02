#ifndef SOCKETADAPTERTOMESSAGETABLES_H_INCLUDED
#define SOCKETADAPTERTOMESSAGETABLES_H_INCLUDED

#include "RemoteVariableSupport.h"

void PrintMessageType(struct PacketHeader *header);
void * SocketAdapterToMessageTable_Thread(void * ptr);
void * JobAndMessageTableExecutor_Thread(void * ptr);

#endif // SOCKETADAPTERTOMESSAGETABLES_H_INCLUDED
