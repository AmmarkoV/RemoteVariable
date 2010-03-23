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
    WRITEVAR
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

#endif // NETWORKFRAMEWORK_H_INCLUDED
