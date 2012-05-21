#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../RemoteVariableSupport.h"



int wait_for_var_to_become_x(int * var , unsigned int timeout , unsigned int x)
{
  unsigned int time_waited=0;
  while ( (*var!= x )&&(time_waited<timeout) ) { usleep(1000); if (time_waited%10==0) { printf(".m."); } ++time_waited; }
  if (*var==x) {  return 1;}

  return 0;
}


int main()
{
    unsigned int WAIT_TIME=100;
    printf("REMOTE VARIABLES MASTER STARTUP!!!!!!!!!!!!! \n");
    struct VariableShare * vsh = Start_VariableSharing("SHARE2","127.0.0.1",12345,"password");
    if ( vsh == 0 )
     {
         fprintf(stderr,"Master : Error Creating share\n");
         return 1;
     }


    // We start up with a Variable with the value 666 , we expect the client program to alter it to 1 after it successfully connects
    int SHARED_VAR=0;
    if ( Add_VariableToSharingList(vsh,"SHARED_VAR",7,&SHARED_VAR,sizeof(SHARED_VAR)) == 0 )
     {
      fprintf(stderr,"Master : Error Adding Shared Variable\n");
      return 1;
     }

    SHARED_VAR=666; // This will propagate to the client

    printf("Master : Starting Self Test waiting for value 1 from peer !\n");
    if (!wait_for_var_to_become_x(&SHARED_VAR,WAIT_TIME,1)) { fprintf(stderr,"Master : Failed the test\n"); return 1; }

    SHARED_VAR=2;
    printf("Master : Now we have changed the variable to 2 , will wait until it becomes 3\n");
    if (!wait_for_var_to_become_x(&SHARED_VAR,WAIT_TIME,3)) { fprintf(stderr,"Master : Failed the test\n"); return 1; }

    SHARED_VAR=4;
    printf("Master : Now we have changed the variable to 4 , will wait until it becomes 5\n");
    if (!wait_for_var_to_become_x(&SHARED_VAR,WAIT_TIME,5)) { fprintf(stderr,"Master : Failed the test\n"); return 1; }

    SHARED_VAR=6;


    fprintf(stderr,"Master : Closing things down \n");
    if (!Stop_VariableSharing(vsh))
     {
         fprintf(stderr,"Master : Error Stopping Variable share\n");
         return 1;
     }

    return 0;
}
