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

/*
 * Access to WsControl(), to obtain info about IP interfaces.
 * Written by Richard Dawe
 * 
 * THIS CODE IS EXPERIMENTAL! USE AT YOUR OWN RISK! Currently it seems to
 * corrupt the stack (i.e. leave the stack pointer not pointer to the right
 * place), which leads to weird errors when you're debugging.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dpmi.h>
#include <sys/farptr.h>
#include <sys/segments.h>

#include <sys/socket.h>

#include <lsck/if.h>
#include "wsock.h"
#include "farptrx.h"
#include "wsockvxd.h"

typedef struct {
    long Number;		/* Interface number, WS_INTERFACE_TCPIP for TCPIP */
    long Unknown;		/* Seems to be used to differentiate between
				 * multiple TCPIP interfaces. */
} WS_INTERFACE __attribute__ ((packed));

typedef struct {
    WS_INTERFACE Interface;	/* Interfaces to query???      */
    long What;			/* Changes for each request    */
    long Unknown;		/* Seems to be always 0x100??? */
    long Command;		/* Obviously a command         */
    unsigned char Unknown1[16];	/* Unknown (Always 0???)       */
} WS_IN_PARAMS __attribute__ ((packed));

#define WS_INTERFACE_TCPIP  0x0301

#define WSC_GET_INFO    0
#define WSC_SET_INFO    1	       /* RD: Maybe */

int __wsock_control (char *inbuf, int inbuflen, char *outbuf, int outbuflen,
		     int proto, int action);

int __wsock_control_test (void)
{
    WS_IN_PARAMS inbuf;
    char outbuf[256];
    int ret;

    bzero (&inbuf, sizeof (inbuf));
    inbuf.Interface.Number = WS_INTERFACE_TCPIP;
    inbuf.Interface.Unknown = 0;
    inbuf.What = 0x100;
    inbuf.Unknown = 0x100;
    inbuf.Command = 0;

    ret = __wsock_control ((char *) &inbuf, sizeof (inbuf),
			   outbuf, sizeof (outbuf),
			   IPPROTO_TCP, WSC_GET_INFO);

    return (ret);
}

int __wsock_control (char *inbuf, int inbuflen, char *outbuf, int outbuflen,
		     int proto, int action)
{
    WSOCK_CONTROL_PARAMS params;
    int ret;

    /* RD: Check the length of the data - only transmit up to the size of the
     * buffer */
    if ((inbuflen + outbuflen) > _SocketD.size) {
	errno = ENOBUFS;
	return (-1);
    }
    bzero (&params, sizeof (params));

    params.InputBuffer = (void *) (SocketD << 16);
    params.OutputBuffer = (void *) ((SocketD + inbuflen) << 16);
    params.InputBufferLength = inbuflen;
    params.OutputBufferLength = outbuflen;
    params.Protocol = proto;
    params.Action = action;

    _farpokex (SocketP, 0, &params, sizeof (WSOCK_CONTROL_PARAMS));
    _farpokex (SocketD, 0, inbuf, inbuflen);
/*_farpokex(SocketD, inbuflen, outbuf,  outbuflen);*/

    __wsock_callvxd (WSOCK_CONTROL_CMD);

    /*if ( _VXDError && _VXDError != 0xffff ) return -1; */

    _farpeekx (SocketP, 0, &params, sizeof (WSOCK_CONTROL_PARAMS));
    _farpeekx (SocketD, 0, inbuf, inbuflen);
    _farpeekx (SocketD, inbuflen, outbuf, outbuflen);

    /* I think the number of paragraphs copied is stored in the output
     * buffer in the first DWORD. */
    ret = *(long *) outbuf;

    return (ret);		       /* TODO: retval? */
}
