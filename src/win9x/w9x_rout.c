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
 * route.c
 * Construct a routing table from the IP data.
 */

/*
 * TODO:
 * 
 * - Add/remove entries from routing table?
 * - Metrics?
 * - Multicast?
 * 
 * Mostly this if for faking Win3x, Win9x's routing tables, but it will be
 * more useful when other interfaces have been added, e.g. packet drivers.
 */

#include <string.h>

#include <netinet/in.h>

#include <lsck/if.h>

/* ------------------------
 * - __ip_getstaticroutes -
 * ------------------------ */

LSCK_IF_ROUTE_TABLE *__ip_getstaticroutes (LSCK_IF_ADDR_TABLE * addr)
{
    LSCK_IF_ADDR_INET **inet = addr->table.inet;

    int if_addrcount = 0;
    int gw_present = 0;
    int gw_added = 0;
    struct in_addr gw;
    int i, j;

    /* Count the available IP interfaces  & check for a gateway */
    bzero (&gw, sizeof (gw));

    for (i = if_addrcount = gw_present = 0;
	 inet[i] != NULL;
	 i++, if_addrcount++) {
	if (inet[i]->gw.s_addr != 0) {
	    gw.s_addr = inet[i]->gw.s_addr;
	    gw_present = 1;
	}
    }

    /* Calculate number of entries needed */
}
