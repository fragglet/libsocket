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

#include <errno.h>

#include <lsck/if.h>
#include "csock.h"

/* -----------------------
 * - __csock_getpeername -
 * ----------------------- */

int __csock_getpeername (LSCK_SOCKET * lsd,
                         struct sockaddr *name, size_t *namelen)
{
	LSCK_SOCKET_CSOCK *csock = (LSCK_SOCKET_CSOCK *) lsd->idata;    
	struct sockaddr_in *peer_sa = (struct sockaddr_in *) name;
	int peer_addr, peer_port;
	int ret = 0;

	if (*namelen < sizeof (struct sockaddr_in)) {
		errno = ENOBUFS;
		return (-1);
	}
	
	/* *INDENT-OFF* */
	__asm__ __volatile__ (
#if    (GAS_MAJOR == 2) \
    && ((GAS_MINOR < 9) || ((GAS_MINOR == 9) && (GAS_MINORMINOR < 5)))
			      "   lcall _csock_entry     \n\
                              "
#else    
                              "   lcall *_csock_entry     \n\
                              "
#endif    
                              "   jc 1f                  \n\
                                  xorl %%eax, %%eax      \n\
                               1:"
			      : "=a" (ret), "=b" (peer_port), "=c" (peer_addr)
			      : "a" (14), "D" (csock->fd)
			      : "cc"
			      );
	/* *INDENT-ON* */

	if (ret != 0) {
		errno = __csock_errno (ret);
		return (-1);
	}
	
	*namelen = sizeof (struct sockaddr_in);

	peer_sa->sin_family = AF_INET;
	peer_sa->sin_addr.s_addr = peer_addr;
	peer_sa->sin_port = peer_port;

	return (0);
}
