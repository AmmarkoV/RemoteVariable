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

#ifndef HELPER_H_INCLUDED
#define HELPER_H_INCLUDED

void under_construction_msg();
void error(char * msg);
int mutex_msg();
int protocol_msg();
int sockadap_msg();
int debug_msg();
void debug_say(char * msg);
void debug_say_nocr(char * msg);
void remove_ending_nl(char * str);

#endif // HELPER_H_INCLUDED
