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

/* Originally by Indrek Mandre (IM), modifications by Richard Dawe (RD) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dpmi.h>
#include <sys/farptr.h>
#include <sys/segments.h>

#include <sys/socket.h>

#include <lsck/if.h>
#include "wsock.h"
#include "farptrx.h"
#include "wsockvxd.h"

/* -----------------
 * - __wsock_close -
 * ----------------- */

int __wsock_close (LSCK_SOCKET * lsd)
{
	LSCK_SOCKET_WSOCK *wsock = (LSCK_SOCKET_WSOCK *) lsd->idata;
	WSOCK_CLOSESOCKET_PARAMS params;

	bzero (&params, sizeof (params));

	params.Socket = (void *) wsock->_Socket;

	_farpokex (SocketP, 0, &params, sizeof (WSOCK_CLOSESOCKET_PARAMS));

	__wsock_callvxd (WSOCK_CLOSESOCKET_CMD);

	if (_VXDError && (_VXDError != 0xFFFF)) return (-1);

	/* RD: Set these to zero now the socket's closed. */
	wsock->_Socket = wsock->_SocketHandle = 0;

	return (0);
}
