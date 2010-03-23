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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "RemoteVariableSupport.h"
#include "NetworkFramework.h"

struct VariableShare * Start_VariableSharing(char * sharename,char * password)
{
    fprintf(stderr,"Starting Variable Sharing!\n");
    struct VariableShare *  vsh = Create_VariableDatabase(sharename,"127.0.0.1",12345,password,128);
    StartRemoteVariableServer(12345);
    return vsh;
}

struct VariableShare * ConnectToRemote_VariableSharing(char * IP,unsigned int port,char * password)
{
    struct VariableShare *  vsh = Create_VariableDatabase("RemoteShare",IP,port,password,128);
    fprintf(stderr,"ConnectToRemote_VariableSharing not implemented yet!");
    return vsh;
}


int Stop_VariableSharing(struct VariableShare * vsh)
{
    stop_server_thread=1;

    Destroy_VariableDatabase(vsh);
    return 0;
}

int Add_VariableToSharingList(struct VariableShare * vsh,char * variable_name,unsigned int permissions,void * ptr,unsigned int ptr_size)
{
    return AddVariable_ToDatabase(vsh,variable_name,permissions,ptr,ptr_size);
}

int Delete_VariableFromSharingList(struct VariableShare * vsh,char * variable_name)
{
    fprintf(stderr,"Delete_VariableFromSharingList not implemented yet!");
    return 0;
}

int Refresh_LocalVariable(struct VariableShare * vsh,char * variable_name)
{
    fprintf(stderr,"Refresh_LocalVariable not implemented yet!");
    return 0;
}
