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
#include "NetworkFramework.h"
#include "VariableDatabase.h"
#include "Protocol.h"
#include "helper.h"


int AddJob(struct VariableShare * vsh,unsigned int our_varid,unsigned int sockpeer , char operation_type)
{
 if (  vsh->jobs_loaded < RVS_MAX_JOBS_PENDING )
  { // NEEDS TO BE REWRITTEN TO KEEP A SORTED LIST!
    unsigned int where_to_add=vsh->jobs_loaded;
    unsigned int peer_id = GetPeerIdBySock(vsh,sockpeer);
    if (peer_id==0) { fprintf(stderr,"Could not find a peer for AddJob operation!\n"); return 0; }
    --peer_id;

    vsh->job_list[where_to_add].local_var_id=our_varid;
    vsh->job_list[where_to_add].remote_peer_socket=sockpeer;
    vsh->job_list[where_to_add].remote_peer_id=peer_id;
    vsh->job_list[where_to_add].action=operation_type;
    vsh->job_list[where_to_add].time=vsh->central_timer;

    fprintf(stderr,"Added JOB %u , our variable %u must be shared with peer %u - ",where_to_add,our_varid,peer_id);
    if (operation_type==READFROM)       fprintf(stderr," READFROM "); else
    if (operation_type==WRITETO)        fprintf(stderr," WRITETO "); else
    if (operation_type==SIGNALCHANGED)  fprintf(stderr," SIGNALCHANGED "); else
    if (operation_type==SYNC)           fprintf(stderr," SYNC "); else
    if (operation_type==NOACTION)       fprintf(stderr," NOACTION ");


    ++vsh->jobs_loaded;
    fprintf(stderr,"Total jobs are now %u\n",vsh->jobs_loaded);

    return 1;
  } else
  {
     fprintf(stderr,"Job queue is full discarding new request!!\n");
  }
 return 0;
}

int SwapJobs(struct VariableShare * vsh,int job_id1,int job_id2)
{
 if ( (job_id1<vsh->jobs_loaded)&&(job_id2<vsh->jobs_loaded) )
  {
    struct ShareJob temp = vsh->job_list[job_id1];
    vsh->job_list[job_id1] = vsh->job_list[job_id2];
    vsh->job_list[job_id2] = temp;
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
  unsigned int sent_to=0;
  fprintf(stderr,"Job_SingalLocalVariableChanged broadcasting to all peers ( %u ) \n",vsh->total_peers);
  if (vsh->total_peers==0) { fprintf(stderr,"No peers to signal to..\n"); return 0; }
   unsigned int i=0;
   for (i=0; i< vsh->total_peers; i++)
    {
      fprintf(stderr,"Job_SingalLocalVariableChanged broadcasting to peer number %u \n",i);
      sent_to+=AddJob(vsh,our_varid,vsh->peer_list[i].socket_to_client,SIGNALCHANGED);
    }

  return sent_to;
}

int ExecuteJob(struct VariableShare *vsh, unsigned int job_id)
{
   if (job_id>=vsh->jobs_loaded) { error("Job Execute call on jobid out of bounds\n"); return 0; }

   unsigned int peer = vsh->job_list[job_id].remote_peer_id;
  // fprintf(stderr,"ExecuteJob job for peer %u \n",peer);

   unsigned int var_id = vsh->job_list[job_id].local_var_id;
   char * variable_name = vsh->share.variables[var_id].ptr_name;
   unsigned int peer_socket =  vsh->job_list[job_id].remote_peer_socket;


   switch ( vsh->job_list[job_id].action )
   {
      case NOACTION :
                       fprintf(stderr,"No Action came up for execution:P \n");
                       DoneWithJob(vsh,job_id);
                       break;
      case WRITETO  :
                       fprintf(stderr,"Execution of Write to peer : %u of variable %s with var id %u \n",peer,variable_name,var_id);
                       AutoRefreshVariable_Thread_Pause(vsh);


                       if ( Request_WriteVariable(vsh,peer,var_id,peer_socket) )
                        {
                            DoneWithJob(vsh,job_id);
                        }  else
                        {
                            fprintf(stderr,"Request of writing variable %u failed \n",var_id);
                        }

                       AutoRefreshVariable_Thread_Resume(vsh);
                       break;
      case READFROM :
                       fprintf(stderr,"Execution of Read from peer : %u of variable %s with var id %u \n",peer,variable_name,var_id);
                       AutoRefreshVariable_Thread_Pause(vsh);


                       if ( Request_ReadVariable(vsh,peer,var_id,peer_socket) )
                        {
                            DoneWithJob(vsh,job_id);
                        }  else
                        {
                            fprintf(stderr,"Request of variable %u failed \n",var_id);
                        }

                       AutoRefreshVariable_Thread_Resume(vsh);
                       break;


      case SIGNALCHANGED :
                           fprintf(stderr,"Execution of Singal Changed to peer : %u of variable %s with var id %u \n",peer,variable_name,var_id);
                           AutoRefreshVariable_Thread_Pause(vsh);

                            if ( Request_SignalChangeVariable(vsh,peer,var_id,peer_socket) )
                             {
                                DoneWithJob(vsh,job_id);
                             } else
                             {
                                fprintf(stderr,"Could not signal change\n");
                             }

                           AutoRefreshVariable_Thread_Resume(vsh);
                           break;
      case SYNC :
                     fprintf(stderr,"Simulating Execution of Sync Operation : %u of variable %s with var id %u \n",peer,variable_name,var_id);
                     DoneWithJob(vsh,job_id);
                    break;
      default :
        fprintf(stderr,"Unhandled job type\n");
        DoneWithJob(vsh,job_id);
        return 0;
      break;
   };


   return 1;
}

int ExecutePendingJobsForPeerID(struct VariableShare *vsh,unsigned int peer_id)
{
   if (!vsh->jobs_loaded)  { /*Nothing to do*/ return 0; }
   //fprintf(stderr,"ExecutePendingJobsForPeerID , id = %u \n",peer_id);

   unsigned int successfull_jobs=0;
   unsigned int i=0;
   while (i<vsh->jobs_loaded)
    {
      if (peer_id==vsh->job_list[i].remote_peer_id)
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
   if (!vsh->jobs_loaded)  { /*Nothing to do*/ return 0; }
   //fprintf(stderr,"ExecutePendingJobs  \n");

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


