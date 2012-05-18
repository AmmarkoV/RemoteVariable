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

#ifndef PROTOCOLTHREADS_H_INCLUDED
#define PROTOCOLTHREADS_H_INCLUDED

#include "RemoteVariableSupport.h"
int Connect_Handshake(struct VariableShare * vsh,int clientsock);
int Accept_Handshake(struct VariableShare * vsh,int clientsock);
int InitCloneShare_Handshake(struct VariableShare * vsh);
int AcceptCloneShare_Handshake(struct VariableShare * vsh);
int RequestVariable_Handshake(struct VariableShare * vsh);
int SendVariable_Handshake(struct VariableShare * vsh);
int MasterReceive_Handshake(struct VariableShare * vsh);
int MasterSend_Handshake(struct VariableShare * vsh);


#endif // PROTOCOLTHREADS_H_INCLUDED
