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
#include <sys/ioctl.h>

#include <sys/socket.h>
#include <sys/un.h>

#include <lsck/if.h>
#include "unix.h"

#include "mailslot.h"

/* Function declarations */
ssize_t __unix_recv_stream (LSCK_SOCKET *lsd,
			    void *buf, size_t len,
			    unsigned int flags);

ssize_t __unix_recv_dgram (LSCK_SOCKET *lsd,
			   void *buf, size_t len, unsigned int flags,
			   struct sockaddr *from, size_t *fromlen);

/* ---------------
 * - __unix_recv -
 * --------------- */

ssize_t __unix_recv (LSCK_SOCKET *lsd,
                     void *buf, size_t len,
		     unsigned int flags)
{
	int ret;
	
	switch(lsd->type) {
	case SOCK_STREAM:
		ret = __unix_recv_stream(lsd, buf, len, flags);
		break;

	case SOCK_DGRAM:
		ret = __unix_recv_dgram(lsd, buf, len, flags, NULL, NULL);
		break;

	default:
		errno = EOPNOTSUPP;
		ret   = -1;
	}

	return(ret);
}

/* -------------------
 * - __unix_recvfrom -
 * ------------------- */

ssize_t __unix_recvfrom (LSCK_SOCKET *lsd,
                         void *buf, size_t len,
			 unsigned int flags,
                         struct sockaddr *from, size_t *fromlen)
{	
	ssize_t ret;
	
	switch(lsd->type) {
	case SOCK_STREAM:
		ret = __unix_recv_stream(lsd, buf, len, flags);
		if ((ret != -1) && (from != NULL) && (fromlen != NULL)) {
			/* Copy the address if necessary. */
			__unix_getpeername(lsd, from, fromlen);
		}		
		break;

	case SOCK_DGRAM:
		ret = __unix_recv_dgram(lsd, buf, len, flags, from, fromlen);
		break;

	default:
		errno = EOPNOTSUPP;
		ret   = -1;
		break;
	}

	return(ret);
}

/* ----------------------
 * - __unix_recv_stream -
 * ---------------------- */

/* TODO: Currently flaky because if len is < MAILSLOT_SENDSIZE, then data
 * will be lost. This could be solved by having a buffer associated with
 * each Unix domain socket descriptor, probably for both read and write
 * would be a good idea. */

ssize_t __unix_recv_stream (LSCK_SOCKET *lsd,
			    void *buf, size_t len,
			    unsigned int flags)
{
	LSCK_SOCKET_UNIX *usock = (LSCK_SOCKET_UNIX *) lsd->idata;
	UNIX_MSG_HEADER *header;
	static char my_buf[MAILSLOT_SENDSIZE];
	char *data;
	size_t recvlen;
	int peeking, pflag, bad_packet;
	int ret;

	/* Should we peek at the data? */
	peeking = ((flags & MSG_PEEK) == MSG_PEEK);
	
	/* Read the data one message at a time. Since multiple messages may
	 * have been written, read() will need to be called repeatedly until
	 * valid data has been read. */
	ret = bad_packet = 0;

	while (ret == 0) {
		/* If the last packet was bad, don't peek. Reprocess it, so it
		 * gets dumped. */
		if (!bad_packet && peeking) {
			pflag = peeking;
			ioctl(usock->fd_r, MIOPEEK, &pflag);
		}

		/* Read some data from the mailslot. */
		ret = read (usock->fd_r, my_buf, sizeof (my_buf));

		/* Turn off peeking, if necessary. */
		if (!bad_packet && peeking) {
			pflag = !pflag;
			ioctl(usock->fd_r, MIOPEEK, &pflag);
		}

		if (ret == -1) {
			errno = EIO;
			ret   = -1;
			break;
		}
	
		/* Get the data */
		header = (UNIX_MSG_HEADER *) my_buf;
		recvlen = header->seq_end - header->seq_start + 1;
		data = my_buf + header->offset;

		/* Is the packet a close packet? */
		if (((header->type == UNIX_MSG_SERVER_CLOSE)
		     && !lsd->accepted
		     && (memcmp(&usock->secret,
				&header->secret,
				sizeof(UNIX_MSG_SECRET)) == 0))
		    || ((header->type == UNIX_MSG_CLIENT_CLOSE)
			&& lsd->accepted
			&& (memcmp(&usock->secret,
				   &header->secret,
				   sizeof(UNIX_MSG_SECRET)) == 0))
		    ) {
			/* TODO: Does this close actually work? */
			/* Peer closing connection */
			__unix_close(lsd);
			errno = ECONNRESET;
			ret   = -1;
			break;
		}
	
		/* - Valid packet? If not, reject it. - */
		bad_packet = 0;

		/* Not a data packet */
		if ((header->type & UNIX_MSG_DATA) == 0) {
			bad_packet = 1;
			continue;
		}

		/* Stream, but the secrets do not match */
		if (((header->type & UNIX_MSG_DGRAM_BIT) == 0)
		    && (memcmp(&header->secret,
			       &usock->secret,
			       sizeof(UNIX_MSG_SECRET)) != 0)) {
			bad_packet = 1;
			continue;
		}

		/* Update sequence numbers if it's not a datagram */
		if ((header->type & UNIX_MSG_DGRAM_BIT) == 0)
			usock->seq_r = (usock->seq_r + recvlen) % MAXINT;

		/* Copy it into the callee's buffer */
		ret = (len < recvlen) ? len : recvlen;
		memcpy (buf, data, ret);
	}
	
	return(ret);
}

/* ---------------------
 * - __unix_recv_dgram -
 * --------------------- */

ssize_t __unix_recv_dgram (LSCK_SOCKET *lsd,
			   void *buf, size_t len, unsigned int flags,
			   struct sockaddr *from, size_t *fromlen)
{
	LSCK_SOCKET_UNIX *usock = (LSCK_SOCKET_UNIX *) lsd->idata;
	UNIX_MSG_DGRAM *rep;
	struct sockaddr_un sun;
	static char my_buf[MAILSLOT_SENDSIZE];
	char *data;
	size_t recvlen;
	int peeking, pflag, bad_packet;
	int ret;
	
	/* Only supported with datagram sockets */
	if (lsd->type != SOCK_DGRAM) {
		errno = EOPNOTSUPP;
		return(-1);
	}

	/* Should we peek at the data? */
	peeking = ((flags & MSG_PEEK) == MSG_PEEK);
	
	/* Read the data one message at a time. Since multiple messages may
	 * have been written, read() will need to be called repeatedly
	 * until valid data has been read. */
	ret = bad_packet = 0;

	while (ret == 0) {
		/* If the last packet was bad, don't peek. Reprocess it, so it
		 * gets dumped. */
		if (!bad_packet && peeking) {
			pflag = peeking;
			ioctl(usock->fd_r, MIOPEEK, &pflag);
		}

		/* Read some data from the mailslot. */
		ret = read(usock->fd_r, my_buf, sizeof (my_buf));

		/* Turn off peeking, if necessary. */
		if (!bad_packet && peeking) {
			pflag = !pflag;
			ioctl(usock->fd_r, MIOPEEK, &pflag);
		}
	
		if (ret == -1) {
			errno = EIO;
			ret   = -1;
			break;
		}
	
		/* Get the data */
		rep = (UNIX_MSG_DGRAM *) my_buf;
		recvlen = rep->header.seq_end - rep->header.seq_start + 1;
		data = my_buf + rep->header.offset;
	
		/* - Valid packet? If not, reject it. - */
		bad_packet = 0;

		/* Not a data packet */
		if ((rep->header.type
		     & (UNIX_MSG_DATA | UNIX_MSG_DGRAM_BIT)) == 0) {
			bad_packet = 1;
			continue;
		}

		/* Store socket address */
		if ((from != NULL) && (fromlen != NULL)) {
			bzero (&sun, sizeof (sun));
			sun.sun_family = AF_UNIX;
			strcpy (sun.sun_path, rep->path);
			if (*fromlen > sizeof(sun)) *fromlen = sizeof(sun);
			memcpy (from, &sun, *fromlen);	    
		}

		/* Copy it into the callee's buffer */
		ret = (len < recvlen) ? len : recvlen;
		memcpy (buf, data, ret);
	}
	
	return(ret);
}
