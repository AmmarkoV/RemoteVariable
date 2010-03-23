#include <stdio.h>
#include <stdlib.h>
#include "../RemoteVariableSupport.h"

int main()
{
    printf("Hello world!\n");

    struct VariableShare * vsh = Start_VariableSharing("SHARE1","password");
    if ( vsh == 0 )  fprintf(stderr,"Error Creating share");

    if ( Stop_VariableSharing(vsh) == 0 ) fprintf(stderr,"Error Deleting share");
    return 0;
}
