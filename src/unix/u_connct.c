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

/* Save some typing - pseudo-function - still require ; at end. I apologise
 * for the ug(h)ly typecast. */
#define __UNIX_UNBINDME(x)					\
{								\
	(x)->bound = 0;						\
	close(((LSCK_SOCKET_UNIX *)((x)->idata))->fd_r);	\
}

/* ------------------
 * - __unix_connect -
 * ------------------ */

/* TODO: More fault tolerance, add close handling. */

int __unix_connect (LSCK_SOCKET * lsd,
                    struct sockaddr *serv_addr, size_t addrlen)
{
	LSCK_SOCKET_UNIX *usock = (LSCK_SOCKET_UNIX *) lsd->idata;
	UNIX_MSG_OPEN_REQUEST req;
	UNIX_MSG_OPEN_REPLY rep;
	struct sockaddr_un sun;
	struct sockaddr_un *serv_sun = (struct sockaddr_un *) serv_addr;
	char path[MAXPATH];
	int fd, bind_here;
	char *p;
	int ret;

	/* Clear the current stored peer name. */
	bzero(usock->peerpath, sizeof(usock->peerpath));
	
	/* Create a file descriptor for the server control mailslot */
	sprintf (path, "%s%s", UNIX_LOCAL_PATH, serv_sun->sun_path);

	fd = open (path, O_WRONLY | O_BINARY, S_IWUSR);
	if (fd == -1) {
		/* TODO: Error? Or just pass through? */
		return (-1);
	}
	
	/* Construct the header */
	bzero (&req, sizeof (req));
	req.header.version    = UNIX_MSG_VERSION;
	req.header.type       = UNIX_MSG_CLIENT_OPEN;
	req.header.secret.n   = random ();
	req.header.secret.pid = getpid();
	req.header.seq_start  = random ();
	req.header.seq_end    = 0;	       /* Computer in a minute */
	req.header.offset     = sizeof (req.header);

	/* If bound to a local address, use that, otherwise create a local
	 * mailslot with a random name. */
	bind_here = !lsd->bound;

	if (bind_here) {
		sun.sun_family = AF_UNIX;
		sprintf (sun.sun_path, "/tmp/lsck/client/%08lx", random ());
		ret = bind (lsd->fd, (struct sockaddr *) &sun, sizeof (sun));
		if (ret == -1) return (-1);
	}
	
	sprintf (req.path, "%s%s", UNIX_LOCAL_PATH, usock->sockpath);

	/* Finish constructing the header */
	req.header.seq_end = (req.header.seq_start + strlen (req.path)
			      /* + 1 - 1 */ ) % MAXINT;

	/* Store the details */
	memcpy(&usock->secret, &req.header.secret,
	       sizeof(UNIX_MSG_SECRET));
	
	usock->seq_w = (req.header.seq_end + 1) % MAXINT;

	/* Send the request to the server & get the reply */
	ret = write (fd, &req, sizeof (req));
	if (ret != -1)
		ret = read (usock->fd_r, &rep, sizeof (rep));

	if (ret == -1) {
		/* If this function bound the socket, then unbind it,
		 * i.e. leave as it was before the call. */
		if (bind_here) __UNIX_UNBINDME (lsd);
		errno = ENETUNREACH;
		return (-1);
	}
	
	/* Check the reply */
	if ((memcmp(&rep.header.secret,
		    &usock->secret, sizeof(UNIX_MSG_SECRET)) != 0)
	    || (rep.header.type != UNIX_MSG_SERVER_OPEN)) {
		if (bind_here) __UNIX_UNBINDME (lsd);
		errno = ECONNREFUSED;
		return (-1);
	}
	
	/* Create a file descriptor for writing to server */
	usock->fd_w = open (rep.path, O_BINARY | O_WRONLY, S_IWUSR);

	if (fd == -1) {
		if (bind_here) __UNIX_UNBINDME (lsd);
		return (-1);
	}

	/* Check that the local path stem is present and at the start of the
	 * address. */
	p = strstr (rep.path, UNIX_LOCAL_PATH);
	
	if ((p == NULL) || (p != rep.path)) {
		if (bind_here) __UNIX_UNBINDME (lsd);
		errno = ECONNREFUSED;
		return (-1);
	}
    
	p += strlen (UNIX_LOCAL_PATH);
    
	/* Save the peer address for later. */
	strcpy(usock->peerpath, p);

	/* Done, phew */
	return (0);
}
