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
 * Original code from Dan Hedlund's wsock library, modified by Indrek Mandre
 * (IM) and Richard Dawe (RD).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/farptr.h>
#include <sys/segments.h>
#include <dpmi.h>
#include <signal.h>
#include <pc.h>
#include <errno.h>

#include <lsck/vxd.h>
#include <lsck/ws.h>
#include <winsock.h>

#include "wsockvxd.h"
#include "lsckglob.h"

int WSockEntry[2];
int WSockEntry2[2];     /* RD: Only used to check if Winsock 2 is present */

/* Used to allocate DPMI memory */
__dpmi_meminfo _SocketP;
__dpmi_meminfo _SocketD;

/* RD: DPMI selectors */
int SocketP;    /* Winsock parameters */
int SocketD;    /* Winsock data (used as well as param's in some func's */

int wsock_initialized = 0;  /* RD: Moved here from socket.c */

/* --------------
   - wsock_init -
   -------------- */

/* RD: wsock_init now returns 1 on success, 0 on failure. If it's previously
   init'd OK, then it returns 1. */

int wsock_init (void)
{    
    /* RD: Done this before, so it's OK */
    if ( wsock_initialized ) return(1); 

    /* RD: Load Winsock VxDs - try loading Winsock 2 to see if we have
           Winsock 2 */
    VxdLdrLoadDevice ("WSOCK.VXD");
    VxdLdrLoadDevice ("WSOCK.386");
    VxdLdrLoadDevice ("WSOCK2.VXD");

    VxdGetEntry (WSockEntry, 0x003E);
    VxdGetEntry (WSockEntry2, 0x3B0A);

    /* RD: Check the WSOCK.VXD entry point is valid - 0:0 = invalid, others OK */
    if ( (WSockEntry[0] == 0) && (WSockEntry[1] == 0) ) {
        errno = ENODEV; /* "No such device" */
        return(0);
    }

    /* RD: If the WSOCK2.VXD entry point is valid then return, as the code
           currently doesn't work with Winsock 2's WSOCK.VXD */
    if ( (WSockEntry2[0] != 0) || (WSockEntry2[1] != 0) ) {
        errno = ENOSYS;
        return(0);
    }

    /* RD: Modify _Socket[P|D|C].size at your peril! *Most* of the library
       looks at the size, but I've inadvertently introduced bugs by fiddling
       with these before. */

    _SocketP.handle = 0;
    _SocketP.size = 4096;               /* RD: 65536 is big */
    _SocketP.address = 0;

    if (__dpmi_allocate_memory (&_SocketP) == -1) return(0); /* RD: failed */

    _SocketD.handle = 0;
    _SocketD.size = 65536;              /* RD: 65536 is big. Later: No, OK! */
    _SocketD.address = 0;

    if (__dpmi_allocate_memory (&_SocketD) == -1) return(0);  /* RD: failed */

    SocketP = __dpmi_allocate_ldt_descriptors(1);
    SocketD = __dpmi_allocate_ldt_descriptors(1);

    /* RD: Descriptors allocated OK? */
    if ( (SocketP == -1) || (SocketD == -1) ) return(0);

    __dpmi_set_segment_base_address (SocketP, _SocketP.address);
    __dpmi_set_segment_base_address (SocketD, _SocketD.address);

    __dpmi_set_segment_limit (SocketP, _SocketP.size);
    __dpmi_set_segment_limit (SocketD, _SocketD.size);

    /* RD: Set this now we've done everything successfully */
    wsock_initialized = 1;

    /* Success */
    return(1);
}

/* -----------------
   - wsock_uninit2 -
   ----------------- */

void wsock_uninit2 (void)
{
    if (!wsock_initialized) return;
    wsock_initialized = 0;

    /* Free DPMI memory */
    __dpmi_free_memory(_SocketP.handle);
    __dpmi_free_memory(_SocketD.handle);

    /* Free descriptors */
    __dpmi_free_ldt_descriptor(SocketP);
    __dpmi_free_ldt_descriptor(SocketD);

    VxdLdrUnLoadDevice ("WSOCK.VXD");
    VxdLdrUnLoadDevice ("WSOCK.386");

    /* RD: Winsock 2 too! */
    VxdLdrUnLoadDevice ("WSOCK2.VXD");
}

/* ----------------
   - wsock_uninit -
   ---------------- */

void wsock_uninit (void)
{
    if (!wsock_initialized) return;
    wsock_initialized = 0;

    /* Free DPMI memory */
    __dpmi_free_memory(_SocketP.handle);
    __dpmi_free_memory(_SocketD.handle);

    /* Free descriptors */
    __dpmi_free_ldt_descriptor(SocketP);
    __dpmi_free_ldt_descriptor(SocketD);
}

