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
 * CallVXD functions in this C file taken from Dan Hedlund's wsock library.
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include <dpmi.h>
#include <sys/farptr.h>
#include <sys/segments.h>

#include "wsock.h"
#include "farptrx.h"

int VXDError (int wsock_errno);

int _VXDError;
extern int WSockEntry[2];

/* -------------------
 * - __wsock_callvxd -
 * ------------------- */

void __wsock_callvxd (int func)
{
    __asm__ __volatile__ ("pushw   %%es                     \n\
                           pushw   %%cx                     \n\
                           popw    %%es                     \n\
                          "
#if    (GAS_MAJOR == 2) \
    && ((GAS_MINOR < 9) || ((GAS_MINOR == 9) && (GAS_MINORMINOR < 5)))
                          "lcall   _WSockEntry              \n\
                          "
#else
			  "lcall   *_WSockEntry              \n\
                          "
#endif			  
                          "andl    $0x0000ffff, %%eax       \n\
                           popw    %%es"
			  :"=a" (_VXDError)
			  :"a" (func), "b" (0), "c" (SocketP)
    );

    if (_VXDError == 0xffff)
	errno = EAGAIN;
    if (_VXDError && (_VXDError != 0xffff))
	errno = __wsock_errno (_VXDError);
}

/* --------------------
 * - __wsock_callvxd2 -
 * -------------------- */

void __wsock_callvxd2 (int func, int sel)
{
    __asm__ __volatile__ ("pushw   %%es                     \n\
                           pushw   %%cx                     \n\
                           popw    %%es                     \n\
                          "
#if    (GAS_MAJOR == 2) \
    && ((GAS_MINOR < 9) || ((GAS_MINOR == 9) && (GAS_MINORMINOR < 5)))
                          "lcall   _WSockEntry              \n\
                          "
#else
			  "lcall   *_WSockEntry              \n\
                          "
#endif			  
                          "andl    $0x0000ffff, %%eax       \n\
                           popw    %%es"
			  :"=a" (_VXDError)
			  :"a" (func), "b" (0), "c" (sel)
    );

    if (_VXDError == 0xffff)
	errno = EAGAIN;
    if (_VXDError && (_VXDError != 0xffff))
	errno = __wsock_errno (_VXDError);
}
