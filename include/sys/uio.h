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

#ifndef _SYS_UIO_H
#define _SYS_UIO_H

#ifndef __libsocket_uio_h__
#define __libsocket_uio_h__

#ifdef __cplusplus
extern "C"
{
#endif

/* Defines */
#define IOV_MAX	16
	
/* Types */
struct iovec {
	void   *iov_base;	/* Base address of a memory region for I/O */
	size_t  iov_len;	/* Size of memory region                   */
};

/* Functions */
extern ssize_t readv (int, const struct iovec *, int);
extern ssize_t writev (int, const struct iovec *, int);
    
#ifdef __cplusplus
}
#endif

#endif  /* __libsocket_uio_h__ */

#endif	/* _SYS_UIO_H */	
