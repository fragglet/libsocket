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

/* INFO: Comments starting with 'IM' indicate that Indrek Mandre made changes.
   Comments starting wiht 'RD' indicate that Richard Dawe made changes. If no
   comment is given, then the code was written by Indrek Mandre. */

#include <stdio.h>
#include <stdlib.h>
#include <sys/farptr.h>
#include <sys/segments.h>
#include <dpmi.h>
#include <pc.h>
#include <sys/fsext.h>

#include <lsck/lsck.h>
#include <lsck/if.h>

#include "winsock.h"
#include "lsckglob.h"

int getsockopt (int s, int level, int optname, void *optval, int *optlen)
{
    LSCK_SOCKET *lsd;
    int ret;

    if (!lsck_init()) {
        errno = ENODEV;
        return(-1);
    }

    lsd = lsckDescriptor[s];

    if (lsd == NULL) {
        errno = EBADF;
        return -1;
    }

    /* Use appropriate interface */
    switch(lsd->interface) {
        case LSCK_IF_WSOCK:
            ret = wsock_getsockopt(lsd, level, optname, optval, optlen);
            break;

        default:
            ret = -1;
            break;
    }

    return(ret);
}

int setsockopt (int s, int level, int optname, const void *optval, int optlen)
{
    LSCK_SOCKET *lsd;
    int ret;

    lsd = lsckDescriptor[s];

    if (lsd == NULL) {
        errno = EBADF;
        return -1;
    }

    /* Use appropriate interface */
    switch(lsd->interface) {
        case LSCK_IF_WSOCK:
            ret = wsock_setsockopt(lsd, level, optname, optval, optlen);
            break;

        default:
            ret = -1;
            break;
    }

    return(ret);
}

