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

#ifndef __libsocket_glob_h__
#define __libsocket_glob_h__

#include <dpmi.h>

#include <lsck/if.h>

extern int WSockEntry[2];

/* RD: P, D, C mean: parameters, data, code */
extern int SocketP;
extern int SocketD;
/*extern int SocketC;*/             /* RD: Commented out now */

extern __dpmi_meminfo _SocketP;     /* RD: init_net.c only */
extern __dpmi_meminfo _SocketD;     /* RD: init_net.c only */
/*extern __dpmi_meminfo _SocketC;*/ /* RD: init_net.c only */

/*extern int WSockDataSel;*/        /* RD: init_net.c only */
/*extern int CallBack;*/            /* RD: init_net.c only */
/*extern int VxdLdrEntry[2];*/      /* RD: vxdldr.c only */
/*extern int WS_initialized;*/      /* RD: Superceeded by WS_init()'s return */

extern int _VXDError;

/* RD: This is declared as wsocket in socket.c, but "struct socket" and
   "wsocket" *are* equivalent. */
/*extern struct socket *WSdescr[256];*/
#ifndef LSCK_MAX_SOCKET
#define LSCK_MAX_SOCKET 256
#endif

extern LSCK_SOCKET *lsckDescriptor[LSCK_MAX_SOCKET];

void CallVxD ( int );
void VXDError ( void );

#endif  /* __libsocket_glob_h__ */
