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

/*
    fsext.c

    File system extensions for treating sockets like ordinary file descriptors.
    Moved here by Rich Dawe from Indrek Mandre's socket.c.
*/                     

#include <sys/fsext.h>
#include <io.h>
#include <fcntl.h>
#include <unistd.h>

#include <lsck/lsck.h>
#include <lsck/ws.h>
#include <winsock.h>

#include "lsckglob.h"

/* RD: If this returns 1, it informs DJGPP that it emulated the required
   file system function. The actual return value is stored in rv. */

int sock_functions(__FSEXT_Fnumber func_number, int *rv, va_list args)
{
    int s;

    if (!lsck_init()) {
        /* Unable to initialise */
        errno = ENODEV;
        *rv = -1;
        return(1);
    }


    switch ( func_number ) {
        case __FSEXT_read:
	{
	     int s;
	     void *data;
	     int count;
	     s = va_arg ( args, int );
	     data = va_arg ( args, void* );
	     count = va_arg ( args, int );

         /* RD: Non-blocking I/O should return 0 if no bytes were read, and
            set errno to EAGAIN! */
         *rv = recv ( s, data, count, 0 );

         if ((*rv == -1) && (errno == EWOULDBLOCK)) {
            *rv = 0;
            errno = EAGAIN;
         }

	     return 1;
	}
        case __FSEXT_write:
	{
	     int s;
	     void *data;
	     int count;
	     s = va_arg ( args, int );
	     data = va_arg ( args, void* );
	     count = va_arg ( args, int );

         /* RD: Try and fix write'ing in the same way as read'ing - I don't
            know whether send() via WSOCK.VXD will ever return the blocking
            I/O error, but this should cope with it. */
         *rv = send ( s, data, count, 0 );

         if ((*rv == -1) && (errno = EWOULDBLOCK)) {
            *rv = 0;
            errno = EAGAIN;
         }

	     return 1;
	}
        case __FSEXT_ready:
        {
             int s;
             int t;
             int r = 0;
             s = va_arg ( args, int );
             t = selectsocket ( s, FD_READ|FD_ACCEPT ) ;
             if ( t > 0 ) r |= __FSEXT_ready_read;
               else if ( t < 0 ) r |= __FSEXT_ready_error;

             t = selectsocket ( s, FD_WRITE );
             if ( t > 0 ) r |= __FSEXT_ready_write;
               else if ( t < 0 ) r |= __FSEXT_ready_error;
             *rv = r;
             /*return r?r:8;*/ /* IM: Must I here return the value? */
             /* RD: No, just non-zero, so the FS extensions know we handled
                it. */
            return(1);
        }
        case __FSEXT_close:
             s = va_arg ( args, int );
             *rv = closesocket ( s );
	     __FSEXT_set_function ( s, NULL );
	     _close ( s );
             return 1;
	case __FSEXT_ioctl:
	{
	     int s;
	     int req;
	     int *comm;
	     s = va_arg ( args, int );
	     req = va_arg ( args, int );
	     comm = va_arg ( args, int* );
             *rv = ioctlsocket ( s, req, comm );
             return 1;
	}
	case __FSEXT_fcntl:
	{
	     int s;
	     int command;
	     int request;
	     s = va_arg ( args, int );
	     command = va_arg ( args, int );

         if (command == F_SETFL) {
            /* Set flags */
            request = va_arg ( args, int );
            *rv = fcntlsocket(s, command, request);
         } else if (command == F_GETFL) {
            /* Get flags */
            *rv = fcntlsocket(s, command, 0);
         } else {
            /* Unsupported command */
            *rv = -1;
         }

	     return 1;
	}
	default:
	     *rv = -1;
	     return 1;		/* That's better now */
 }

 /* This function hasn't handled the call */
 return 0;
}
