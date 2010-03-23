#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../RemoteVariableSupport.h"

int main()
{
    printf("Hello world I Will try to test SHARED_VARS!\n");

    struct VariableShare * vsh = Start_VariableSharing("SHARE1","password");
    if ( vsh == 0 )  fprintf(stderr,"Error Creating share");

    int SHARED_VAR=666;
    if ( Add_VariableToSharingList(vsh,"SHARED_VAR",7,&SHARED_VAR,sizeof(SHARED_VAR)) == 0 ) fprintf(stderr,"Error Adding Shared Variable");

    while (SHARED_VAR != 12345 )
     {

       usleep(100);
     }


    if ( Stop_VariableSharing(vsh) == 0 ) fprintf(stderr,"Error Deleting share\n");
    return 0;
}
