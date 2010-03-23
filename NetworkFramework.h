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
#include <unistd.h>
#include <sys/uio.h>

enum RequestTypeEnum
{
    READVAR=0,
    WRITEVAR,
    ERROR,
    INVALID_TYPE
};

struct NetworkRequestVariablePacket
{
  unsigned char RequestType;

  unsigned int name_size;
  unsigned char * name;

  unsigned int payload_size;
  unsigned char * payload;

};

struct NetworkRequestGeneralPacket
{
  unsigned char RequestType;
  unsigned int data_size;
  unsigned char * data;
};

extern unsigned int stop_server_thread;

int StartRemoteVariableServer(unsigned int port);

#endif // NETWORKFRAMEWORK_H_INCLUDED
