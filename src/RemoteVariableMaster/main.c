#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../RemoteVariableSupport.h"

#include <time.h>


int wait_for_var_to_become_x(volatile int * var , unsigned int timeout , unsigned int x)
{
  unsigned int time_waited=0;
  while ( (*var!= x )&&(time_waited<timeout) ) { usleep(1000);  ++time_waited; }
  if (*var==x) {  return 1;}

  fprintf(stderr,"\n\nTIMEOUT WAITING FOR %u \n\n",x);
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

    srand(time(0));

    if (!RemoteVariableSupport_InternalTest())
      {
        fprintf(stderr,"Master : Failed preliminary internal test\n");
        return 1;
      }


    SetPolicy(vsh,VSP_MANUAL);

    // We start up with a Variable with the value 666 , we expect the client program to alter it to 1 after it successfully connects
    static volatile int DUMMY_VAR=0;
    static volatile int SHARED_VAR=0;
    if ( Add_VariableToSharingList(vsh,"SHARED_VAR",7,&SHARED_VAR,sizeof(SHARED_VAR)) == 0 )
     {
      fprintf(stderr,"Master : Error Adding Shared Variable\n");
      return 1;
     }
   if ( ! Add_VariableToSharingList(vsh,"DUMMY_VAR",7,&DUMMY_VAR,sizeof(DUMMY_VAR)) )
      {
        fprintf(stderr,"Master : Error Adding DUMMY_VAR Shared Variable , cannot continue\n");
        return 1;
      }



   fprintf(stderr,"Master : Waiting for a peer to startup test..\n");
    while (!PeersActive_VariableShare(vsh))
     {
       fprintf(stderr,"*");
       usleep(1000);
     }
   fprintf(stderr,"Master : Peer found , proceeding..\n");


    printf("Master : TEST STEP 1 , setting SHARED VAR to 666\n");
    SHARED_VAR=666; // This will propagate to the client
     Refresh_AllLocalVariables(vsh);



    printf("Master : Starting Self Test waiting for value 1 from peer !\n");
    int i=1;
    for (i=1; i<100; i+=2)
    {
     if (!wait_for_var_to_become_x(&SHARED_VAR,WAIT_TIME,i)) { fprintf(stderr,"Master : Failed the test STEP%u , waiting for %u\n",i,i); return 1; }
     DUMMY_VAR=rand()%10000;
     fprintf(stderr,"TEST STEP %u\n",i+1);
     SHARED_VAR=i+1;
     Refresh_AllLocalVariables(vsh);
     printf("Master : Now we have changed the variable to %u , will wait until it becomes %u\n",i+1,i+2);
    }


    MakeSureVarReachedPeers_RemoteVariable(vsh,"SHARED_VAR",5000);
    usleep(1000);

    printf("\nMaster: Test is successfull\n");
    fprintf(stderr,"Master : Test is successfull!\n");
    i=system("aplay Documentation/sound.wav");
    if (i!=0 ) {fprintf(stderr,"Failed to play sound\n"); }

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
