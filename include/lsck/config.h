/*
 *  libsocket - BSD socket like library for DJGPP
 *  Copyright 1997, 1998 by Indrek Mandre
 *  Copyright 1997, 1998 by Richard Dawe
 *
 *  This library is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU Library General Public License as published
 *  by the Free Software Foundation; either version 2 of the License, or (at
 *  your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 * 
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
    config.h - libsocket configuration functions
*/

#ifndef __libsocket_config_h__
#define __libsocket_config_h__

/* Use these two functions in next release? */
/*char *ls_config_getentry (char *filename, char *entry);
int   lsck_config_setentry (char *filename, char *entry, char *entryvalue);*/

char *lsck_config_getdir   (void);
char *lsck_config_setdir   (char *newdir);

#endif  /* __libsocket_config_h__ */
