#ifndef REMOTEVARIABLESUPPORT_H_INCLUDED
#define REMOTEVARIABLESUPPORT_H_INCLUDED

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


#ifdef __cplusplus
extern "C" {
#endif

struct ShareListItem
{
    void * ptr;
    unsigned int size_of_ptr;

    char * ptr_name;
    unsigned int size_of_ptr_name;

    char permissions[4];
};



struct ShareList
{
    unsigned int total_variables_memory;
    unsigned int total_variables_shared;
    struct ShareListItem * variables;
};

struct VariableShare
{
    char * sharename;

    char * ip;
    unsigned int port;
    int state;

    struct ShareList share;

};


struct VariableShare * Start_VariableSharing(char * sharename,char * password);
struct VariableShare * ConnectToRemote_VariableSharing(char * IP,unsigned int port,char * password);
int Stop_VariableSharing(struct VariableShare * vsh);
int Add_VariableToSharingList(struct VariableShare * vsh,char * variable_name,char * permissions,void * ptr,unsigned int ptr_size);
int Delete_VariableFromSharingList(struct VariableShare * vsh,char * variable_name);
int Refresh_LocalVariable(struct VariableShare * vsh,char * variable_name);

#ifdef __cplusplus
}
#endif


#endif // REMOTEVARIABLESUPPORT_H_INCLUDED
