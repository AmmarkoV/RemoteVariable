#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../RemoteVariableSupport.h"

int main()
{
    printf("REMOTE VARIABLES MASTER !!!!!!!!!!!!! \n");
    printf("Hello world I Will try to test SHARED_VARS!\n");

    struct VariableShare * vsh = Start_VariableSharing("SHARE2","0.0.0.0",12345,"password");
    if ( vsh == 0 )
     {
         fprintf(stderr,"Error Creating share");
         return 1;
     }

    int SHARED_VAR=666;
    if ( Add_VariableToSharingList(vsh,"SHARED_VAR",7,&SHARED_VAR,sizeof(SHARED_VAR)) == 0 ) fprintf(stderr,"Error Adding Shared Variable");

    printf("Starting Self Test!\n");
    while (SHARED_VAR != 1 )
     {
       usleep(1000);
       SHARED_VAR = SHARED_VAR - 1;
     }
    printf("Ended Self Test ,if nothing printed since start test failed :P !\n");
    SHARED_VAR=666;

    while (SHARED_VAR != 1 )
     {
       usleep(100);
     }
    printf("Received 1 \n");


    while (SHARED_VAR != 2 )
     {
       usleep(100);
     }
    printf("Received 2 \n");

     while (SHARED_VAR != 12345 )
     {
       usleep(100);
     }
    printf("Received 3 \n");

    if ( Stop_VariableSharing(vsh) == 0 ) fprintf(stderr,"Error Deleting share\n");
    return 0;
}
