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

#include <dpmi.h>
#include <sys/farptr.h>
#include <sys/segments.h>

#include <sys/socket.h>

#include <lsck/if.h>
#include "wsock.h"
#include "farptrx.h"
#include "wsockvxd.h"

/* -----------------
 * - __wsock_ioctl -
 * ----------------- */

int __wsock_ioctl (LSCK_SOCKET *lsd, int *rv, int request, int *param)
{
	LSCK_SOCKET_WSOCK *wsock = (LSCK_SOCKET_WSOCK *) lsd->idata;
	WSOCK_IOCTLSOCKET_PARAMS params;

	/* Check the inbound parameters */
	switch (request) {
	case FIONBIO:
	case FIONREAD:
	case SIOCATMARK:
		break;

		/* These aren't supported! */
		/*case SIOCSHIWAT:
		 * case SIOCGHIWAT:
		 * case SIOCSLOWAT:
		 * case SIOCGLOWAT: */

	/* Unsupported ioctl */
	default:
		/* Unhandled */
		return(0);
	}

	/* Set up the call */
	bzero (&params, sizeof (params));

	params.Socket = (void *) wsock->_Socket;
	params.Command = request;
	params.Param = *param;

	_farpokex (SocketP, 0, &params, sizeof (WSOCK_IOCTLSOCKET_PARAMS));

	__wsock_callvxd (WSOCK_IOCTLSOCKET_CMD);

	if (_VXDError && _VXDError != 0xffff) {
		/* Handled and it failed. */
		*rv = -1;
		return(1);
	}

	_farpeekx (SocketP, 0, &params, sizeof (WSOCK_IOCTLSOCKET_PARAMS));
	*param = params.Param;

	if (request == FIONBIO) {
		if (*param == 0)
			lsd->blocking = 1;
		else
			lsd->blocking = 0;
	}

	*rv = 0;
	return(1);
}
