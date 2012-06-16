#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../RemoteVariableSupport.h"


#include <time.h>


int wait_for_var_to_become_x(volatile int * var , unsigned int timeout , unsigned int x)
{
  unsigned int time_waited=0;
  while ( (*var!= x )&&(time_waited<timeout) ) { usleep(1000); ++time_waited; }
  if (*var==x) {   return 1;}

  fprintf(stderr,"\n\nTIMEOUT WAITING FOR %u \n\n",x);
  return 0;
}


int main()
{
    unsigned int WAIT_TIME=20000;
    printf("REMOTE VARIABLES CLIENT STARTUP !!!!!!!!!!!!! \n");
    struct VariableShare * vsh = RVS_ConnectToVariableShare("SHARE2","127.0.0.1",12345,"password");
    if ( vsh == 0 )
      {
        fprintf(stderr,"Client : Error Creating share");
        return 1;
      }

    srand(time(0));
    if (!RVS_InternalTest())
      {
        fprintf(stderr,"Client : Failed preliminary internal test\n");
        return 1;
      }

    RVS_SetPolicy(vsh,VSP_MANUAL);

    static volatile int DUMMY_VAR=0;
    static volatile int SHARED_VAR=0;
    static volatile char MSG[32]={0};
    if ( ! RVS_AddVariable(vsh,"SHARED_VAR",7,&SHARED_VAR,sizeof(SHARED_VAR)) ) { fprintf(stderr,"Client : Error Adding SHARED_VAR Shared Variable , cannot continue\n"); return 1; }
    if ( ! RVS_AddVariable(vsh,"DUMMY_VAR",7,&DUMMY_VAR,sizeof(DUMMY_VAR)) ) { fprintf(stderr,"Client : Error Adding DUMMY_VAR Shared Variable , cannot continue\n"); return 1; }
    if ( ! RVS_AddVariable(vsh,"MESSAGE",7,&MSG,32) ) { fprintf(stderr,"Client : Error Adding MSG Shared Variable , cannot continue\n"); return 1; }


    printf("Client : Starting Self Test , Waiting to get the 666 initial value!\n");
    if ( !wait_for_var_to_become_x(&SHARED_VAR,WAIT_TIME,666)) { fprintf(stderr,"Client : Failed the test PRE1\n"); return 1; }

    printf("Client : Master wrote to us `%s` \n",MSG);



    printf("Client : TEST STEP 1 , setting SHARED VAR to 1\n");
    SHARED_VAR=1;
     Refresh_AllLocalVariables(vsh);

    printf("Client : Starting Self Test waiting for value 2 from peer !\n");
    int i=2;
    for (i=2; i<=100; i+=2)
    {
     if (!wait_for_var_to_become_x(&SHARED_VAR,WAIT_TIME,i)) { fprintf(stderr,"Client : Failed the test STEP%u , waiting for %u\n",i,i); return 1; }


     fprintf(stderr,"TEST STEP %u\n",i+1);
     DUMMY_VAR=rand()%10000;
     //usleep(rand()%10000);

     SHARED_VAR=i+1;
     Refresh_AllLocalVariables(vsh);
     printf("Client : Now we have changed the variable to %u , will wait until it becomes %u\n",i+1,i+2);
    }


    fprintf(stderr,"TEST STEPS DONE \n");
    printf("Client : Test is successfull!\n");
    i=system("aplay Documentation/sound.wav");
    if (i!=0 ) {fprintf(stderr,"Failed to play sound\n"); }
    fprintf(stderr,"Client : Test is successfull!\n");


    printf("Client : Master wrote to us `%s` \n",MSG);

    fprintf(stderr,"Client : Cleaning VariableShare context!\n");
    if ( RVS_StopVariableShare(vsh) == 0 ) fprintf(stderr,"Client : Error Deleting share\n");
    fprintf(stderr,"Client : Complete Halt!\n");

    return 0;
}
