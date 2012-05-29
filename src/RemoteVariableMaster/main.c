#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../RemoteVariableSupport.h"



int wait_for_var_to_become_x(volatile int * var , unsigned int timeout , unsigned int x)
{
  unsigned int time_waited=0;
  while ( (*var!= x )&&(time_waited<timeout) ) { usleep(1000);  ++time_waited; }
  if (*var==x) {  return 1;}

  return 0;
}


int main()
{
    unsigned int WAIT_TIME=20000;
    printf("REMOTE VARIABLES MASTER STARTUP!!!!!!!!!!!!! \n");
    struct VariableShare * vsh = Start_VariableSharing("SHARE2","127.0.0.1",12345,"password");
    if ( vsh == 0 )
     {
         fprintf(stderr,"Master : Error Creating share\n");
         return 1;
     }


    // We start up with a Variable with the value 666 , we expect the client program to alter it to 1 after it successfully connects
    static volatile int SHARED_VAR=0;
    if ( Add_VariableToSharingList(vsh,"SHARED_VAR",7,&SHARED_VAR,sizeof(SHARED_VAR)) == 0 )
     {
      fprintf(stderr,"Master : Error Adding Shared Variable\n");
      return 1;
     }

   fprintf(stderr,"Master : Waiting for a peer to startup test..\n");
    while (!PeersActive_VariableShare(vsh))
     {
       fprintf(stderr,"*");
       usleep(1000);
     }
   fprintf(stderr,"Master : Peer found , proceeding..\n");


    fprintf(stderr,"TEST STEP 1\n");
    SHARED_VAR=666; // This will propagate to the client



    printf("Master : Starting Self Test waiting for value 1 from peer !\n");
    int i=1;
    for (i=1; i<100; i+=2)
    {
     if (!wait_for_var_to_become_x(&SHARED_VAR,WAIT_TIME,i)) { fprintf(stderr,"Master : Failed the test STEP%u , waiting for %u\n",i,i); return 1; }

     fprintf(stderr,"TEST STEP %u\n",i+1);
     SHARED_VAR=i+1;
     printf("Master : Now we have changed the variable to %u , will wait until it becomes %u\n",i+1,i+2);
    }


    MakeSureVarReachedPeers_RemoteVariable(vsh,"SHARED_VAR",5000);
    usleep(1000);

    printf("\nMaster: Test is successfull\n");
    fprintf(stderr,"Master : Test is successfull!\n");

    fprintf(stderr,"Master : Closing things down \n");

    fprintf(stderr,"Master : Cleaning VariableShare context!\n");
    if (!Stop_VariableSharing(vsh))
     {
         fprintf(stderr,"Master : Error Stopping Variable share\n");
         return 1;
     }
    fprintf(stderr,"Master : Complete Halt!\n");


    return 0;
}
