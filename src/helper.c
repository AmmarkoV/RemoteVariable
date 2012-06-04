/***************************************************************************
* Copyright (C) 2010 by Ammar Qammaz *
* ammarkov@gmail.com *
* *
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or *
* (at your option) any later version. *
* *
* This program is distributed in the hope that it will be useful, *
* but WITHOUT ANY WARRANTY; without even the implied warranty of *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the *
* GNU General Public License for more details. *
* *
* You should have received a copy of the GNU General Public License *
* along with this program; if not, write to the *
* Free Software Foundation, Inc., *
* 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. *
***************************************************************************/

#include "helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "version.h"

void under_construction_msg()
{
   fprintf(stderr,"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
   fprintf(stderr," THIS VERSION OF libRemoteVariableSupport.a doesnt work \n");
   fprintf(stderr,"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");



   fprintf(stderr,"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
   fprintf(stderr,"RemoteVariables Version %s build %u ,  %s / %s / %s \n",FULLVERSION_STRING,(unsigned int) BUILDS_COUNT,DATE,MONTH,YEAR);

   fprintf(stderr,"!!PLEASE NOTE!! that this version of RemoteVariables is still in development\n");
   fprintf(stderr,"It isn`t fit for use on a stable project..\nIf you are a developer you can help out! :) \n");
   fprintf(stderr,"When the project will be mature enough this warning message will be removed!\n");
   fprintf(stderr,"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");

}


void error(char * msg)
{
 fprintf(stderr,"ERROR MESSAGE : %s\n",msg);
 return;
}

int protocol_msg()
{
 return 0;
}

int sockadap_msg()
{
 return 0;
}
int debug_msg()
{
 return 1;
}

void debug_say(char * msg)
{
 if ( debug_msg() == 1 ) fprintf(stderr,"%s\n",msg);
 return;
}

void debug_say_nocr(char * msg)
{
 if ( debug_msg() == 1 ) fprintf(stderr,"%s",msg);
 return;
}

void remove_ending_nl(char * str)
{
  int length = strlen(str);
  if ( length == 0 ) { return ; }

  while ( (str[length]==10) || (str[length]==13) )
   {
       str[length]=0;
       --length;
       if (length == 0) { break; }
   }

  if ( (str[0]==10) || (str[0]==13) ) { str[0]=0; }
}

