/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "i_priv.h"     /* RD: inetprivate.h -> i_priv.h */

#pragma weak writev = __writev

/* Write data pointed by the buffers described by VECTOR, which
   is a vector of COUNT `struct iovec's, to file descriptor FD.
   The data is written in the order specified.
   Operates just like `write' (see <unistd.h>) except that the data
   are taken from VECTOR instead of a contiguous buffer.  */

int __writev( int, const struct iovec *, size_t);

/* Not all versions of the kernel support the large number of records.
*/
#ifndef UIO_FASTIOV
# define UIO_FASTIOV	8	/* 8 is a safe number.  */
#endif

int
__writev (fd, vector, count)
      int fd;
      const struct iovec *vector;
      size_t count;
{
  int errno_saved = errno;
  char *buffer;
  register char *bp;
  size_t bytes, to_copy;
  register size_t i;

  /* We try the system call first. */
#define	min(a, b)	((a) > (b) ? (b) : (a))

  errno = errno_saved;

  /* Find the total number of bytes to be written.  */
  bytes = 0;
  for (i = 0; i < count; ++i)
    bytes += vector[i].iov_len;

  if (bytes == 0)
    return 0;

  /* Allocate a temporary buffer to hold the data.  */
  buffer = (char *) alloca(bytes);

  /* Copy the data into BUFFER.  */
  to_copy = bytes;
  bp = buffer;
  for (i = 0; i < count; ++i)
    {
      size_t copy = min(vector[i].iov_len, to_copy);

      (void) memcpy( bp, vector[i].iov_base, copy);

      bp += copy;
      to_copy -= copy;
      if (to_copy == 0)
	break;
    }

  return write(fd, buffer, bytes);
}
