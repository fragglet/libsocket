/*
 *  libsocket - BSD socket like library for DJGPP
 *  Copyright 1997, 1998 by Indrek Mandre
 *  Copyright 1997-2000 by Richard Dawe
 *
 *  Portions of libsocket Copyright 1985-1993 Regents of the University of 
 *  California.
 *  Portions of libsocket Copyright 1991, 1992 Free Software Foundation, Inc.
 *  Portions of libsocket Copyright 1997, 1998 by the Regdos Group.
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
#include <string.h>
#include <unistd.h>

#include <dpmi.h>
#include <sys/farptr.h>
#include <sys/segments.h>

#include <sys/socket.h>

#include <lsck/lsck.h>
#include <lsck/if.h>
#include "wsock.h"
#include "farptrx.h"
#include "wsockvxd.h"

/* Waiting for wsock_select_wait() to finish - for tidying up. */
static int wsock_select_waiting = 0;

/* ------------------
 * - __wsock_select -
 * ------------------ */

int __wsock_select (LSCK_SOCKET * lsd, int event)
{
	LSCK_SOCKET_WSOCK *wsock = (LSCK_SOCKET_WSOCK *) lsd->idata;
	WSOCK_SELECT_SETUP_PARAMS params;
	WSOCK_SELECT_CLEANUP_PARAMS params2;
	SOCK_LIST lps;
	WSIOSTATUS wios;
	int t;

	if (__lsck_debug_getlevel () >= LSCK_DEBUG_VERBOSE) {
		printf ("Entering __wsock_select()\n");
		fflush (stdout);
	}
	
	/* - Set up all parameters before calls - */
	bzero (&wios, sizeof (wios));
	bzero (&lps, sizeof (lps));
	bzero (&params, sizeof (params));
	bzero (&params2, sizeof (params2));

	lps.Socket = (void *) wsock->_Socket;
	lps.EventMask = event;
	lps.Context = wsock->_SocketHandle;

	/* Input parameters */
	params.ReadList = (void *) (SocketSD << 16);
	params.WriteList = (void *) (SocketSD << 16) + sizeof (SOCK_LIST);
	params.ExceptList = (void *) (SocketSD << 16)
		+ (2 * sizeof (SOCK_LIST));
	params.ApcRoutine = SPECIAL_16BIT_APC;
	params.ApcContext = (SocketSP << 16) + 100;

	/* Output parameters */
	params2.ReadList = (void *) (SocketSD2 << 16);
	params2.WriteList = (void *) (SocketSD2 << 16) + sizeof (SOCK_LIST);
	params2.ExceptList = (void *) (SocketSD2 << 16)
		+ (2 * sizeof (SOCK_LIST));

	/* The count lists */
	switch (event) {
	case FD_READ:
		params.ReadCount = params2.ReadCount = 1;
		params.WriteCount = params2.WriteCount = 0;
		params.ExceptCount = params2.ExceptCount = 0;
		break;

	case FD_WRITE:
		params.ReadCount = params2.ReadCount = 0;
		params.WriteCount = params2.WriteCount = 1;
		params.ExceptCount = params2.ExceptCount = 0;
		break;

	    /* TODO: Is this correct? Is this how the other FD_* should be
	     * handled? This solution seems better than previous one of
	     * select'ing on all 3 here. I think only one event is passed
	     * at a time. */
	default:
		params.ReadCount = params2.ReadCount = 0;
		params.WriteCount = params2.WriteCount = 0;
		params.ExceptCount = params2.ExceptCount = 1;
		break;
	}

	/* Fill up the memory */
	_farpokex (SocketSD, 0, &lps, sizeof (SOCK_LIST));
	_farpokex (SocketSD, sizeof (SOCK_LIST), &lps, sizeof (SOCK_LIST));
	_farpokex (SocketSD, 2 * sizeof (SOCK_LIST),
		   &lps, sizeof (SOCK_LIST));

	_farpokex (SocketSD2, 0, &lps, sizeof (SOCK_LIST));
	_farpokex (SocketSD2, sizeof (SOCK_LIST), &lps, sizeof (SOCK_LIST));
	_farpokex (SocketSD2, 2 * sizeof (SOCK_LIST),
		   &lps, sizeof (SOCK_LIST));

	/* Do the select() op */
	_farpokex (SocketSP, 0, &params, sizeof (WSOCK_SELECT_SETUP_PARAMS));
	_farpokex (SocketSP, 100, &wios, sizeof (WSIOSTATUS));

	__wsock_callvxd2 (WSOCK_SELECT_SETUP_CMD, SocketSP);

	if (_VXDError && _VXDError != 0xffff) return -1;

	/* TODO: Make this peek the wios struct from offset 100. */
	t = (int) _farpeekb (SocketSP, 100 + 4);	/* IoCompleted flag */

	_farpokex (SocketSP2, 0,
		   &params2, sizeof (WSOCK_SELECT_CLEANUP_PARAMS));

	if (__lsck_debug_getlevel () >= LSCK_DEBUG_VERBOSE) {
		printf ("Entering __wsock_select() clean up\n");
		fflush (stdout);
	}
	
	/* RD: Use a different descriptor to the first call. The reason: I
	 * think using the same data for the setup and cleanup calls might
	 * cause a protection fault. */
	__wsock_callvxd2 (WSOCK_SELECT_CLEANUP_CMD, SocketSP2);

	if (_VXDError && _VXDError != 0xffff) return -1;

	return t;
}

/* -----------------------
 * - __wsock_select_wait -
 * ----------------------- */

int __wsock_select_wait (LSCK_SOCKET * lsd, int event)
{
	LSCK_SOCKET_WSOCK *wsock = (LSCK_SOCKET_WSOCK *) lsd->idata;
	WSOCK_SELECT_SETUP_PARAMS params;
	WSOCK_SELECT_CLEANUP_PARAMS params2;
	SOCK_LIST lps;
	WSIOSTATUS wios;

	if (__lsck_debug_getlevel () >= LSCK_DEBUG_VERBOSE) {
		printf ("Entering __wsock_socket_wait()\n");
		fflush (stdout);
	}
	
	/* - Set up all parameters before calls - */
	bzero (&wios, sizeof (wios));
	bzero (&lps, sizeof (lps));
	bzero (&params, sizeof (params));
	bzero (&params2, sizeof (params2));

	lps.Socket = (void *) wsock->_Socket;
	lps.EventMask = event;
	lps.Context = wsock->_SocketHandle;

	/* Input parameters */
	params.ReadList = (void *) (SocketSD << 16);
	params.WriteList = (void *) (SocketSD << 16) + sizeof (SOCK_LIST);
	params.ExceptList = (void *) (SocketSD << 16)
		+ (2 * sizeof (SOCK_LIST));
	params.ApcRoutine = SPECIAL_16BIT_APC;
	params.ApcContext = (SocketSP << 16) + 100;

	/* Output parameters */
	params2.ReadList = (void *) (SocketSD2 << 16);
	params2.WriteList = (void *) (SocketSD2 << 16) + sizeof (SOCK_LIST);
	params2.ExceptList = (void *) (SocketSD2 << 16)
		+ (2 * sizeof (SOCK_LIST));

	/* The count lists */
	switch (event) {
	case FD_READ:
		params.ReadCount = params2.ReadCount = 1;
		params.WriteCount = params2.WriteCount = 0;
		params.ExceptCount = params2.ExceptCount = 0;
		break;

	case FD_WRITE:
		params.ReadCount = params2.ReadCount = 0;
		params.WriteCount = params2.WriteCount = 1;
		params.ExceptCount = params2.ExceptCount = 0;
		break;

	    /* TODO: Is this correct? Is this how the other FD_* should be
	     * handled? This solution seems better than previous one of
	     * select'ing on all 3 here. I think only one event is passed
	     * at a time. */
	default:
		params.ReadCount = params2.ReadCount = 0;
		params.WriteCount = params2.WriteCount = 0;
		params.ExceptCount = params2.ExceptCount = 1;
		break;
	}

	/* Fill up the memory */
	_farpokex (SocketSD, 0, &lps, sizeof (SOCK_LIST));
	_farpokex (SocketSD, sizeof (SOCK_LIST), &lps, sizeof (SOCK_LIST));
	_farpokex (SocketSD, 2 * sizeof (SOCK_LIST),
		   &lps, sizeof (SOCK_LIST));

	_farpokex (SocketSD2, 0, &lps, sizeof (SOCK_LIST));
	_farpokex (SocketSD2, sizeof (SOCK_LIST), &lps, sizeof (SOCK_LIST));
	_farpokex (SocketSD2, 2 * sizeof (SOCK_LIST),
		   &lps, sizeof (SOCK_LIST));

	/* Do the select() op */
	_farpokex (SocketSP, 0, &params, sizeof (WSOCK_SELECT_SETUP_PARAMS));
	_farpokex (SocketSP, 100, &wios, sizeof (WSIOSTATUS));

	__wsock_callvxd2 (WSOCK_SELECT_SETUP_CMD, SocketSP);

	if (_VXDError && (_VXDError != 0xFFFF)) return (-1);

	if (__lsck_debug_getlevel () >= LSCK_DEBUG_VERBOSE) {
		printf ("Entering __wsock_socket_wait() loop\n");
		fflush (stdout);
	}
	
	/* This was the old code:
	 *
	 * _farpeekb(SocketSP, 100 + 4) == 0) { __dpmi_yield(); }
	 * 
	 * which works fine, but this is easier to debug & clearer: */
	wsock_select_waiting = 1;
	while (wsock_select_waiting) {
		__dpmi_yield ();
		_farpeekx (SocketSP, 100, &wios, sizeof (WSIOSTATUS));
		if (wios.IoCompleted) wsock_select_waiting = 0;
	}

	_farpokex (SocketSP2, 0,
		   &params2, sizeof (WSOCK_SELECT_CLEANUP_PARAMS));

	__wsock_callvxd2 (WSOCK_SELECT_CLEANUP_CMD, SocketSP2);

	if (_VXDError && (_VXDError != 0xFFFF)) return (-1);

	return (1);
}

/* ----------------------------
 * - __wsock_select_wait_kill -
 * ---------------------------- */

void __wsock_select_wait_kill (void)
{
	if (!wsock_select_waiting)
		return;
	else
		__wsock_callvxd2 (WSOCK_SELECT_CLEANUP_CMD, SocketSP2);
}
