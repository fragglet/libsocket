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
#include <unistd.h>

#include <dpmi.h>
#include <sys/farptr.h>
#include <sys/segments.h>

#include <sys/socket.h>

#include <lsck/if.h>
#include "wsock.h"
#include "wsockvxd.h"
#include "farptrx.h"

/* -------------------
 * - __wsock_connect -
 * ------------------- */

int __wsock_connect (LSCK_SOCKET *lsd,
                     struct sockaddr *serv_addr, size_t addrlen)
{
	LSCK_SOCKET_WSOCK *wsock = (LSCK_SOCKET_WSOCK *) lsd->idata;
	WSOCK_CONNECT_PARAMS params;
	int blocking = lsd->blocking;
	int flag = lsd->blocking;
	int rv;

	bzero (&params, sizeof (params));

	params.Address = (void *) (SocketP << 16) + (5 * 4);
	params.Socket = (void *) wsock->_Socket;
	params.AddressLength = addrlen;
	params.ApcRoutine = 0;
	params.ApcContext = 0;

	/* RD: This shouldn't be used this way! */
	/*if (lsd->blocking)
	 * if (__wsock_select_wait (lsd, FD_CONNECT) == -1) return(-1); */

	/* RD: Flip to non-blocking mode if connecting, so that Ctrl+C can
	 * break out of the program. BTW __wsock_ioctl stamps on
	 * lsd->blocking, hence the use of blocking rather than
	 * lsd->blocking. */
	if (blocking)
		__wsock_ioctl (lsd, &rv, FIONBIO, &flag);

	_farpokex (SocketP, 0, &params, sizeof (WSOCK_CONNECT_PARAMS));
	_farpokex (SocketP, 5 * 4, serv_addr, addrlen);

	__wsock_callvxd (WSOCK_CONNECT_CMD);

	/* RD: If the socket was in blocking mode, then we need to catch the
	 * failure of the non-blocking connect, wait for a connection and then
	 * flip back to blocking mode. */
	if (blocking) {
		/* For some reason WSOCK.VXD returns WSAEWOULDBLOCK rather than
		 * WSAEINPROGRESS, so we have to detect both mapped errors from
		 * errno here. */
		if (_VXDError
		    && (errno != EWOULDBLOCK)
		    && (errno != EINPROGRESS)) {
			/* Flip back to blocking mode */
			flag = !flag;
			__wsock_ioctl (lsd, &rv, FIONBIO, &flag);
			return (-1);
		}
		
		/* Wait for writability */
		__wsock_select_wait (lsd, FD_WRITE);

		/* Flip back to blocking mode */
		flag = !flag;
		__wsock_ioctl (lsd, &rv, FIONBIO, &flag);
	} else {
		if (_VXDError && _VXDError != 0xffff) return -1;
	}

	_farpeekx (SocketP, 5 * 4, serv_addr, addrlen);
	memcpy (&wsock->inetaddr, serv_addr, addrlen);

	/* RD: This is no longer needed */
	/* Now new attitude, it seems to work.. */
	/*if (lsd->blocking)
	 * if (__wsock_select_wait(lsd, FD_WRITE ) == -1) return -1; */

	return 0;
}
