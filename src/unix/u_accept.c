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

/* -----------------
 * - __unix_accept -
 * ----------------- */

/* TODO: This should really return better error codes! */

int __unix_accept (LSCK_SOCKET * lsd, LSCK_SOCKET * nsd,
		   struct sockaddr *addr, size_t *addrlen)
{
	LSCK_SOCKET_UNIX *usock = (LSCK_SOCKET_UNIX *) lsd->idata;
	LSCK_SOCKET_UNIX *nusock = NULL;
	UNIX_MSG_OPEN_REQUEST req;
	UNIX_MSG_OPEN_REPLY rep;
	struct sockaddr_un *serv_sun = (struct sockaddr_un *) &lsd->sockname;
	struct sockaddr_un cli_sun;	/* Actual client address */
	char path[MAXPATH];
	char *p;
	int ret;

	/* Create a data struct for the new socket. */
	nusock = (LSCK_SOCKET_UNIX *) malloc(sizeof(*nusock));
	nsd->idata = (void *) nusock;

	if (nusock == NULL) {
		errno = ENOMEM;
		return(-1);
	}
	
	/* Zero the socket's peer name. */
	bzero(nusock->peerpath, sizeof(nusock->peerpath));
	
	/* Do we have enough space to store the address? */
	if (*addrlen < sizeof (struct sockaddr_un)) {
		errno = ENOBUFS;
		return (-1);
	}
	
	/* Listen on the server control mailslot */
	ret = read (usock->fd_r, &req, sizeof (req));
	if (ret == -1) return (-1);

	/* Check the message */
	if (req.header.type != UNIX_MSG_CLIENT_OPEN) return (-1);

	/* Create a mailslot for the client to write to, and one to write to
	 * the client. */
	sprintf (path, "%s%s/%08lx", UNIX_LOCAL_PATH, serv_sun->sun_path,
		 random());
	nusock->fd_r = open (path, O_BINARY | O_RDONLY, S_IRUSR);
	if (nusock->fd_r == -1) return (-1);

	nusock->fd_w = open (req.path, O_BINARY | O_WRONLY, S_IWUSR);
	if (nusock->fd_w == -1) return (-1);

	/* Create peer socket name */
	p = strstr (req.path, UNIX_LOCAL_PATH);
	
	if ((p == NULL) || (p != req.path)) {
		return (-1);
	}
	
	p += strlen (UNIX_LOCAL_PATH);
	cli_sun.sun_family = AF_UNIX;
	strcpy (cli_sun.sun_path, p);

	/* Store details in socket descriptor */
	memcpy(&nusock->secret, &req.header.secret,
	       sizeof(UNIX_MSG_SECRET));
	nusock->seq_w = random ();
	nusock->seq_r = (req.header.seq_end + 1) % MAXINT;

	/* Construct the reply header */
	bzero (&req, sizeof (rep));
	rep.header.version = UNIX_MSG_VERSION;
	rep.header.type = UNIX_MSG_SERVER_OPEN;
	memcpy(&rep.header.secret, &nusock->secret,
	       sizeof(UNIX_MSG_SECRET));
	rep.header.seq_start = nusock->seq_w;
	rep.header.seq_end = 0;	       /* Computer in a minute */
	rep.header.offset = sizeof (req.header);
	strcpy (rep.path, path);

	/* Finish constructing the header */
	rep.header.seq_end = (rep.header.seq_start + strlen (req.path)
			      /* + 1 - 1 */ ) % MAXINT;

	/* Send the reply to the client */
	ret = write (nusock->fd_w, &rep, sizeof (req));
	
	if (ret == -1) {
		return (-1);
	}
	
	/* Update the socket descriptor */
	nusock->seq_w = (rep.header.seq_end + 1) % MAXINT;

	/* Store peer details */
	strcpy(nusock->peerpath, cli_sun.sun_path);
	memcpy(addr, &cli_sun, sizeof (cli_sun));
	*addrlen = sizeof (cli_sun);

	/* Done, phew */
	return (0);
}
