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
    dnsaddr.c

    Description: This code obtains the DNS server's IP address. It abstracts
    this process so that the calling code doesn't have to know which transport
    is being used.
*/

/* Include files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lsck/dnsaddr.h>
#include <lsck/mstcp.h>
#include <lsck/ras.h>
#include <lsck/vdhcp.h>
#include <lsck/ini.h>

/* -------------------
   - lsck_getdnsaddr -
   ------------------- */

char *lsck_getdnsaddr (void)
{
    char **dnsaddrs = NULL, *dnsaddr = NULL;
    char **p;

    /* Get all addresses, but only return first */
    p = dnsaddrs = lsck_getdnsaddrs();
    if (dnsaddrs == NULL) return(NULL);

    /* This is the only one we want */
    dnsaddr = *dnsaddrs;

    /* Free all the unused memory */
    p++;
    while (*p != NULL) { free(*p), p++; }
    free(dnsaddrs);

    /* Return the IP address */
    return( dnsaddr );
}

/* -------------
   - strinlist -
   ------------- */

/* This function checks to see if the string 'str' is anywhere in 'list', e.g.
   for finding duplicate entries. */

int strinlist (char **list, char *str)
{
    int i;

    for (i = 0; list[i] != NULL; i++) {
        /* Found match */
        if (strcmp(list[i], str) == 0) return(1);
    }

    /* No match */
    return(0);
}

/* --------------------
   - lsck_getdnsaddrs -
   -------------------- */

/* The DNS address can be obtained via the MSTCP registry keys, from the RAS
   settings or from the DHCP VxD (courtesy of Alfons Hoogervorst's
   information). The MSTCP DNS addresses are assumed to have higher priority
   than the RAS and DHCP ones (i.e. local has higher priority than remote).
   Note that the returned list needs its elements free'ing once they've been
   finished with. The list will only have each address once it (i.e. duplicates
   entries are eliminated). NULL is returned if the call fails. */

char **lsck_getdnsaddrs (void)
{
    char **mstcp_dnsaddrs     = NULL;
    char **ras_dnsaddrs       = NULL;
    char **vdhcp_dnsaddrs     = NULL;
    char **systemini_dnsaddrs = NULL;
    char **dnsaddrs           = NULL;
    int i, count = 0;

    /* Get all the DNS addresses and combine the two lists */
    mstcp_dnsaddrs     = mstcp_getdnsaddrs();
    ras_dnsaddrs       = ras_getdnsaddrs();
    vdhcp_dnsaddrs     = vdhcp_getdnsaddrs();
    systemini_dnsaddrs = systemini_getdnsaddrs();

    if (mstcp_dnsaddrs != NULL)
        for (i = 0; mstcp_dnsaddrs[i] != NULL; i++, count++) {;}

    if (ras_dnsaddrs != NULL)
        for (i = 0; ras_dnsaddrs[i] != NULL; i++, count++) {;}

    if (vdhcp_dnsaddrs != NULL)
        for (i = 0; vdhcp_dnsaddrs[i] != NULL; i++, count++) {;}

    if (systemini_dnsaddrs != NULL)
        for (i = 0; systemini_dnsaddrs[i] != NULL; i++, count++) {;}

    dnsaddrs = malloc((count + 1) * sizeof(char *));
    if (dnsaddrs == NULL) return(NULL);                 /* malloc failed */
    memset(dnsaddrs, 0, (count + 1) * sizeof(char *));  /* NULL pointers */

    count = 0;

    if (mstcp_dnsaddrs != NULL)
        for (i = 0; mstcp_dnsaddrs[i] != NULL; i++) {
            if (!strinlist(dnsaddrs, mstcp_dnsaddrs[i])) {
                dnsaddrs[count] = mstcp_dnsaddrs[i];
                count++;
            }
        }

    if (ras_dnsaddrs != NULL)
        for (i = 0; ras_dnsaddrs[i] != NULL; i++) {
            if (!strinlist(dnsaddrs, ras_dnsaddrs[i])) {
                dnsaddrs[count] = ras_dnsaddrs[i];
                count++;
            }
        }

    if (vdhcp_dnsaddrs != NULL)
        for (i = 0; vdhcp_dnsaddrs[i] != NULL; i++) {
            if (!strinlist(dnsaddrs, vdhcp_dnsaddrs[i])) {
                dnsaddrs[count] = vdhcp_dnsaddrs[i];
                count++;
            }
        }

    if (systemini_dnsaddrs != NULL)
        for (i = 0; systemini_dnsaddrs[i] != NULL; i++) {
            if (!strinlist(dnsaddrs, systemini_dnsaddrs[i])) {
                dnsaddrs[count] = systemini_dnsaddrs[i];
                count++;
            }
        }

    /* Free some memory */
    free(mstcp_dnsaddrs);
    free(ras_dnsaddrs);
    free(vdhcp_dnsaddrs);
    free(systemini_dnsaddrs);

    /* Done */
    return(dnsaddrs);
}
