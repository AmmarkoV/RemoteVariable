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

#include "NetworkFramework.h"
#include "helper.h"
/*
  TODO
  SPAWN A NEW LISTEN THEAD WHEN
  Start_VariableSharing is called
*/



/*
  TODO
  SPAWN A NEW CONNECTION THEAD WHEN
  ConnectToRemote_VariableSharing is called
*/


/*
  TODO
  KILL EVERYTHING WHEN
  Stop_VariableSharing is called
*/

/*
  TODO INTERCEPT REQUESTS FOR VARIABLES
  PASS THEM TO VARIABLE DATABASE FOR CHECK
  AND IF THEY PASS THE CHECK send THEM
*/
int GeneralPacket_ConvertTo_VariablePacket(struct NetworkRequestVariablePacket * variablepack , struct NetworkRequestGeneralPacket * generalpack )
{
 debug_say("GeneralPacket_ConvertTo_VariablePacket not implemented");
 return 0;
}

int
StartRemoteVariableServer(unsigned int port)
{
  if ( debug_msg() ) printf("TFTPServer\n");
  int sock, length, fromlen, n;
  struct sockaddr_in server;
  struct sockaddr_in from;

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if ( sock < 0 )
      error("Opening socket");

  length = sizeof (server);
  bzero(&server, length);
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(port);

  if ( bind(sock, (struct sockaddr *) & server, length) < 0 ) error("binding master port for atftp!");
  fromlen = sizeof (struct sockaddr_in);
  char filename[512]={0};
  int fork_res, packeterror = 0;
  while (1)
  {
      struct NetworkRequestGeneralPacket request={0}; // = { 0 };

     debug_say("Waiting for a tftp client");


      n = recvfrom(sock, (char*) & request, sizeof (request), 0, (struct sockaddr *) & from, &fromlen);

      if ( n < 0 ) error("Error RecvFrom , dropping session"); else
     { //RECEIVED PACKET OK
         packeterror = 0;
         /*DISASSEMBLE REQUEST @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
           RQST ID   PAYLOAD SIZE   PAYLOAD DATA
           1 byte   |   2 bytes   |   n bytes
              A            B            C
         */
         if (request.RequestType>=INVALID_TYPE) { packeterror=1; } else
         if (request.RequestType==ERROR)
         { // ERROR PACKET
           packeterror=1;
           error("Error Packet Received");
         }


         if ( packeterror == 0 )
         { debug_say("No Packet Request Error , Passing through to VariableDatabase Check");
           if ( (request.RequestType==READVAR) || (request.RequestType==WRITEVAR) )
           {
              debug_say("Generic Packet should contain Variable Packet as a payload");
              struct NetworkRequestVariablePacket * variablepacket=0;
              variablepacket (struct NetworkRequestVariablePacket * ) malloc(request.data_size);
              GeneralPacket_ConvertTo_VariablePacket(variablepacket,&request);
              // LOOOOOOOOOOOOOOOOOOOOOOTS OF THINGS TODO :P
           }
         }


     }//RECEIVED PACKET OK

  }
/*
      packeterror = 0;
      // DISASSEMBLE TFTP PACKET! @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
      // 2 bytes 1 byte 1byte
      // RRQ/WRQ | opcode | filename | 0 | Mode | 0
      // A B C D E
      // A part
      if ( (request.Op1 != 0) || ((request.Op2 != 2) && (request.Op2 != 1)) )
      {
          packeterror = 1;
      }
      // B part
      //write(1, request.data, n - 2);
      strcpy(filename, request.data);
      unsigned int fnm_end = strlen(filename);
      //CHECK FOR INCORRECT FILENAMES!
      if ( SecurityFilename(filename) == -1 )
      {
          packeterror = 1;
          if ( error_msg() ) printf("Insecure filename string.. , failing packet \n");
          TransmitError("Insecure filename ", 2, sock, &from);
      }
      else
          if ( fnm_end == 0 )
      {
          packeterror = 1;
          if ( error_msg() ) printf("Null filename.. , failing packet \n");
          TransmitError("Null filename ", 3, sock, &from);
      }
      // DISASSEMBLE TFTP PACKET! @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


      if ( packeterror == 0 )
      {
          fork_res = fork();
          if ( fork_res < 0 )
          {
              if ( error_msg() ) printf("Could not fork server , client will fail\n");
              TransmitError("Cannot fork accept connection", 1, sock, &from);
          }
          else if ( fork_res == 0 )
          {
              int f_fromlen = fromlen;
              struct sockaddr_in f_from = from;
              if ( debug_msg() ) printf("New UDP server fork to serve file %s , operation type %u \n", filename, request.Op2);
              fflush(stdout);
               //check if root

              if ( getuid() == ROOT_ID )
              {
                  setuid(1000);
                  if ( debug_msg() ) printf("Switched from root(uid=%d) to normal user(uid=%d)\n", ROOT_ID, getuid());
              }

              HandleClient(filename, f_fromlen, f_from, request.Op2);
          }
          else
          {
              // Server loop
          }
      }
      else
      {
          printf("TFTP Server master thread - Incoming Request Denied..\n");
          fflush(stdout);
          write(1, request.data, n - 2);
      }*/


  return;
}
