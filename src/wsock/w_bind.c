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

#include <stdlib.h>
#include <sys/socket.h>

#include <lsck/if.h>
#include <lsck/ws.h>
#include <winsock.h>

#include "lsckglob.h"
#include "wsockvxd.h"
#include "farptrx.h"

int wsock_bind (LSCK_SOCKET *lsd, struct sockaddr *my_addr, int addrlen)
{
    WSOCK_BIND_PARAMS params;

    params.Address = (void *)((SocketP << 16) + (5 * 4));
    params.Socket = (void *) lsd->wsock._Socket;
    params.AddressLength = addrlen;
    params.ApcRoutine = 0;
    params.ApcContext = 0;

    _farpokex (SocketP, 0, &params, sizeof (WSOCK_BIND_PARAMS));
    _farpokex (SocketP, 5 * 4, my_addr, addrlen);

    CallVxD (WSOCK_BIND_CMD);

    if (_VXDError && _VXDError != 0xffff) return -1;

    _farpeekx (SocketP, 5 * 4, my_addr, addrlen);

    memcpy(&lsd->wsock.inetaddr, my_addr, sizeof (struct sockaddr));

    return(0);
}

