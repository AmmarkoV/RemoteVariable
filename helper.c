#include "helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void error(char * msg)
{
 fprintf(stderr,"ERROR MESSAGE : %s\n",msg);
 return;
}
