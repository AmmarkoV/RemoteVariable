/***************************************************************************
* Copyright (C) 2010 by Ammar Qammaz *
* ammarkov@gmail.com *
* *
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or *
* (at your option) any later version. *
* *
* This program is distributed in the hope that it will be useful, *
* but WITHOUT ANY WARRANTY; without even the implied warranty of *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the *
* GNU General Public License for more details. *
* *
* You should have received a copy of the GNU General Public License *
* along with this program; if not, write to the *
* Free Software Foundation, Inc., *
* 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. *
***************************************************************************/

#ifndef NETWORKFRAMEWORK_H_INCLUDED
#define NETWORKFRAMEWORK_H_INCLUDED

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/uio.h>

#include "RemoteVariableSupport.h"

enum RequestTypeEnum
{
    OK=0,
    READVAR,
    WRITEVAR,
    ERROR,
    INVALID_TYPE
};


struct NetworkRequestGeneralPacket
{
  char RequestType;
  unsigned int data_size;
};


int GetPeerIdBySock(struct VariableShare * vsh,int clientsock);

void RemoteVariableClient_Thread_Pause(struct VariableShare * vsh);
void RemoteVariableClient_Thread_Resume(struct VariableShare * vsh);
int StartRemoteVariableServer(struct VariableShare * vsh);

void RemoteVariableServer_Thread_Pause(struct VariableShare * vsh);
void RemoteVariableServer_Thread_Resume(struct VariableShare * vsh);
int StartRemoteVariableConnection(struct VariableShare * vsh);


#endif // NETWORKFRAMEWORK_H_INCLUDED
