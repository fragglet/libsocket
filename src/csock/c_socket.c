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

/* Track which fd's are in use */
unsigned long __csock_fd_table = 0;

/* NOTE: This mirrors how SOCK.VXD keeps track of file descriptors. This ugly
 * hack is needed to stop __csock_accept() from failing, if possible. If
 * __csock_accept() uses an existing SOCK.VXD file descriptor, then the
 * waiting connection will be dropped and a re-connection will be forced. This
 * is not good, especially as the client appears not to notice the failure. */

/* ------------------------
 * - __csock_fd_set_usage -
 * ------------------------ */

void __csock_fd_set_usage (int fd, int flag)
{
    if (flag)
	__csock_fd_table |= 1UL << fd;
    else
	__csock_fd_table = __csock_fd_table ^ (1UL << fd);
}

void __csock_fd_set_used (int fd)
{
    __csock_fd_set_usage (fd, 1);
}
void __csock_fd_set_clear (int fd)
{
    __csock_fd_set_usage (fd, 0);
}

/* ------------------------
 * - __csock_fd_get_usage -
 * ------------------------ */

int __csock_fd_get_usage (int fd)
{
    int ret = 0;

    /* SOCK.VXD version 1 has a function call to do this. Use it if
     * possible. */
    if (__csock_version > 0) {
        __asm__ __volatile__ (
#if    (GAS_MAJOR == 2) \
    && ((GAS_MINOR < 9) || ((GAS_MINOR == 9) && (GAS_MINORMINOR < 5)))
			      "lcall _csock_entry"
#else
                              "lcall *_csock_entry"    
#endif
                              : "=a" (ret)
			      : "a" (16)
			      : "cc");

        /* This call is probably more reliable than libsocket, so use its
	 * value for the fd table. -1 indicates that no fd's are in use. */
	if (ret == -1) return(0);
	if (ret >   0) __csock_fd_table = ret;
    }

    /* Check the table first - may be able to avoid calling SOCK.VXD */
    if (__csock_fd_table & (1UL << fd)) return (1);

    /* It's not in the table, but another process in the DOS VM might be using
     * it, so "probe" the file descriptor by creating & then closing a socket
     * using the value of 'fd'. This piece of assembly sets ret to 0 if the
     * fd is free, 1 else. If the socket creation succeeds, but the close
     * fails, then the socket is indicated as being used. */
	/* *INDENT-OFF* */
	__asm__ __volatile__ ( "    movl $1, %%eax     \n\
	                            movl $6, %%ebx     \n\
                               "
#if    (GAS_MAJOR == 2) \
    && ((GAS_MINOR < 9) || ((GAS_MINOR == 9) && (GAS_MINORMINOR < 5)))
	                       "    lcall _csock_entry \n\
                               "
#else			       
			       "    lcall *_csock_entry \n\
                               "
#endif			       
	                       "    jc 1f              \n\
	                            movl $2, %%eax     \n\
	                            xorl %%ebx, %%ebx  \n\
                               "
#if    (GAS_MAJOR == 2) \
    && ((GAS_MINOR < 9) || ((GAS_MINOR == 9) && (GAS_MINORMINOR < 5)))
	                       "    lcall _csock_entry \n\
                               "
#else			       
			       "    lcall *_csock_entry \n\
                               "
#endif			       
	                       "    jc 1f              \n\
	                            xorl %%eax, %%eax  \n\
	                            jmp 2f             \n\
	                        1:                     \n\
	                            movl $1, %%eax     \n\
	                        2:"
	                      : "=a" (ret)
	                      : "D"  (fd)
	                      : "cc" );
	/* *INDENT-ON* */

    if (ret) {
	__csock_fd_set_used (fd);
	return (1);
    }
    /* Nope, it's free */
    return (0);
}

/* ------------------
 * - __csock_socket -
 * ------------------ */

int __csock_socket (LSCK_SOCKET *lsd)
{
	LSCK_SOCKET_CSOCK *csock = NULL;
	int flag = 0;
	int ret = -1;
	int rv;
	int i;

	/* Allocate a buffer for the interface data. */
	csock = (LSCK_SOCKET_CSOCK *) malloc(sizeof(*csock));
	lsd->idata = (void *) csock;

	if (csock == NULL) {
		errno = ENOMEM;
		return(-1);
	}
	
	/* Loop on file descriptors from 1 to 31 until one creates
	 * successfully. Don't use fd 0 as this could be confusing. */
	for (csock->fd = -1, i = 1; i < 32; i++) {
		/* fd already being used */
		if (__csock_fd_get_usage (i)) continue;

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
				      : "a"  (1), "b" (lsd->protocol), "D" (i)
				      : "cc"
				      );
		/* *INDENT-ON* */

		/* Got one! */
		if (ret == 0) {
			csock->fd = i;
			break;
		}
		
		/* This should be the only non-fatal error here. Any other
		 * means there are no free descriptors or there's no memory
		 * left. NB: SOCK.VXD returns its errors negatively! */
		if (ret != -CSOCK_ERR_FD_INUSE)
			break;
		else
			__csock_fd_set_used (i);
	}

	if (csock->fd == -1) {
		/* No free descriptors! */
		/* TODO: Does SOCK.VXD allocate per process or per system? If
		 * it's per system, then this needs to be changed to ENFILE. */
		errno = EMFILE;
		return (-1);
	}
    
	if (ret != 0) {
		errno = __csock_errno (ret);
		return (-1);
	}
	
	__csock_fd_set_used (csock->fd);		/* fd now used! */
	__csock_ioctl (lsd, &rv, FIONBIO, &flag);	/* Blocking I/O */

	return (0);
}
