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
#include <string.h>
#include <dos.h>

#include <lsck/if.h>
#include <lsck/ws.h>
#include <winsock.h>

#include "lsckglob.h"
#include "wsockvxd.h"
#include "farptrx.h"

int wsock_selectsocket (LSCK_SOCKET *lsd, int event)
{
    WSOCK_SELECT_SETUP_PARAMS params;
    WSOCK_SELECT_CLEANUP_PARAMS params2;
    SOCK_LIST lps;
    WSIOSTATUS wios;
    int t;

    memset ( &wios, 0, sizeof ( WSIOSTATUS ) );
    memset ( &lps, 0, sizeof ( SOCK_LIST ) );
    memset ( &params, 0, sizeof ( WSOCK_SELECT_SETUP_PARAMS ) );

    params.ReadList = (void *)((SocketD<<16));
    params.WriteList = (void *)((SocketD<<16));
    params.ExceptList = (void *)((SocketD<<16));

    lps.Socket = (void *) lsd->wsock._Socket;
    lps.EventMask = event;
    lps.Context = 0;

    params.ReadCount = 1;
    params.WriteCount = 1;
    params.ExceptCount = 1;
    params.ApcRoutine = SPECIAL_16BIT_APC;
    params.ApcContext = (SocketP<<16)+100;

    _farpokex ( SocketP, 0, &params, sizeof ( WSOCK_SELECT_SETUP_PARAMS ) );
    _farpokex ( SocketD, 0, &lps, sizeof ( SOCK_LIST ) );
    _farpokex ( SocketP, 100, &wios, sizeof ( WSIOSTATUS ) );

    CallVxD ( WSOCK_SELECT_SETUP_CMD );

    if ( _VXDError && _VXDError != 0xffff ) return -1;

    t = (int)_farpeekb ( SocketP, 100 + 4 ); 
    if ( t <= 0 ) _farpokeb ( SocketP, 100+6, 1 );

    memset ( &params2, 0, sizeof ( WSOCK_SELECT_CLEANUP_PARAMS ) );

    params2.ReadList = (void *)(SocketD << 16 );
    params2.WriteList = (void *)((SocketD<<16));
    params2.ExceptList = (void *)((SocketD<<16));
    params2.ReadCount = 1;
    params2.WriteCount = 1;
    params2.ExceptCount = 1;

    _farpokex ( SocketP, 0, &params2, sizeof ( WSOCK_SELECT_CLEANUP_PARAMS ) );
    _farpokex ( SocketD, 0, &lps, sizeof ( SOCK_LIST ) );

    CallVxD ( WSOCK_SELECT_CLEANUP_CMD );

    if ( _VXDError && _VXDError != 0xffff ) return -1;

    return t;
}

int wsock_selectsocket_wait (LSCK_SOCKET *lsd, int event)
{
    WSOCK_SELECT_SETUP_PARAMS params;
    WSOCK_SELECT_CLEANUP_PARAMS params2;
    SOCK_LIST lps;
    WSIOSTATUS wios;

    memset ( &wios, 0, sizeof ( WSIOSTATUS ) );
    memset ( &lps, 0, sizeof ( SOCK_LIST ) );
    memset ( &params, 0, sizeof ( WSOCK_SELECT_SETUP_PARAMS ) );

    params.ReadList = (void *)((SocketD<<16));
    params.WriteList = (void *)((SocketD<<16));
    params.ExceptList = (void *)((SocketD<<16));

    lps.Socket = (void *) lsd->wsock._Socket;
    lps.EventMask = event;
    lps.Context = 0;

    params.ReadCount = 1;
    params.WriteCount = 1;
    params.ExceptCount = 1;
    params.ApcRoutine = SPECIAL_16BIT_APC;
    params.ApcContext = (SocketP<<16)+100;

    _farpokex ( SocketP, 0, &params, sizeof ( WSOCK_SELECT_SETUP_PARAMS ) );

    _farpokex ( SocketD, 0, &lps, sizeof ( SOCK_LIST ) );
    _farpokex ( SocketP, 100, &wios, sizeof ( WSIOSTATUS ) );

    CallVxD ( WSOCK_SELECT_SETUP_CMD );

    if ( _VXDError && _VXDError != 0xffff )	return -1;

    while ( _farpeekb ( SocketP, 100+4 ) == 0 );

    memset ( &params2, 0, sizeof ( WSOCK_SELECT_CLEANUP_PARAMS ) );

    params2.ReadList = (void *)(SocketD << 16 );
    params2.WriteList = (void *)(SocketD << 16 );
    params2.ExceptList = (void *)(SocketD << 16 );
    params2.ReadCount = 1;
    params2.WriteCount = 1;
    params2.ExceptCount = 1;

    _farpokex ( SocketP, 0, &params2, sizeof ( WSOCK_SELECT_CLEANUP_PARAMS ) );
    _farpokex ( SocketD, 0, &lps, sizeof ( SOCK_LIST ) );

    CallVxD ( WSOCK_SELECT_CLEANUP_CMD );

    if ( _VXDError && _VXDError != 0xffff )	return -1;

    return 0;
}

