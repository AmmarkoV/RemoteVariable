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


int
StartRemoteVariableServer(unsigned int port)
{
  if ( debug_msg() ) printf("TFTPServer\n");
  int sock, length, fromlen, n;
  struct sockaddr_in server;
  struct sockaddr_in from;

  sock = socket(AF_INET, SOCK_DGRAM, 0);
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
      struct TFTP_PACKET request; // = { 0 };
      if ( debug_msg() )
          printf("\n Waiting for a tftp client \n");
      n = recvfrom(sock, (char*) & request, sizeof (request), 0, (struct sockaddr *) & from, &fromlen);
      if ( n < 0 )
          error("recvfrom");
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
      }
  }

  return;
}
