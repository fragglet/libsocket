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

/* INFO: Comments starting with 'IM' indicate that Indrek Mandre made changes.
 * Comments starting wiht 'RD' indicate that Richard Dawe made changes. If no
 * comment is given, then the code was written by Indrek Mandre. */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <dpmi.h>
#include <sys/farptr.h>
#include <sys/segments.h>

#include <sys/socket.h>

#include <lsck/if.h>
#include "wsock.h"
#include "farptrx.h"
#include "wsockvxd.h"

/* ----------------------
 * - __wsock_getsockopt -
 * ---------------------- */

/* TODO: This needs to perform better parameter-length checking. */

int __wsock_getsockopt (LSCK_SOCKET *lsd, int *rv,
			int level, int optname,
                        void *optval, size_t *optlen)
{
	LSCK_SOCKET_WSOCK *wsock = (LSCK_SOCKET_WSOCK *) lsd->idata;
	WSOCK_GETSOCKOPT_PARAMS params;
	int is_int = 0;

	switch (optname) {
	/* Catch the non-supported BSD options. WSOCK.VXD doesn't seem to
	 * return error messages for some of these, in spite of the fact
	 * they're not supported by Winsock (source: Win32 API help file)! */
	case SO_RCVLOWAT:
	case SO_RCVTIMEO:
	case SO_SNDLOWAT:
	case SO_SNDTIMEO:
		errno = ENOPROTOOPT;
		*rv = -1;
		return(1);
		break;

	default:
		break;
	}
	
	/* RD: Check that the buffer won't overflow! */
	if ((*optlen + sizeof (WSOCK_GETSOCKOPT_PARAMS)) > _SocketP.size) {
		errno = ENOBUFS;
		*rv   = -1;
		return(1);
	}
	
	bzero (&params, sizeof (params));

	params.Value = (void *) (  (SocketP << 16)
				 + sizeof (WSOCK_GETSOCKOPT_PARAMS));
	params.Socket = (void *) wsock->_Socket;
	params.OptionLevel = level;
	params.OptionName = optname;
	params.ValueLength = *optlen;
	params.IntValue = 0;

	_farpokex (SocketP, 0, &params, sizeof (WSOCK_GETSOCKOPT_PARAMS));
	_farpokex (SocketP, sizeof (WSOCK_GETSOCKOPT_PARAMS), optval, *optlen);

	__wsock_callvxd (WSOCK_GETSOCKOPT_CMD);

	if (_VXDError && _VXDError != 0xffff) {
		/* We've handled it & it failed. */
		*rv = -1;
		return(1);
	}

	_farpeekx (SocketP, 0, &params, sizeof (WSOCK_GETSOCKOPT_PARAMS));

	/* The return value may be returned using Value & ValueLength, or it
	 * may be present in the params structure as IntValue. In the latter
	 * case, both Value & ValueLength will be set to zero. */
	if ((params.Value == NULL) && (params.ValueLength == 0)) is_int = 1;

	if (!is_int)
		*optlen = params.ValueLength;
	else
		*optlen = sizeof(params.IntValue);

	switch (optname) {
	case SO_ERROR:
		/* This error needs converting to a BSD-style error. */
		if (!is_int) {
			_farpeekx (SocketP, sizeof (WSOCK_GETSOCKOPT_PARAMS),
				   optval, *optlen);		
			*(int *) optval = __wsock_errno(*(int *) optval);
		} else {
			*(int *) optval = __wsock_errno(params.IntValue);
		}
		break;

	default:
		if (!is_int) {
			_farpeekx (SocketP, sizeof (WSOCK_GETSOCKOPT_PARAMS),
				   optval, *optlen);
		} else {
			*(int *) optval = params.IntValue;
		}
		break;
	}

	/* Call handled and succeeded. */
	*rv = 0;
	return(1);
}

/* ----------------------
 * - __wsock_setsockopt -
 * ---------------------- */

int __wsock_setsockopt (LSCK_SOCKET *lsd, int *rv,
			int level, int optname,
			const void *optval, size_t optlen)
{
	LSCK_SOCKET_WSOCK *wsock = (LSCK_SOCKET_WSOCK *) lsd->idata;
	WSOCK_SETSOCKOPT_PARAMS params;
	int a_optlen = optlen;
	void *a_optval = (void *) optval;

	switch (optname) {
	/* Catch the non-supported BSD options. WSOCK.VXD doesn't seem to
	 * return error messages for some of these, in spite of the fact
	 * they're not supported by Winsock (source: Win32 API help file)! */
	case SO_ACCEPTCONN:
	case SO_RCVLOWAT:
	case SO_RCVTIMEO:
	case SO_SNDLOWAT:
	case SO_SNDTIMEO:
	case SO_TYPE:
		errno = ENOPROTOOPT;
		*rv = -1;
		return(1);
		break;

	default:
		a_optval = (void *) optval;
		a_optlen = optlen;
		break;
	}

	/* RD: Check that the buffer won't overflow! */
	if ((a_optlen + sizeof (WSOCK_SETSOCKOPT_PARAMS)) > _SocketP.size) {
		errno = ENOBUFS;
		*rv   = -1;
		return(1);
	}

	bzero(&params, sizeof(params));
	params.Value = (void *) (  (SocketP << 16)
				 + sizeof (WSOCK_SETSOCKOPT_PARAMS));
	params.Socket = (void *) wsock->_Socket;
	params.OptionLevel = level;
	params.OptionName = optname;
	params.ValueLength = a_optlen;
	params.IntValue = 0;

	_farpokex (SocketP, 0, &params, sizeof (WSOCK_SETSOCKOPT_PARAMS));
	_farpokex (SocketP, sizeof (WSOCK_SETSOCKOPT_PARAMS),
		   a_optval, a_optlen);

	__wsock_callvxd (WSOCK_SETSOCKOPT_CMD);

	if (_VXDError && _VXDError != 0xffff)
		*rv = -1;
	else
		*rv = 0;
	
	return(1);
}
