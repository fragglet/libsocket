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

/* INFO: Comments starting with 'IM' indicate that Indrek Mandre made changes.
 * Comments starting wiht 'RD' indicate that Richard Dawe made changes. If no
 * comment is given, then the code was written by Indrek Mandre. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/socket.h>

#include <lsck/if.h>
#include "wsock.h"
#include "farptrx.h"
#include "wsockvxd.h"

/* ------------------
 * - __wsock_socket -
 * ------------------ */

int __wsock_socket (LSCK_SOCKET * lsd)
{
	LSCK_SOCKET_WSOCK *wsock = NULL;
	WSOCK_SOCKET_PARAMS params;
	time_t now;
	int rv;
		
	int optval;	/* Option value  */
	size_t optlen;	/* Option length */

	/* Allocate memory to store wsock data. */
	wsock = (LSCK_SOCKET_WSOCK *) malloc(sizeof(*wsock));
	lsd->idata = (void *) wsock;
	
	if (wsock == NULL) {
		errno = ENOMEM;
		return(-1);
	}
	
	/* Set up the parameters for the creation call. */
	time (&now);
	bzero (&params, sizeof (params));
	
	params.AddressFamily = lsd->family;
	params.SocketType    = lsd->type;
	params.Protocol      = lsd->protocol;
	params.NewSocket     = 0;

	/* Indrek Mandre: accoring to Alfons Hoogervorst here must be a unique
	 * value. */	
	params.NewSocketHandle = (long) now;

	_farpokex (SocketP, 0, &params, sizeof (WSOCK_SOCKET_PARAMS));

	__wsock_callvxd (WSOCK_SOCKET_CMD);

	_farpeekx (SocketP, 0, &params, sizeof (WSOCK_SOCKET_PARAMS));

	if (_VXDError && (_VXDError != 0xffff)) return -1;

	wsock->_Socket = (int) params.NewSocket;
	wsock->_SocketHandle = params.NewSocketHandle;

	/* Set some default socket options, if necessary. */
	if (   (lsd->family == AF_INET)
	    && (lsd->type == SOCK_DGRAM)
	    && (lsd->protocol == IPPROTO_UDP)) {
		optval = 1;
		optlen = sizeof(optval);
		__wsock_setsockopt(lsd, &rv,
				   SOL_SOCKET, SO_BROADCAST,
				   &optval, optlen);
	}
	
	return (0);
}
