#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../RemoteVariableSupport.h"

int main()
{

    printf("Hello world I Will try to Connect to Shared Variables now :) \n");

    struct VariableShare * vsh = ConnectToRemote_VariableSharing("127.0.0.1",12345,"password");
    if ( vsh == 0 )  fprintf(stderr,"Error Creating share");

    int SHARED_VAR=12345;
    if ( Add_VariableToSharingList(vsh,"SHARED_VAR",7,&SHARED_VAR,sizeof(SHARED_VAR)) == 0 ) fprintf(stderr,"Error Adding Shared Variable");

    Refresh_RemoteVariable(vsh,"SHARED_VAR");


    if ( Stop_VariableSharing(vsh) == 0 ) fprintf(stderr,"Error Deleting share\n");
    return 0;
}
