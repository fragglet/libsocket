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

#include <fcntl.h>
#include <sys/ioctl.h>

#include <lsck/if.h>
#include "unix.h"

/* ----------------
 * - __unix_ioctl -
 * ---------------- */

int __unix_ioctl (LSCK_SOCKET *lsd, int *rv, int request, int *param)
{
	LSCK_SOCKET_UNIX *usock = (LSCK_SOCKET_UNIX *) lsd->idata;
	int ret = -1;
	int handled = 0;	/* Set if we've handled ioctl here. */

	switch (request) {
	case FIONBIO:
		/* Set non-blocking I/O on mailslot fds */
		if (*param == 1) {
			ret = fcntl (usock->fd_w, F_SETFL, O_NONBLOCK);
			if (ret != -1)
				fcntl (usock->fd_r, F_SETFL, O_NONBLOCK);
			*rv = 0;
		} else {
			errno = EINVAL;
			*rv   = -1;			
		}
		handled = 1;
		break;

	/* Get the amount of data waiting. */	
	case FIONREAD:
		*rv = ioctl(usock->fd_w, request, param);
		handled = 1;
		break;
		
	default:
		break;
	}

	return(handled);
}
