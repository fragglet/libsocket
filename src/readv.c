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

/* ---------
 * - readv -
 * --------- */

ssize_t readv (int fd, const struct iovec *iov, int iovcnt)
{
	void *buf = NULL;
	size_t maxbytes, nbytes, pos, remainder, len;
	ssize_t ret;
	int i;

	/* Check args */
	if ((iovcnt <= 0) || (iovcnt > IOV_MAX)) {
		errno = EINVAL;
		return(-1);
	}
	
	/* Calculate total number of bytes that can be read. */
	for (maxbytes = 0, i = 0; i < iovcnt; i++) {
		maxbytes += iov[i].iov_len;
	}

	/* If we read the maximum number of bytes, we may overflow the return
	 * value. Return an error if this is the case. */
	if (maxbytes > SSIZE_MAX) {
		errno = EINVAL;
		return(-1);
	}
	
	/* Allocate a buffer to store the data in. This buffer will be read
	 * in all at once. If the buffer is read all at once, we avoid the
	 * potential problem of dealing with multiple errors from read(). */
	buf = malloc(maxbytes);
	if (buf == NULL) {
		errno = ENOMEM;
		return(-1);
	}

	/* Read in the data. */
	ret = read(fd, buf, maxbytes);

	/* Pass through any errors that occur. */
	if (ret < 0) return(ret);

	/* Slice'n'dice the data according to the iovecs. */
	nbytes = (size_t) ret;
	
	for (remainder = nbytes, pos = 0, i = 0; i < iovcnt; i++) {
		/* If we have more data than we can store in this iovec,
		 * do a partial copy. Otherwise, copy the remaining data. */
		len = (iov[i].iov_len < remainder)
			? iov[i].iov_len
			: remainder;

		memcpy(iov[i].iov_base, (char *) buf + pos, len);
		pos += len;
		remainder -= len;
	}

	/* Tidy up */
	free(buf);
	
	return(nbytes);
}
