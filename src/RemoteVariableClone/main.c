#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../RemoteVariableSupport.h"




int wait_for_var_to_become_x(int * var , unsigned int timeout , unsigned int x)
{
  unsigned int time_waited=0;
  while ( (*var!= x )&&(time_waited<timeout) ) { usleep(1000); ++time_waited; }
  if (*var==0) { fprintf(stderr,"Client : The experiment was a complete failure\n");  return 0;}

  return 1;
}




int main()
{

    printf("REMOTE VARIABLES CLIENT !!!!!!!!!!!!! \n");
    printf("Hello world I Will try to Connect to Shared Variables now :) \n");

    struct VariableShare * vsh = ConnectToRemote_VariableSharing("127.0.0.1",12345,"password");
    if ( vsh == 0 )  fprintf(stderr,"Client : Error Creating share");

    int SHARED_VAR=0;
    if ( ! Add_VariableToSharingList(vsh,"SHARED_VAR",7,&SHARED_VAR,sizeof(SHARED_VAR)) )
      {
        fprintf(stderr,"Client : Error Adding Shared Variable , cannot continue\n");
        return 1;
      }


    printf("Client : Starting Self Test , Waiting to get the 666 initial value!\n");
    wait_for_var_to_become_x(&SHARED_VAR,10000,666);


    SHARED_VAR=1;
    printf("Client : Now we have changed the variable to 1 , will wait until it becomes 2\n");
    wait_for_var_to_become_x(&SHARED_VAR,10000,2);

    SHARED_VAR=3;
    printf("Client : Now we have changed the variable to 3 , will wait until it becomes 4\n");
    wait_for_var_to_become_x(&SHARED_VAR,10000,4);

    SHARED_VAR=5;
    printf("Client : Now we have changed the variable to 5 , will wait until it becomes 6\n");
    wait_for_var_to_become_x(&SHARED_VAR,10000,6);



     if ( Stop_VariableSharing(vsh) == 0 ) fprintf(stderr,"Client : Error Deleting share\n");
    return 0;
}
