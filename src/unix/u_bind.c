/*
 *  libsocket - BSD socket like library for DJGPP
 *  Copyright 1997, 1998 by Indrek Mandre
 *  Copyright 1997-2000 by Richard Dawe
 *
 *  Portions of libsocket Copyright 1985-1993 Regents of the University of 
 *  California.
 *  Portions of libsocket Copyright 1991, 1992 Free Software Foundation, Inc.
 *  Portions of libsocket Copyright 1997, 1998 by the Regdos Group.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dir.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <sys/un.h>

#include <lsck/if.h>
#include "unix.h"

/* ---------------
 * - __unix_bind -
 * --------------- */

int __unix_bind (LSCK_SOCKET *lsd, struct sockaddr *my_addr, size_t addrlen)
{
	LSCK_SOCKET_UNIX *usock = (LSCK_SOCKET_UNIX *) lsd->idata;
	struct sockaddr_un *sun = (struct sockaddr_un *) my_addr;
	char path[MAXPATH], buf[MAXPATH];
	int abs_path; /* Path absolute => 1, path relative => 0 */
	int ret;

	/* Check the parameters */
	if (addrlen < sizeof (struct sockaddr_un)) {
		errno = EINVAL;
		return (-1);
	}

	if (addrlen > sizeof (struct sockaddr_un)) {
		errno = ENAMETOOLONG;
		return (-1);
	}

	/* Build the local mailslot name */

	/* TODO: Should this also cope with DOS filenames with [A-Z]: format?
	 * Unix domain sockets currently have a separate namespace from files,
	 * so currently this is not an issue. All Unix-ported programs should
	 * not have a drive letter in the address. */
	abs_path = sun->sun_path[0] == '/';

	/* TODO: Prevent possible buffer overflow */
	strcpy(path, UNIX_LOCAL_PATH);
	if (!abs_path) {
		if (getcwd(buf, sizeof(buf)) == NULL) {
			/* TODO: Appropriate error? Unable to get working
			 * dir -> not able to access component of path. */
			errno = EACCES;
			return(-1);
		}
		strcat(path, buf + 2); /* Skip [A-Z]: drive spec */
		strcat(path, "/");
	}
	strcat(path, sun->sun_path);

	/* Create local mailslot for reading */
	ret = usock->fd_r = open (path, O_BINARY | O_RDONLY, S_IRUSR);
	
	/* If the "bind" was successful, store the path name for later. */
	if (ret != -1)
		strcpy(usock->sockpath, sun->sun_path);
	else
		bzero(usock->sockpath, sizeof(usock->sockpath));
    
	return ((ret == -1) ? -1 : 0);
}
