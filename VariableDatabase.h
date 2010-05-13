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
struct VariableShare * Create_VariableDatabase(char * sharename,char * IP,unsigned int port,char * password,unsigned int newsize);
int AddVariable_Database(struct VariableShare * vsh,char * var_name,unsigned int permissions,void * ptr,unsigned int ptr_size);
int Destroy_VariableDatabase(struct VariableShare * vsh);

int AddVariable_Database(struct VariableShare * vsh,char * var_name,unsigned int permissions,void * ptr,unsigned int ptr_size);
int DeleteVariable_Database(struct VariableShare * vsh,char * var_name);
signed int FindVariable_Database(struct VariableShare * vsh,char * var_name);

int CanWriteTo_VariableDatabase(struct VariableShare * vsh,unsigned int var_spot);
int CanReadFrom_VariableDatabase(struct VariableShare * vsh,unsigned int var_spot);


int RefreshLocalVariable_VariableDatabase(struct VariableShare * vsh,char * variable_name);
int RefreshRemoteVariable_VariableDatabase(struct VariableShare * vsh,char * variable_name);

int StartAutoRefreshVariable(struct VariableShare * vsh);
#endif // VARIABLEDATABASE_H_INCLUDED
