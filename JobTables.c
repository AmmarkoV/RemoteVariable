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
#include <stdio.h>
#include <stdlib.h>
#include "JobTables.h"
#include "RemoteVariableSupport.h"
#include "helper.h"

int AddJob(struct VariableShare * vsh,unsigned int our_varid,unsigned int peer_varid , char operation_type)
{
 if (  vsh->jobs_loaded < MAX_JOBS_PENDING )
  { // NEEDS TO BE REWRITTEN TO KEEP A SORTED LIST!
    unsigned int where_to_add=vsh->jobs_loaded;
    vsh->job_list[where_to_add].our_cache_id=our_varid;
    vsh->job_list[where_to_add].remote_cache_id=peer_varid;
    vsh->job_list[where_to_add].action=operation_type;
    vsh->job_list[where_to_add].time=central_timer;
    printf("Added JOB %u , %u -> %u  - type %u @ %u\n",where_to_add,our_varid,peer_varid,operation_type,central_timer);
    ++vsh->jobs_loaded;
    return 1;
  } else
   printf("Job queue is full discarding new request!!\n");
 return -1;
}

int RemJob(struct VariableShare * vsh,int job_id)
{
 debug_say("REMove Job not implemented \n");
 return -1;
}

int DoneWithJob(struct VariableShare * vsh,int job_id)
{
 if ( job_id < vsh->jobs_loaded )
  {
     vsh->job_list[job_id].action=NOACTION;
  }
 return -1;
}

int GetNextJobIDOperation(struct VariableShare * vsh,char operation_type)
{
  if (operation_type==NOACTION) { return -1; /*No sense in returning the next null job :P */ }

  unsigned int total_jobs=vsh->jobs_loaded;
  unsigned int i=0;
   if ( total_jobs > 0 )
    {
        for ( i=0; i<total_jobs; i++)
         {
              if (vsh->job_list[i].action == operation_type)
                 {
                      return i;
                 }
         }
    }

  return -1;
}


int Job_UpdateLocalVariable(struct VariableShare * vsh,unsigned int our_varid,unsigned int peer_varid)
{
 return AddJob(vsh,our_varid,peer_varid,READFROM);
}

int Job_UpdateRemoteVariable(struct VariableShare * vsh,unsigned int our_varid,unsigned int peer_varid)
{
 return AddJob(vsh,our_varid,peer_varid,WRITETO);
}

int Job_UpdateLocalVariableToAllPeers(struct VariableShare * vsh,unsigned int our_varid)
{
 debug_say("TODO : Add jobs for each client that has registered itself as a client to our cache..");

 return 0;
}
