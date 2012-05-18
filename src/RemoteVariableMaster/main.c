#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../RemoteVariableSupport.h"



int wait_for_var_to_become_x(int * var , unsigned int timeout , unsigned int x)
{
  unsigned int time_waited=0;
  while ( (*var!= x )&&(time_waited<timeout) ) { usleep(1000); ++time_waited; }
  if (*var==0) { fprintf(stderr,"Master : The experiment was a complete failure\n");  return 0;}

  return 1;
}


int main()
{
    printf("REMOTE VARIABLES MASTER !!!!!!!!!!!!! \n");
    printf("Hello world I Will try to test SHARED_VARS!\n");

    struct VariableShare * vsh = Start_VariableSharing("SHARE2","0.0.0.0",12345,"password");
    if ( vsh == 0 )
     {
         fprintf(stderr,"Master : Error Creating share\n");
         return 1;
     }



    // We start up with a Variable with the value 666 , we expect the client program to alter it to 1 after it successfully connects
    int SHARED_VAR=666;
    if ( Add_VariableToSharingList(vsh,"SHARED_VAR",7,&SHARED_VAR,sizeof(SHARED_VAR)) == 0 )
     {
      fprintf(stderr,"Master : Error Adding Shared Variable\n");
      return 1;
     }


    int time_waited=0;
    printf("Master : Starting Self Test!\n");
    wait_for_var_to_become_x(&SHARED_VAR,10000,1);

    SHARED_VAR=2;
    printf("Master : Now we have changed the variable to 2 , will wait until it becomes 3\n");
    wait_for_var_to_become_x(&SHARED_VAR,10000,3);

    SHARED_VAR=4;
    printf("Master : Now we have changed the variable to 4 , will wait until it becomes 5\n");
    wait_for_var_to_become_x(&SHARED_VAR,10000,5);

    SHARED_VAR=6;


    fprintf(stderr,"Master : Closing things down \n");
    if (!Stop_VariableSharing(vsh))
     {
         fprintf(stderr,"Master : Error Stopping Variable share\n");
         return 1;
     }

    return 0;
}
