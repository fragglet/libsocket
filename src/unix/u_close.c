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
#include <values.h>
#include <dir.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sys/un.h>

#include <lsck/if.h>
#include "unix.h"

/* ----------------
 * - __unix_close -
 * ---------------- */

/* TODO: Lingering? */

int __unix_close (LSCK_SOCKET * lsd)
{
	LSCK_SOCKET_UNIX *usock = (LSCK_SOCKET_UNIX *) lsd->idata;
	UNIX_MSG_HEADER header;
	int ret;

	/* Construct the header */
	bzero (&header, sizeof (header));
	header.version = UNIX_MSG_VERSION;
	header.type = lsd->accepted ? UNIX_MSG_SERVER_CLOSE
		: UNIX_MSG_CLIENT_CLOSE;
	memcpy(&header.secret, &usock->secret, sizeof(UNIX_MSG_SECRET));
	header.seq_start = usock->seq_w;
	header.seq_end = usock->seq_w; /* No data */
	header.offset = sizeof (header);

	/* Send the request to the server & expect no reply */
	fcntl (usock->fd_w, F_SETFL, O_NONBLOCK);
	ret = write (usock->fd_w, &header, sizeof (header));
	usock->seq_w++;

	/* Close file descriptors */
	close (usock->fd_r);
	close (usock->fd_w);

	/* For safety, prevent further ops. */
	usock->fd_r = usock->fd_w = -1;

	/* Done */
	return (0);
}
