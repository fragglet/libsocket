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
 * One day I discovered that _farp*x functions are buggy, so I replaced them...
 */

#ifndef ___farptrx_h___
#define ___farptrx_h___

#include <sys/movedata.h>
#include <sys/segments.h>

#define _farpokex(sel,off,dat,len) movedata(_my_ds(),(int)dat,sel,off,len)
#define _farpeekx(sel,off,dat,len) movedata(sel,off,_my_ds(),(int)dat,len)

#endif
