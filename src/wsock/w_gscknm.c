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

#include <stdio.h>
#include <stdlib.h>
#include <sys/farptr.h>
#include <sys/segments.h>
#include <dpmi.h>
#include <pc.h>
#include <sys/fsext.h>

#include <lsck/if.h>
#include <lsck/ws.h>
#include <winsock.h>

#include "lsckglob.h"
#include "wsockvxd.h"
#include "farptrx.h"

int wsock_getsockname (LSCK_SOCKET *lsd, struct sockaddr * name, int * namelen)
{
    WSOCK_GETSOCKNAME_PARAMS params;

    /* RD: Check the buffer is large enough to perform the operation */
    if ((*namelen + (3 * 4)) > _SocketP.size) {
        errno = ENOBUFS;
        return(-1);
    }

    params.Address = (void *) ( (SocketP << 16) + ( 3 * 4 ) );
    params.Socket = (void *) lsd->wsock._Socket;
    params.AddressLength = *namelen;

    _farpokex ( SocketP, 0, &params, sizeof ( WSOCK_GETSOCKNAME_PARAMS ) );
    _farpokex ( SocketP, 3 * 4, name, *namelen );

    CallVxD ( WSOCK_GETSOCKNAME_CMD );

    if ( _VXDError && _VXDError != 0xffff )
        return -1;

    _farpeekx ( SocketP, 0, &params, sizeof ( WSOCK_GETSOCKNAME_PARAMS ) );
    *namelen = params.AddressLength;
    _farpeekx ( SocketP, 3 * 4, name, *namelen );

    return 0;
}

