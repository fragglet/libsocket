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

#include <lsck/if.h>
#include <lsck/ws.h>
#include <winsock.h>

#include "lsckglob.h"
#include "wsockvxd.h"
#include "farptrx.h"

int wsock_getsockopt (LSCK_SOCKET *lsd, int level, int optname, void *optval,
                      int *optlen)
{
    WSOCK_GETSOCKOPT_PARAMS params;

    /* RD: Check that the buffer won't overflow! */
    if ((*optlen + (6 * 4)) > _SocketP.size) {
        errno = EFAULT;
        return(-1);
    }

    params.Value = (void *) ((SocketP<<16)+ (6 * 4));
    params.Socket = (void *) lsd->wsock._Socket;
    params.OptionLevel = level;
    params.OptionName = optname;
    params.ValueLength = *optlen;
    params.IntValue = 0;

    _farpokex ( SocketP, 0, &params, sizeof ( WSOCK_GETSOCKOPT_PARAMS ) );
    _farpokex ( SocketP, 6 * 4, optval, *optlen );

    CallVxD ( WSOCK_GETSOCKOPT_CMD );

    if ( _VXDError && _VXDError != 0xffff ) return -1;

    _farpeekx ( SocketP, 0, &params, sizeof ( WSOCK_GETSOCKOPT_PARAMS ) );
    *optlen = params.ValueLength;
    _farpeekx ( SocketP, 6 * 4, optval, *optlen );

    return 0;
}

int wsock_setsockopt (LSCK_SOCKET *lsd, int level, int optname,
                      const void *optval, int optlen)
{
    WSOCK_SETSOCKOPT_PARAMS params;

    /* RD: Check that the buffer won't overflow! */
    if ((optlen + (6 * 4)) > _SocketP.size) {
        errno = EFAULT;
        return(-1);
    }

    params.Value = (void *) ((SocketP<<16)+ (6 * 4));
    params.Socket = (void *) lsd->wsock._Socket;
    params.OptionLevel = level;
    params.OptionName = optname;
    params.ValueLength = optlen;
    params.IntValue = *((int *)optval);

    _farpokex ( SocketP, 0, &params, sizeof ( WSOCK_SETSOCKOPT_PARAMS ) );
    _farpokex ( SocketP, 6 * 4, optval, optlen );

    CallVxD ( WSOCK_SETSOCKOPT_CMD );

    if ( _VXDError && _VXDError != 0xffff ) return -1;
    return 0;
}
