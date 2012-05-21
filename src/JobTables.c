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
    vsh->job_list[where_to_add].our_var_id=our_varid;
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


int ClearAllJobs(struct VariableShare * vsh)
{
  fprintf(stderr,"Flushing all jobs\n");
  vsh->jobs_loaded=0;
  return 1;
}


int DoneWithJob(struct VariableShare * vsh,int job_id)
{
 if ( job_id < vsh->jobs_loaded )
  {
     vsh->job_list[job_id].action=NOACTION;
     RemJob(vsh,job_id);
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

   unsigned int peer = vsh->job_list[job_id].remote_peer_id;
   unsigned int var_id = vsh->job_list[job_id].our_var_id;
   char * variable_name = vsh->share.variables[var_id].ptr_name;
   unsigned int peer_socket = vsh->peer_list[peer].socket_to_client;

   switch ( vsh->job_list[job_id].action )
   {
      case NOACTION : break;
      case WRITETO  : fprintf(stderr,"Simulating Execution of Write to peer : %u of variable %s with var id %u \n",peer,variable_name,var_id); DoneWithJob(vsh,job_id);  break;
      case READFROM : fprintf(stderr,"Simulating Execution of Read from peer : %u of variable %s with var id %u \n",peer,variable_name,var_id); DoneWithJob(vsh,job_id);  break;
      case SIGNALCHANGED : fprintf(stderr,"Simulating Execution of Singal Changed to peer : %u of variable %s with var id %u \n",peer,variable_name,var_id);
                            if ( MasterSignalChange_Handshake(vsh,var_id,peer_socket) )
                             {
                                DoneWithJob(vsh,job_id);
                             } else
                             {
                                fprintf(stderr,"Could not signal change\n");
                             }
                            break;
      case SYNC : break;
      default :
        fprintf(stderr,"Unhandled job type\n");
        return 0;
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
      if (client_id==vsh->job_list[i].remote_peer_id)
      {
       if (ExecuteJob(vsh,i)) { ++successfull_jobs; } else
                              { fprintf(stderr,"Job %u on Share %s failed \n",i,vsh->sharename); }

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
                              { fprintf(stderr,"Job %u on Share %s failed \n",i,vsh->sharename); }
       ++i;
    }
  return successfull_jobs;
}




void *
JobExecutioner_Thread(void * ptr)
{
  debug_say("JobExecutioner Thread started..\n");
  struct VariableShare *vsh;
  vsh = (struct VariableShare *) ptr;

   unsigned int total_jobs_done=0 ;
   while ( vsh->stop_job_thread==0 )
   {
     usleep(1000);
     total_jobs_done+=ExecutePendingJobs(vsh);
   }

   return 0;
}

/* THREAD STARTER */
int StartJobExecutioner(struct VariableShare * vsh)
{
  vsh->stop_job_thread=0;
  int retres = pthread_create( &vsh->job_thread, NULL,  JobExecutioner_Thread ,(void*) vsh);
  if (retres!=0) retres = 0; else
                 retres = 1;
  return retres;
}



