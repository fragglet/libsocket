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
#include <time.h>

#include <lsck/if.h>
#include "unix.h"

/* -----------------
 * - __unix_select -
 * ----------------- */

int __unix_select (LSCK_SOCKET *lsd, int event)
{
	LSCK_SOCKET_UNIX *usock = (LSCK_SOCKET_UNIX *) lsd->idata;
	int nfds;
	fd_set fds;
	struct timeval tv;
	int ret;

	FD_ZERO(&fds);
	bzero(&tv, sizeof(tv));

	/* Perform select() on the unidirectional mailslots. */
	switch(event) {
	case FD_READ:
		FD_SET(usock->fd_r, &fds);
		nfds = usock->fd_r + 1;
		ret  = select(nfds, &fds, NULL, NULL, &tv);
		ret  = (ret > 0) ? 1 : ret;
		break;

	case FD_WRITE:
		FD_SET(usock->fd_w, &fds);
		nfds = usock->fd_w + 1;
		ret  = select(nfds, NULL, &fds, NULL, &tv);
		ret  = (ret > 0) ? 1 : ret;
		break;

	/* Error condition */
	/* TODO: Er, what exactly should we do? We could pass it onto the
	 * mailslots or we could handle it at this layer. For the moment,
	 * just pass both fds onto mailslot layer. */
	default:
		FD_SET(usock->fd_w, &fds);
		FD_SET(usock->fd_r, &fds);
		nfds = (usock->fd_w > usock->fd_r) ? usock->fd_w : usock->fd_r;
		nfds++;		
		ret  = select(nfds, NULL, NULL, &fds, &tv);
		ret  = (ret > 0) ? 1 : ret;
		break;		
	}

	return(ret);
}
