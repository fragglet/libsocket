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
#include <sys/movedata.h>

#include <lsck/if.h>
#include <lsck/ws.h>
#include <winsock.h>

#include "lsckglob.h"
#include "wsockvxd.h"
#include "farptrx.h"

int wsock_recv (LSCK_SOCKET *lsd, void *buf, int len, unsigned int flags)
{
    WSOCK_RECV_PARAMS params;


    /* RD: Check that the request buffer length isn't bigger than the transfer
       buffer SocketD. If so, only transfer the size of SocketD. */
    if (len > _SocketD.size) len = _SocketD.size;

    params.Buffer = (void *) ((SocketD << 16));
    params.Address = NULL;
    params.Socket = (void *) lsd->wsock._Socket;
    params.BufferLength = len;
    params.Flags = flags;
    params.AddressLength = 0;
    params.BytesReceived = 0;
    params.ApcRoutine = NULL;
    params.ApcContext = 0;
    params.Timeout = 0;

    _farpokex ( SocketP, 0, &params, sizeof ( WSOCK_RECV_PARAMS ) );

    CallVxD ( WSOCK_RECV_CMD );

    /* RD: I'm not sure this if {} block is necessary. */
    if ( _VXDError == WSAEWOULDBLOCK || _VXDError == 0xFFFF ) /* Which?? */
    {
        if ( lsd->flags & LSCK_FLAG_BLOCKING ) {
            if (wsock_selectsocket_wait(lsd, FD_READ ) == -1) return -1;

            _farpokex ( SocketP, 0, &params, sizeof ( WSOCK_RECV_PARAMS ) );

            CallVxD ( WSOCK_RECV_CMD );
        } else return -1;
    }

    if (_VXDError) return -1;

    _farpeekx ( SocketP, 0, &params, sizeof ( WSOCK_RECV_PARAMS ) );
    movedata ( SocketD, 0, _my_ds(), (int)buf, params.BytesReceived );

    return params.BytesReceived;
}

int wsock_recvfrom (LSCK_SOCKET *lsd, void *buf, int len, unsigned int flags,
                    struct sockaddr *from, int *fromlen)
{
    WSOCK_RECV_PARAMS params;

    /* RD: Check that the request buffer length isn't bigger than the transfer
       buffer SocketD. If so, only transfer the size of SocketD. */
    if (len > _SocketD.size) len = _SocketD.size;

    params.Buffer = (void *) (SocketD << 16);
    params.Address = (void *) (SocketP << 16) + (10 * 4);
    params.Socket = (void *) lsd->wsock._Socket;
    params.BufferLength = len;
    params.Flags = flags;
    params.AddressLength = *fromlen;
    params.BytesReceived = 0;
    params.ApcRoutine = NULL;
    params.ApcContext = 0;
    params.Timeout = 0;

    _farpokex ( SocketP, 0, &params, sizeof ( WSOCK_RECV_PARAMS ) );
    _farpokex ( SocketP, 10 * 4, from, *fromlen );

    CallVxD ( WSOCK_RECV_CMD );

    /* RD: NB: errno set automatically */
    if ( _VXDError == WSAEWOULDBLOCK || _VXDError == 0xFFFF ) /* Which?? */
    {
        /* Blocking */
        if ( lsd->flags & LSCK_FLAG_BLOCKING ) {
            if (wsock_selectsocket_wait(lsd, FD_READ ) == -1) return -1;

            _farpokex ( SocketP, 10 * 4, from, *fromlen );
            _farpokex ( SocketP, 0, &params, sizeof ( WSOCK_RECV_PARAMS ) );

            CallVxD ( WSOCK_RECV_CMD );

        /* Non-blocking */
        } else return(-1);
    }

    if (_VXDError) return -1;

    _farpeekx ( SocketP, 0, &params, sizeof ( WSOCK_RECV_PARAMS ) );
    _farpeekx ( SocketD, 0, buf, params.BytesReceived);
    *fromlen = params.AddressLength;
    _farpeekx ( SocketP, 10 * 4, from, *fromlen );

    return params.BytesReceived;
}

