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
#include <io.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

#include <dpmi.h>
#include <sys/farptr.h>
#include <sys/segments.h>

#include <sys/socket.h>

#include <lsck/if.h>
#include "wsock.h"
#include "wsockvxd.h"
#include "farptrx.h"

/* ------------------
 * - __wsock_accept -
 * ------------------ */

int __wsock_accept (LSCK_SOCKET * lsd, LSCK_SOCKET * nsd,
		    struct sockaddr *addr, size_t *addrlen)
{
	LSCK_SOCKET_WSOCK *wsock = (LSCK_SOCKET_WSOCK *) lsd->idata;
	LSCK_SOCKET_WSOCK *nwsock = NULL;
	WSOCK_ACCEPT_PARAMS params;
	time_t now;

	/* Create memory the new socket's parameters. */
	nwsock = (LSCK_SOCKET_WSOCK *) malloc(sizeof(*nwsock));
	nsd->idata = (void *) nwsock;
	
	if (nwsock == NULL) {
		errno = ENOMEM;
		return(-1);
	}
	
	/* Set up the parameters for the accept. */
	time (&now);
	bzero (&params, sizeof (params));

	params.Address = (void *) ((SocketP << 16) + (7 * 4));
	params.ListeningSocket = (void *) wsock->_Socket;
	params.ConnectedSocket = NULL;
	params.AddressLength = *addrlen;
	params.ConnectedSocketHandle = (int) now;
	params.ApcRoutine = 0;
	params.ApcContext = 0;

	if (lsd->blocking)
		/* RD: FD_READ instead of FD_ACCEPT doesn't work here. */
		if (__wsock_select_wait (lsd, FD_ACCEPT) == -1)
			return (-1);

	_farpokex (SocketP, 0, &params, sizeof (WSOCK_ACCEPT_PARAMS));

	__wsock_callvxd (WSOCK_ACCEPT_CMD);

	if (_VXDError && _VXDError != 0xffff)
		return (-1);

	_farpeekx (SocketP, 0, &params, sizeof (WSOCK_ACCEPT_PARAMS));

	if (params.ConnectedSocket == NULL) return (-1);

	*addrlen = params.AddressLength;
	_farpeekx (SocketP, 7 * 4, addr, *addrlen);

	nwsock->_Socket = (int) params.ConnectedSocket;
	nwsock->_SocketHandle = params.ConnectedSocketHandle;
	memcpy (&nwsock->inetaddr, addr, *addrlen);

	return (0);
}
