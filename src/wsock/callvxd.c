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
 * CallVXD functions in this C file taken from Dan Hedlund's wsock library.
 */

#include <sys/farptr.h>
#include <sys/segments.h>
#include <stdio.h>
#include <dpmi.h>
#include <pc.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include <lsck/wsock.h>
#include <lsck/ws.h>
#include <winsock.h>

#include "lsckglob.h"
#include "farptrx.h"

int _VXDError;
extern LSCK_WSOCK_ERROR WSA_ERRORS[];

void CallVxD (int func)
{
    asm volatile ("pushl   %%es                     \n\
                   pushl   %%ecx                    \n\
                   popl    %%es                     \n\
                   lcall   _WSockEntry              \n\
                   andl    %%eax, 0x0000ffff        \n\
                   popl    %%es"
                   : "=a" (_VXDError)
                   : "a" (func), "b" (0), "c" (SocketP));

    /* andl    0x0000ffff, %%eax       \n\ */

    if ( _VXDError == 0xffff ) errno = EAGAIN;
    if (_VXDError && (_VXDError != 0xffff)) VXDError ();
}

void VXDError (void)
{
  int a;

  for (a = 0; (WSA_ERRORS [a].ErrorNum != _VXDError) && (WSA_ERRORS [a].ErrorNum != -1); ++ a);

  errno = WSA_ERRORS[a].error;

/*  printf ("%s\r\n", WSA_ERRORS [a].Error);
			IM: no need for that, we're not so alpha anymore */
}
