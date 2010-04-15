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

#include "JobTables.h"


int AddJob(struct VariableShare * vsh,unsigned int our_varid,unsigned int peer_varid , char operation_type)
{
 if (  vsh->jobs_loaded < MAX_JOBS_PENDING )
  {
    ++vsh->jobs_loaded;
  } else
   printf("Job queue is full discarding new request!!\n");
 return -1;
}

int RemJob(struct VariableShare * vsh,int job_id)
{

 return -1;
}

int GetNextJobOperation(struct VariableShare * vsh,char operation_type)
{
  return -1;
}


int UpdateLocalVariable()
{
 return -1;
}

int UpdateRemoteVariable()
{
 return -1;
}
