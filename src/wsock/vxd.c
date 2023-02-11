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
 * Original code from Dan Hedlund's wsock library.
 */

#include <lsck/vxd.h>

void VxdGetEntry (int *Entry, int ID)
{
  asm ("pushl   %%es            \n\
        movw    %%di, %%es      \n\
        intb    $0x2f           \n\
        movl    $0, %%ecx       \n\
        movw    %%es, %%cx      \n\
        popl    %%es"
        : "=c" (Entry [1]), "=D" (Entry [0])
        : "a" (0x1684), "b" (ID), "D" (0)
        : "%eax", "%edx");
}

