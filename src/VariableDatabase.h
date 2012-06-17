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

#ifndef VARIABLEDATABASE_H_INCLUDED
#define VARIABLEDATABASE_H_INCLUDED

#include "RemoteVariableSupport.h"

int VariableShareOk(struct VariableShare * vsh);
int VariableIdExists(struct VariableShare * vsh,unsigned int var_id);

struct VariableShare * Create_VariableDatabase(char * sharename,char * IP,unsigned int port,char * password,unsigned int newsize);

int Destroy_VariableDatabase(struct VariableShare * vsh);

unsigned long GetVariableHash(struct VariableShare * vsh,void * ptr,unsigned int size_of_ptr);
unsigned long GetVariableHashForVar(struct VariableShare * vsh,unsigned int var_id);

int AddVariable_Database(struct VariableShare * vsh,char * var_name,unsigned int permissions,volatile void * ptr,unsigned int ptr_size);
int DeleteVariable_Database(struct VariableShare * vsh,unsigned int var_id);
struct failint FindVariable_Database(struct VariableShare * vsh,char * var_name);


int  MarkVariableAsNeedsRefresh_VariableDatabase(struct VariableShare * vsh,unsigned int var_id,int peer_id);

//int CanWriteTo_VariableDatabase(struct VariableShare * vsh,unsigned int var_spot);
//int CanReadFrom_VariableDatabase(struct VariableShare * vsh,unsigned int var_spot);
int CheckForChangedVariables(struct VariableShare * vsh);
int SignalUpdatesForAllLocalVariablesThatNeedIt(struct VariableShare * vsh);
int RefreshAllVariablesThatNeedIt(struct VariableShare *vsh);

int NewRemoteValueForVariable(struct VariableShare * vsh,unsigned int peer_id,unsigned int var_id,void * new_val,unsigned int new_val_size,unsigned int time);

//int RefreshLocalVariable_VariableDatabase(struct VariableShare * vsh,char * variable_name);
//int RefreshRemoteVariable_VariableDatabase(struct VariableShare * vsh,char * variable_name);

int MakeSureVarReachedPeers(struct VariableShare *vsh,char * varname,unsigned int wait_time);

void AutoRefreshVariable_Thread_Pause(struct VariableShare * vsh);
void AutoRefreshVariable_Thread_Resume(struct VariableShare * vsh);
int StartAutoRefreshVariable(struct VariableShare * vsh);
#endif // VARIABLEDATABASE_H_INCLUDED
