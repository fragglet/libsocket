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
#include <unistd.h>
#include <values.h>
#include <dir.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <sys/socket.h>
#include <sys/un.h>

#include <lsck/if.h>
#include "unix.h"

#include "mailslot.h"

/* Function declarations */
ssize_t __unix_send_stream (LSCK_SOCKET *lsd,
			    void *msg, size_t len,
			    unsigned int flags);

ssize_t __unix_send_dgram (LSCK_SOCKET * lsd,
			   void *msg, size_t len,
			   unsigned int flags,
			   struct sockaddr *to, size_t tolen);

/* ---------------
 * - __unix_send -
 * --------------- */

ssize_t __unix_send (LSCK_SOCKET *lsd,
		     void *msg, size_t len,
		     unsigned int flags)
{
	/* Handle the send appropriately. */
	switch(lsd->type) {
	case SOCK_STREAM:
		return(__unix_send_stream(lsd, msg, len, flags));
		break;
		
	case SOCK_DGRAM:
		return(__unix_send_dgram(lsd, msg, len, flags, NULL, NULL));
		break;
	}

	errno = EOPNOTSUPP;
	return(-1);
}

/* -----------------
 * - __unix_sendto -
 * ----------------- */

ssize_t __unix_sendto (LSCK_SOCKET *lsd,
		       void *msg, size_t len,
		       unsigned int flags,
		       struct sockaddr *to, size_t tolen)
{
	/* Handle the send appropriately. */
	switch(lsd->type) {
	case SOCK_STREAM:
		return(__unix_send_stream(lsd, msg, len, flags));
		break;
		
	case SOCK_DGRAM:
		return(__unix_send_dgram(lsd, msg, len, flags, to, tolen));
		break;
	}

	errno = EOPNOTSUPP;
	return(-1);
}

/* ----------------------
 * - __unix_send_stream -
 * ---------------------- */

/* TODO: More fault tolerance, add close handling. */
ssize_t __unix_send_stream (LSCK_SOCKET *lsd,
			    void *msg, size_t len,
			    unsigned int flags)
{
	LSCK_SOCKET_UNIX *usock = (LSCK_SOCKET_UNIX *) lsd->idata;
	UNIX_MSG_HEADER header;
	static char buf[MAILSLOT_SENDSIZE];
	int total;
	size_t sendlen;
	ssize_t ret;

	/* Construct the header */
	header.version = UNIX_MSG_VERSION;
	header.type = UNIX_MSG_DATA;
	memcpy(&header.secret, &usock->secret, sizeof(UNIX_MSG_SECRET));
	header.offset = sizeof (header);

	/* Send all the data */
	for (total = 0; total < len;) {
		sendlen = (len - total) > (MAILSLOT_SENDSIZE - sizeof (header))
			? MAILSLOT_SENDSIZE - sizeof (header) : len - total;

		/* Finish the header */
		header.seq_start = usock->seq_w;
		header.seq_end = (usock->seq_w + sendlen - 1) % MAXINT;

		/* Fill the buffer */
		memcpy (buf, &header, sizeof (header));
		memcpy (buf + sizeof (header), msg + total, sendlen);

		/* Write the data */
		ret = write (usock->fd_w, buf, sizeof(header) + sendlen);

		if ((ret == -1) || ((ret - sizeof(header)) != sendlen)) {
			errno = EIO;
			return (-1);
		}
	
		usock->seq_w = (usock->seq_w + sendlen) % MAXINT;
		total += sendlen;
	}

	return (total);
}

/* ---------------------
 * - __unix_send_dgram -
 * --------------------- */

ssize_t __unix_send_dgram (LSCK_SOCKET * lsd,
			   void *msg, size_t len,
			   unsigned int flags,
			   struct sockaddr *to, size_t tolen)
{	
	static char buf[MAILSLOT_SENDSIZE];
	UNIX_MSG_DGRAM req;
	struct sockaddr_un sun;
	char dest_path[MAXPATH];
	int bind_here, fd;
	int total;
	size_t sendlen;
	ssize_t ret;

	/* Param checking */
	if ((tolen < sizeof (struct sockaddr_un))
	    && (to != NULL)) {
		errno = EINVAL;
		return (-1);
	}
    
	/* Only supported with datagram sockets */
	if (lsd->type != SOCK_DGRAM) {
		errno = EOPNOTSUPP;
		return (-1);
	}
	
	/* Construct the header */
	bzero(&req.header, sizeof(req.header));
	req.header.version = UNIX_MSG_VERSION;
	req.header.type    = (UNIX_MSG_DATA | UNIX_MSG_DGRAM_BIT);
	req.header.offset  = sizeof (req);

	/* If bound to a local address, use that, otherwise create a local
	 * mailslot with a random name. */
	bind_here = !lsd->bound;

	if (bind_here) {
		sun.sun_family = AF_UNIX;
		sprintf (sun.sun_path, "/tmp/lsck/client/%08lx", random ());
		ret = bind (lsd->fd, (struct sockaddr *) &sun, sizeof (sun));
		if (ret == -1) return (-1);
	}
    
	sprintf (req.path, "%s%s", UNIX_LOCAL_PATH,
		 ((struct sockaddr_un *) &lsd->sockname)->sun_path);

	/* Create a file descriptor for writing to this address. */
	sprintf (dest_path, "%s%s", UNIX_LOCAL_PATH,
		 ((struct sockaddr_un *) to)->sun_path);

	fd = open (dest_path, O_WRONLY | O_BINARY, S_IWUSR);
	if (fd == -1) {
		/* TODO: Error? Or just pass through? */
		return (-1);
	}
	
	/* Send all the data */
	for (total = 0; total < len;) {
		sendlen = (len - total) > (MAILSLOT_SENDSIZE - sizeof (req))
			? MAILSLOT_SENDSIZE - sizeof (req) : len - total;

		/* It's a datagram, so use the sequence numbers to indicate
		 * the length of data in this packet. */
		req.header.seq_start = 0;
		req.header.seq_end = len - 1;

		/* Fill the buffer */
		memcpy (buf, &req, sizeof (req));
		memcpy (buf + sizeof (req), msg + total, sendlen);

		/* Write the data */
		ret = write (fd, buf, sizeof(req) + sendlen);

		if ((ret == -1) || ((ret - sizeof(req)) != sendlen)) {
			errno = EIO;
			return (-1);
		}
	
		total += sendlen;
	}

	close(fd);
	
	return (total);
}
