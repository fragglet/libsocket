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

/*
    fcntl.c

    Written by Richard Dawe (RD) - Code for F_SETFL written by Indrek Mandre
    (IM). fcntl() is unusual in that it doesn't actually require any interface
    code - this is handled by ioctlsocket().
*/

#include <fcntl.h>
#include <errno.h>

#include <lsck/lsck.h>

#include "winsock.h"
#include "lsckglob.h"

int fcntlsocket (int s, int command, int request)
{
    LSCK_SOCKET *lsd;
    int x = 1;          /* Used only ioctlsocket()                */
    int fakeflags = 0;  /* Used to build return value for F_GETFL */

    if (!lsck_init()) {
        errno = ENODEV;
        return(-1);
    }

    lsd = lsckDescriptor[s];

    /* Process the command appropriately */

    switch(command) {
        /* Set flags - taken from IM's original code in fsext.c */
        case F_SETFL:
            if (request == O_NONBLOCK) return(ioctlsocket(s, FIONBIO, &x));
            break;

        /* Get flags */
        case F_GETFL:
            /* Fake socket flags from flags element of socket struct, as the
               flags in the socket struct don't correspond one-to-one. */

            /* Only O_NONBLOCK is changed by the code - other flags? */
            if ((lsd->flags & LSCK_FLAG_BLOCKING) != LSCK_FLAG_BLOCKING)
                fakeflags |= O_NONBLOCK;

            return(fakeflags);
            break;

        /* Something unknown */
        default:
            break;
    }

    /* Fail by default */
    return(-1);
}
