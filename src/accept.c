/*
 *  libsocket - BSD socket like library for DJGPP
 *  Copyright 1997, 1998 by Indrek Mandre
 *  Copyright 1997, 1998 by Richard Dawe
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
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/fsext.h>

#include <lsck/lsck.h>
#include <lsck/if.h>
#include <lsck/ws.h>
#include <winsock.h>

#include "lsckglob.h"
#include "farptrx.h"

int sock_functions(__FSEXT_Fnumber func_number, int *rv, va_list args);

/* ----------
   - accept -
   ---------- */

int accept (int s, struct sockaddr *addr, int *addrlen)
{
    LSCK_SOCKET *lsd;   /* Current socket descriptor */
    LSCK_SOCKET *nsd;   /* New socket descriptor     */
    int newfd;          /* New file descriptor       */
    int ret;            /* Return value              */

    if (!lsck_init()) {
        errno = ENODEV;
        return(-1);
    }

    lsd = lsckDescriptor[s];

    if (lsd == NULL) {
        errno = EBADF;
        return -1;
    }

    if (lsd->SocketType != SOCK_STREAM ) {
        errno = EOPNOTSUPP;
        return(-1);
    }

    newfd = __FSEXT_alloc_fd(sock_functions);

    if (newfd < 0) {
        errno = EMFILE;
        return(-1);
    }

    setmode (newfd, O_BINARY);

    if (lsckDescriptor[newfd] != NULL) free (lsckDescriptor[newfd]);
    nsd = lsckDescriptor[newfd] = malloc(sizeof(LSCK_SOCKET));

    if (nsd == NULL) {
        _close(newfd);
        errno = ENOBUFS;
        return(-1);
    }

    nsd->AddressFamily = lsd->AddressFamily;
    nsd->SocketType = lsd->SocketType;
    nsd->Protocol = lsd->Protocol;
    nsd->flags = LSCK_FLAG_BLOCKING;
    nsd->fd = newfd;

    /* Always accept on the same interface! */
    switch (lsd->interface) {
        case LSCK_IF_WSOCK:
            ret = wsock_accept(lsd, nsd, addr, addrlen);
            break;

        default:
            ret = -1;
            break;
    }

    if (ret == -1) _close(newfd);
    return(ret);
}

