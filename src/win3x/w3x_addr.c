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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <lsck/if.h>
#include <lsck/win3x.h>

/* ------------------------
 * - __win3x_getaddrtable -
 * ------------------------ */

LSCK_IF_ADDR_TABLE *__win3x_getaddrtable (void)
{
    LSCK_IF_ADDR_TABLE *addr = NULL;
    LSCK_IF_ADDR_INET *lo = NULL;
    LSCK_IF_ADDR_INET if_build;
    int if_count = 0;
    char *p = NULL;
    char **q = NULL;
    char **r = NULL;
    int i, j;

    /* Get the loopback device info */
    lo = __lsck_if_addr_inet_loopback ();

    /* Set up the address table */
    addr = (LSCK_IF_ADDR_TABLE *) malloc (sizeof (LSCK_IF_ADDR_TABLE));
    if (addr == NULL) {
	free (lo);
	return (NULL);
    }
    for (i = 0; i < (LSCK_IF_ADDR_TABLE_MAX + 1); i++) {
	addr->table.inet[i] = NULL;
    }

    addr->family = AF_INET;
    addr->table.inet[if_count] = lo;
    if_count++;

    /* Enumerate all local interfaces */
    q = __win3x_systemini_getinterfaces ();

    if (q == NULL) {
	/* No interfaces => fail */
	free (lo);
	free (addr);
	return (NULL);
    }
    for (i = 0; (p = q[i]) != NULL; i++) {
	bzero (&if_build, sizeof (if_build));

	/* Default interface details */
	if_build.fixed = 1;
	if_build.hw_type = LSCK_IF_HW_TYPE_ETHERNET;
	if_build.hw_addrlen = 6;       /* TODO: Find Ethernet address too */

	/* IP address & network mask */
	if (!__win3x_systemini_getipaddr (p, &if_build.addr))
	    continue;
	if (!__win3x_systemini_getnetmask (p, &if_build.netmask))
	    continue;

	/* Gateway */
	if (__win3x_systemini_getgwaddr (p, &if_build.gw))
	    if_build.gw_present = 1;
	else
	    if_build.gw_present = 0;

	/* DNS addresses */
	if (!__win3x_systemini_getdnsaddr (p, 1, &if_build.dns[0])) {
	    /* Use global DNS addresses */
	    r = __win3x_systemini_getdnsaddrs ();
	    if (r != NULL) {
		for (j = 0; j < LSCK_IF_ADDR_INET_DNS_MAX; j++) {
		    if (!inet_aton (r[j], &if_build.dns[j]))
			break;
		}

		/* Free the DNS address list */
		for (j = 0; r[j] != NULL; j++) {
		    free (r[j]);
		}
		free (r);
	    }
	} else {
	    /* Get all DNS addresses */
	    for (j = 0; j < LSCK_IF_ADDR_INET_DNS_MAX; j++) {
		if (!__win3x_systemini_getdnsaddr (p, j + 1, &if_build.dns[j]))
		    break;
	    }
	}

	/* Host & domain names */
	/* NB: p doesn't point to q[i] now! */
	p = __win3x_systemini_gethostname (); /* NB: Actually node name! */
	strcpy (if_build.hostname, p);
	free (p);

	p = __win3x_systemini_getdomainname ();
	strcpy (if_build.domainname, p);
	free (p);

	if (strchr(if_build.hostname, '.') == 0) {
            if (  (strlen(if_build.hostname) + 1 + strlen(if_build.domainname))
		< MAXGETHOSTNAME) {
	        strcat(if_build.hostname, ".");
	        strcat(if_build.hostname, if_build.domainname);
	    }
        }

	/* Done with this interface - update and free */
	memcpy (addr->table.inet[if_count], &if_build, sizeof (if_build));
	if_count++;
    }

    /* Free the interface list */
    for (i = 0; q[i] != NULL; i++) {
	free (q[i]);
    }
    free (q);

    /* Done */
    return (addr);
}
