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

#include "ProtocolThreads.h"

/*

    HERE IS THE SPACE WHERE THE SERVER THREAD AND CLIENT THREAD OF EACH VARIABLE SHARE WILL
    BE WRITTEN


*/


/*
   The server thread of each Variable Share should wait for network connections and when the network connection is present , it should
   analyze the message received , and according to the state respond with a correct response

*/


/*
   The client thread should do 3 things!

   #1 Check the state of the variables shared from our share ( master variables ) and for each one that needs refreshing in a peer share
   add a job that describes the operation

   #2 Keep a TCP connection to the master remote variable in case our node is a clone , and if the connection is broken , re-establish it

   #3 Carry out the jobs mentioned above
*/



/*

  ALL THE SOCKET STUFF SHOULD BE WRITTEN IN NETWORK FRAMEWORK

  ALL THE MESSAGES SENT/RECEVEIED SHOULD BE PROCESSED HERE

*/
