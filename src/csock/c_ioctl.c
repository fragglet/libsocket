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
#include <errno.h>

#include <lsck/if.h>
#include "csock.h"

/* -----------------
 * - __csock_ioctl -
 * ----------------- */

int __csock_ioctl (LSCK_SOCKET *lsd, int *rv, int request, int *param)
{
	LSCK_SOCKET_CSOCK *csock = (LSCK_SOCKET_CSOCK *) lsd->idata;
	int   ret;			/* Temporary return value     */
	int   handled = 0;		/* Have we handled the ioctl? */
	int   nonblocking   =  0;

	switch (request) {
	/* Non-blocking operations */
        case FIONBIO:
		nonblocking = (*param == 1) ? 1 : 0;

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
				      : "=a" (ret)
				      : "a" (15), "D" (csock->fd),
				        "c" (1), "d" (nonblocking)
				      : "cc"
				      );
		/* *INDENT-ON* */

		if (ret != 0) {
			errno = __csock_errno (ret);
			*rv = -1;
		} else {
			lsd->blocking = (nonblocking == 1) ? 0 : 1;
			*rv = 0;
		}
		handled = 1;
		break;

	/* Amount of data that can be read atomically from the socket. */
	case FIONREAD:
		/* TODO: Is this correct or should it return the amount of
		 * queued data for a stream? The Win32 API docs seem vague on
		 * this issue. */
		/* TODO: This should actually return the amount of data
		 * waiting to be read. */
		if (lsd->type == SOCK_STREAM) {
			*param  = CSOCK_RECV_BUFSIZE;
			*rv     = 0;
			handled = 1;
			break;
		}		

		ret     = -1;
		errno   = EINVAL;
		handled = 1;
		break;

	default:
		handled = 0;
		break;
	}

	return(handled);
}
