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
#include <io.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

#include <lsck/if.h>
#include <lsck/ws.h>
#include <winsock.h>

#include "lsckglob.h"
#include "wsockvxd.h"
#include "farptrx.h"

int wsock_accept (LSCK_SOCKET *lsd, LSCK_SOCKET *nsd,
                  struct sockaddr *addr, int *addrlen)
{
    WSOCK_ACCEPT_PARAMS params;
    time_t now;

    time (&now);

    params.Address = (void *) ( (SocketP << 16) + (7 * 4) );
    params.ListeningSocket = (void *) lsd->wsock._Socket;
    params.ConnectedSocket = NULL;
    params.AddressLength = *addrlen;
    params.ConnectedSocketHandle = (int) now;
    params.ApcRoutine = 0;
    params.ApcContext = 0;

    if ((lsd->flags & LSCK_FLAG_BLOCKING) == LSCK_FLAG_BLOCKING)
        if (wsock_selectsocket_wait (lsd, FD_ACCEPT) == -1) return(-1);

    _farpokex (SocketP, 0, &params, sizeof (WSOCK_ACCEPT_PARAMS));

    CallVxD (WSOCK_ACCEPT_CMD);

    if (_VXDError && _VXDError != 0xffff ) return(-1);

    _farpeekx (SocketP, 0, &params, sizeof (WSOCK_ACCEPT_PARAMS));

    if (params.ConnectedSocket == NULL) return(-1);

    *addrlen = params.AddressLength;

    _farpeekx (SocketP, 7 * 4, addr, *addrlen);

    nsd->wsock._Socket = (int) params.ConnectedSocket;
    nsd->wsock._SocketHandle = params.ConnectedSocketHandle;
    nsd->interface = LSCK_IF_WSOCK; /* wsock_accept() => WSOCK.VXD interface */

    memcpy (&nsd->wsock.inetaddr, addr, *addrlen);

    return(nsd->fd);
}

