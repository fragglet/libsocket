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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

#include <sys/uio.h>

/* ----------
 * - writev -
 * ---------- */

ssize_t writev (int fd, const struct iovec *iov, int iovcnt)
{
	void *buf = NULL;
	size_t nbytes, pos;
	ssize_t ret;
	int i;

	/* Check args */
	if ((iovcnt <= 0) || (iovcnt > IOV_MAX)) {
		errno = EINVAL;
		return(-1);
	}
	
	/* Calculate the total number of bytes needed to write data. */
	for (nbytes = 0, i = 0; i < iovcnt; i++) { nbytes += iov[i].iov_len; }

	/* If we write the maximum number of bytes, we may overflow the return
	 * value. Return an error if this is the case. */
	if (nbytes > SSIZE_MAX) {
		errno = EINVAL;
		return(-1);
	}
	
	/* Allocate a buffer to store data in. This buffer will be written
	 * out all at once. If the buffer is written all at once, we avoid
	 * the potential problem of dealing with multiple errors from
	 * write(). */
	buf = malloc(nbytes);
	if (buf == NULL) {
		errno = ENOMEM;
		return(-1);
	}

	/* Copy all the data into the buffer. */
	for (pos = 0, i = 0; i < iovcnt; i++) {
		memcpy((char *) buf + pos, iov[i].iov_base, iov[i].iov_len);
		pos += iov[i].iov_len;
	}

	/* Write and free the buffer. */
	ret = write(fd, buf, nbytes);
	free(buf);
	
	return(ret);
}
