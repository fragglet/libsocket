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

#include <string.h>

#include <sys/socket.h>
#include <sys/un.h>

#include <lsck/lsck.h>
#include <lsck/if.h>
#include "unix.h"

/* ----------------------
 * - __unix_getpeername -
 * ---------------------- */

int __unix_getpeername (LSCK_SOCKET *lsd,
                        struct sockaddr *name, size_t *namelen)
{
	LSCK_SOCKET_UNIX *usock = (LSCK_SOCKET_UNIX *) lsd->idata;
	struct sockaddr_un sun;
	int copylen = *namelen < sizeof (struct sockaddr_un)
		    ? *namelen : sizeof (struct sockaddr_un);

	/* Build the peer address up from the I-layer data. */
	sun.sun_family = AF_UNIX;
	strcpy(sun.sun_path, usock->peerpath);
	
	memcpy (name, &sun, copylen);
	*namelen = copylen;
	return (0);
}
