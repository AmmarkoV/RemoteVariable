#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../RemoteVariableSupport.h"




int wait_for_var_to_become_x(int * var , unsigned int timeout , unsigned int x)
{
  unsigned int time_waited=0;
  while ( (*var!= x )&&(time_waited<timeout) ) { usleep(1000); if (time_waited%10==0) { printf(".m."); } ++time_waited; }
  if (*var==x) {   return 1;}

  return 0;
}




int main()
{
    unsigned int WAIT_TIME=100;
    printf("REMOTE VARIABLES CLIENT STARTUP !!!!!!!!!!!!! \n");
    struct VariableShare * vsh = ConnectToRemote_VariableSharing("SHARE2","127.0.0.1",12345,"password");
    if ( vsh == 0 )
      {
        fprintf(stderr,"Client : Error Creating share");
        return 1;
      }



    int SHARED_VAR=0;
    if ( ! Add_VariableToSharingList(vsh,"SHARED_VAR",7,&SHARED_VAR,sizeof(SHARED_VAR)) )
      {
        fprintf(stderr,"Client : Error Adding Shared Variable , cannot continue\n");
        return 1;
      }


    printf("Client : Starting Self Test , Waiting to get the 666 initial value!\n");
    if ( !wait_for_var_to_become_x(&SHARED_VAR,WAIT_TIME,666)) { fprintf(stderr,"Client : Failed the test\n"); return 1; }


    SHARED_VAR=1;
    printf("Client : Now we have changed the variable to 1 , will wait until it becomes 2\n");
    if ( !wait_for_var_to_become_x(&SHARED_VAR,WAIT_TIME,2)) { fprintf(stderr,"Client : Failed the test\n"); return 1; }

    SHARED_VAR=3;
    printf("Client : Now we have changed the variable to 3 , will wait until it becomes 4\n");
    if ( !wait_for_var_to_become_x(&SHARED_VAR,WAIT_TIME,4)) { fprintf(stderr,"Client : Failed the test\n"); return 1; }

    SHARED_VAR=5;
    printf("Client : Now we have changed the variable to 5 , will wait until it becomes 6\n");
    if ( !wait_for_var_to_become_x(&SHARED_VAR,WAIT_TIME,6)) { fprintf(stderr,"Client : Failed the test\n"); return 1; }



     if ( Stop_VariableSharing(vsh) == 0 ) fprintf(stderr,"Client : Error Deleting share\n");
    return 0;
}
