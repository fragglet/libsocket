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
#include <stdlib.h>
#include <sys/socket.h>

#include <lsck/lsck.h>
#include <lsck/if.h>

#include "winsock.h"
#include "lsckglob.h"

int listen (int s, int backlog)
{
    LSCK_SOCKET *lsd;

    if (!lsck_init()) {
        errno = ENODEV;
        return(-1);
    }

    lsd = lsckDescriptor[s];

    if (lsd == NULL) {
        errno = EBADF;
        return -1;
    }

    if (lsd->SocketType != SOCK_STREAM) {
        errno = EOPNOTSUPP;
        return -1;
    }

    switch(lsd->interface)
    {
        case LSCK_IF_WSOCK:
            return(wsock_listen(lsd, backlog));

        default:
            break;
    }

    /* Fail by default */
    return(-1);
}
