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

int AddJob(struct VariableShare * vsh,unsigned int our_varid,unsigned int peer_id , char operation_type)
{
 if (  vsh->jobs_loaded < RVS_MAX_JOBS_PENDING )
  { // NEEDS TO BE REWRITTEN TO KEEP A SORTED LIST!
    unsigned int where_to_add=vsh->jobs_loaded;
    vsh->job_list[where_to_add].our_sharelist_id=our_varid;
    vsh->job_list[where_to_add].remote_peer_id=peer_id;
    vsh->job_list[where_to_add].action=operation_type;
    vsh->job_list[where_to_add].time=central_timer;
    fprintf(stderr,"Added JOB %u , our variable %u must be shared with peer %u (%s)  - type %u @ %u\n",where_to_add,our_varid,peer_id,vsh->peer_list[peer_id].IP,operation_type,central_timer);
    ++vsh->jobs_loaded;
    return 1;
  } else
   fprintf(stderr,"Job queue is full discarding new request!!\n");
 return 0;
}


int SwapJobs(struct VariableShare * vsh,int job_id1,int job_id2)
{
 if ( (job_id1<vsh->jobs_loaded)&&(job_id2<vsh->jobs_loaded) )
  {
    struct ShareJob temp=vsh->job_list[job_id1];
    vsh->job_list[job_id1]=vsh->job_list[job_id2];
    vsh->job_list[job_id2]=temp;
  } else
  {
    fprintf(stderr,"SwapJobs called for jobs %u and %u which are out of bounds ( %u ) \n",job_id1,job_id2,vsh->jobs_loaded);
    return 0;
  }
 return 1;
}


int RemJob(struct VariableShare * vsh,int job_id)
{
 if ( (vsh->jobs_loaded==1)&&(job_id==0)) { vsh->jobs_loaded=0; vsh->job_list[0].action=NOACTION; return 1; }
 if ( (vsh->jobs_loaded>1) )
  {
    unsigned int last_job = vsh->jobs_loaded-1;
    SwapJobs(vsh,job_id,last_job);
    vsh->job_list[last_job].action=NOACTION;
    --vsh->jobs_loaded;
    return 1;
  }



 return 0;
}

int DoneWithJob(struct VariableShare * vsh,int job_id)
{
 if ( job_id < vsh->jobs_loaded )
  {
     vsh->job_list[job_id].action=NOACTION;
  }
 return 0;
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

  return 0;
}


int Job_UpdateLocalVariable(struct VariableShare * vsh,unsigned int our_varid,unsigned int peer_varid)
{
 return AddJob(vsh,our_varid,peer_varid,READFROM);
}

int Job_UpdateRemoteVariable(struct VariableShare * vsh,unsigned int our_varid,unsigned int peer_varid)
{
 return AddJob(vsh,our_varid,peer_varid,WRITETO);
}

int Job_SingalLocalVariableChanged(struct VariableShare * vsh,unsigned int our_varid)
{
 return AddJob(vsh,our_varid,0,SIGNALCHANGED);
}


int ExecuteJob(struct VariableShare *vsh, unsigned int job_id)
{
   if (job_id>=vsh->jobs_loaded) { error("Job Execute call on jobid out of bounds\n"); return 0; }

   switch ( vsh->job_list[job_id].action )
   {
      case NOACTION : break;
      case WRITETO : break;
      case READFROM : break;
      case SIGNALCHANGED : break;
      case SYNC : break;
      default :

        fprintf(stderr,"Unhandled job type\n");
      break;
   };


   return 1;
}



int ExecutePendingJobsForClient(struct VariableShare *vsh,unsigned int client_id)
{
   unsigned int successfull_jobs=0;
   unsigned int i=0;
   while (i<vsh->jobs_loaded)
    {
      if (client_id)
      {
       if (ExecuteJob(vsh,i)) { ++successfull_jobs; } else
                              { fprintf(stderr,"Job %u on Share %s failed \n"); }

      }
       ++i;
    }
  return successfull_jobs;
}



int ExecutePendingJobs(struct VariableShare *vsh)
{
   unsigned int successfull_jobs=0;
   unsigned int i=0;
   while (i<vsh->jobs_loaded)
    {
       if (ExecuteJob(vsh,i)) { ++successfull_jobs; } else
                              { fprintf(stderr,"Job %u on Share %s failed \n"); }
       ++i;
    }
  return successfull_jobs;
}
