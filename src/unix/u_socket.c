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

#include <stdlib.h>
#include <string.h>

#include <lsck/if.h>
#include "unix.h"

/* -----------------
 * - __unix_socket -
 * ----------------- */

/* Unix domain socket creation occurs when a name is assigned, using either
 * __unix_bind() or __unix_connect() (which calls __unix_bind() anyhow). */

int __unix_socket (LSCK_SOCKET * lsd)
{
	LSCK_SOCKET_UNIX *usock = NULL;

	/* Create the interface data. */
	usock = (LSCK_SOCKET_UNIX *) malloc(sizeof(*usock));
	lsd->idata = (void *) usock;

	if (usock == NULL) {
		errno = ENOMEM;
		return(-1);
	}
	
	/* Set these for safefty - invalid file descriptors */
	bzero(usock, sizeof(*usock));
	usock->fd_w = usock->fd_r = -1;

	return (0);
}
