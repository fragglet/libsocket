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

/* ----------------
 * - __wsock_send -
 * ---------------- */

ssize_t __wsock_send (LSCK_SOCKET * lsd, void *msg, size_t len,
                      unsigned int flags)
{
	LSCK_SOCKET_WSOCK *wsock = (LSCK_SOCKET_WSOCK *) lsd->idata;
	WSOCK_SEND_PARAMS params;

	/* RD: Check the length of the data - only transmit up to the size of
	 * the buffer */
	if (len > _SocketD.size) len = _SocketD.size;

	bzero (&params, sizeof (params));

	params.Buffer = (void *) (SocketD << 16);
	params.Address = NULL;
	params.Socket = (void *) wsock->_Socket;
	params.BufferLength = len;
	params.Flags = flags;
	params.AddressLength = 0;
	params.BytesSent = 0;
	params.ApcRoutine = NULL;
	params.ApcContext = 0;
	params.Timeout = 0;

	_farpokex (SocketP, 0, &params, sizeof (WSOCK_SEND_PARAMS));
	_farpokex (SocketD, 0, msg, len);

	__wsock_callvxd (WSOCK_SEND_CMD);

	if (_VXDError && _VXDError != 0xffff) return -1;

	_farpeekx (SocketP, 0, &params, sizeof (WSOCK_SEND_PARAMS));

	return params.BytesSent;
}

/* ------------------
 * - __wsock_sendto -
 * ------------------ */

ssize_t __wsock_sendto (LSCK_SOCKET * lsd, void *msg, size_t len,
                        unsigned int flags,
		        struct sockaddr *to, size_t tolen)
{
	LSCK_SOCKET_WSOCK *wsock = (LSCK_SOCKET_WSOCK *) lsd->idata;
	WSOCK_SEND_PARAMS params;

	/* RD: Check the length of the data - only transmit up to the size of
	 * the buffer */
	if (len > _SocketD.size) len = _SocketD.size;

	bzero (&params, sizeof (params));

	params.Buffer = (void *) (SocketD << 16);
	params.Address = (void *) (SocketP << 16) + (10 * 4);
	params.Socket = (void *) wsock->_Socket;
	params.BufferLength = len;
	params.Flags = flags;
	params.AddressLength = tolen;
	params.BytesSent = 0;
	params.ApcRoutine = NULL;
	params.ApcContext = 0;
	params.Timeout = 0;

	_farpokex (SocketP, 0, &params, sizeof (WSOCK_SEND_PARAMS));
	_farpokex (SocketP, 10 * 4, to, tolen);
	_farpokex (SocketD, 0, msg, len);

	__wsock_callvxd (WSOCK_SEND_CMD);

	if (_VXDError && _VXDError != 0xffff) return -1;

	_farpeekx (SocketP, 0, &params, sizeof (WSOCK_SEND_PARAMS));

	return params.BytesSent;
}
