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
 * Original code from Dan Hedlund's wsock library.
 */

#include <lsck/vxd.h>

/* RD: Modified the assembly code slightly: "pushl %%es" -> "pushw %%es" &
 * used xorl ... instead of movl $0, ... . Made it volatile too. */

/* ---------------
 * - VxdGetEntry -
 * --------------- */

void VxdGetEntry (int *Entry, int ID)
{
  __asm__ __volatile__ ("pushw   %%es            \n\
                         movw    %%di, %%es      \n\
                         int     $0x2f           \n\
                         xorl    %%ecx, %%ecx    \n\
                         movw    %%es, %%cx      \n\
                         popw    %%es"
			  : "=c" (Entry[1]), "=D" (Entry[0])
			  : "a" (0x1684), "b" (ID), "D" (0)
			  : "cc"
    );
}
