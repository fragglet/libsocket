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

#include <time.h>

#include <lsck/if.h>

#include "winsock.h"
#include "wsockvxd.h"
#include "farptrx.h"
#include "lsckglob.h"

int wsock_socket (LSCK_SOCKET *lsd)
{
    WSOCK_SOCKET_PARAMS params;
    time_t now;

    time (&now);

    params.AddressFamily = lsd->AddressFamily;
    params.SocketType = lsd->SocketType;
    params.Protocol = lsd->Protocol;
    params.NewSocket = 0;
    params.NewSocketHandle = (int) now; /* IM: accoring to Alfons Hoogervorst
                                           here must be a uniqe value */

    _farpokex (SocketP, 0, &params, sizeof (WSOCK_SOCKET_PARAMS));

    CallVxD (WSOCK_SOCKET_CMD);

    _farpeekx (SocketP, 0, &params, sizeof (WSOCK_SOCKET_PARAMS));

    if (_VXDError && (_VXDError != 0xffff)) return -1;

    lsd->wsock._Socket = (int) params.NewSocket;
    lsd->wsock._SocketHandle = params.NewSocketHandle;
    lsd->interface = LSCK_IF_WSOCK;    /* WSOCK.VXD interface */

    return(0);
}

