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

/* Originally by Indrek Mandre (IM), modifications by Richard Dawe (RD) */

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

int wsock_send (LSCK_SOCKET *lsd, void *msg, int len, unsigned int flags)
{
    WSOCK_SEND_PARAMS params;

    /* RD: Check the length of the data - only transmit up to the size of the
       buffer */
    if (len > _SocketD.size) len = _SocketD.size;

    params.Buffer = (void *) (SocketD << 16);
    params.Address = NULL;
    params.Socket = (void *) lsd->wsock._Socket;
    params.BufferLength = len;
    params.Flags = flags;
    params.AddressLength = 0;
    params.BytesSent = 0;
    params.ApcRoutine = NULL;
    params.ApcContext = 0;
    params.Timeout = 0;

    _farpokex ( SocketP, 0, &params, sizeof ( WSOCK_SEND_PARAMS ) );
    _farpokex ( SocketD, 0, msg, len );

    CallVxD ( WSOCK_SEND_CMD );

    if ( _VXDError && _VXDError != 0xffff ) return -1;

    _farpeekx ( SocketP, 0, &params, sizeof ( WSOCK_SEND_PARAMS ) );

    return params.BytesSent;
}

int wsock_sendto (LSCK_SOCKET *lsd, void *msg, int  len, unsigned int flags,
                  struct sockaddr *to, int tolen)
{
    WSOCK_SEND_PARAMS params;

    /* RD: Check the length of the data - only transmit up to the size of the
       buffer */
    if (len > _SocketD.size) len = _SocketD.size;

    params.Buffer = (void *) (SocketD << 16);
    params.Address = (void *) (SocketP << 16) + (10 * 4);
    params.Socket = (void *) lsd->wsock._Socket;
    params.BufferLength = len;
    params.Flags = flags;
    params.AddressLength = tolen;
    params.BytesSent = 0;
    params.ApcRoutine = NULL;
    params.ApcContext = 0;
    params.Timeout = 0;

    _farpokex ( SocketP, 0, &params, sizeof ( WSOCK_SEND_PARAMS ) );
    _farpokex ( SocketP, 10 * 4, to, tolen );
    _farpokex ( SocketD, 0, msg, len );

    CallVxD ( WSOCK_SEND_CMD );

    if ( _VXDError && _VXDError != 0xffff ) return -1;

    _farpeekx ( SocketP, 0, &params, sizeof ( WSOCK_SEND_PARAMS ) );

    return params.BytesSent;
}
