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
 * w9x_ras.c
 * 
 * This file was written by Richard Dawe.
 * 
 * ---
 * 
 * Description:
 * 
 * This file obtains key network transport settings from the registry
 * using the RegDos Group's registry-access code. The keys dealt with are from
 * the keys:
 * 
 * . HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services\RemoteAccess
 * . HKEY_CURRENT_USER\RemoteAccess
 * 
 * The information used to write this code was supplied by Alfons Hoogervorst.
 * See the file ipdata.txt that should be in <library root>/misc/docs/.
 * 
 * Please note that functions that return char *'s use strdup to allocate
 * memory, so any code using these functions should free up this memory after
 * usage.
 */

/* Include some files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <lsck/registry.h>
#include <lsck/win9x.h>

/* Save some typing - this is the root key for all the settings that we need */
#define RAS_KEY    "System\\CurrentControlSet\\Services\\RemoteAccess"
#define RAS_USERKEY "RemoteAccess" /* Pointless; off HKEY_CURRENT_USER btw */

/* According to MS SDK headers - used for maximum registry key length */
#ifndef MAX_PATH
#define MAX_PATH 260
#endif

/* ----------------------
 * - __win9x_ras_active -
 * ---------------------- */

/* This returns 1 if there is an active dial-up connection. */

int __win9x_ras_active (void)
{
    HKEY hkey = NULL;
    DWORD valuetype = REG_DWORD;
    DWORD value = 0;
    DWORD valuesize = sizeof (value);
    DWORD ret;

    /* Open the RAS key */
    ret = RegOpenKey (HKEY_LOCAL_MACHINE, RAS_KEY, &hkey);
    if (ret != ERROR_SUCCESS)
	return (0);

    /* Get the value */
    ret = RegQueryValueEx (hkey, "Remote Connection", NULL,
			   &valuetype, (LPSTR) & value, &valuesize);
    if (ret != ERROR_SUCCESS)
	value = 0;		       /* Just to be safe, set value */

    /* Close the key */
    RegCloseKey (hkey);

    /* Return the value */
    return (value);
}

/* -----------------------------------
 * - __win9x_ras_getdefaultphonebook -
 * ----------------------------------- */

/* This returns the name of the default phonebook from
 * HKEY_CURRENT_USER\RemoteAccess\Default. This is used by ras_getdata()
 * below. */

char *__win9x_ras_getdefaultphonebook (void)
{
    HKEY hkey = NULL;
    DWORD buffertype = REG_SZ;
    char buffer[MAX_PATH];
    DWORD buffersize = sizeof (buffer);
    DWORD ret;

    /* Open the key */
    ret = RegOpenKey (HKEY_CURRENT_USER, "RemoteAccess", &hkey);
    if (ret != ERROR_SUCCESS)
	return (NULL);

    /* Get the phonebook name */
    ret = RegQueryValueEx (hkey, "Default", NULL, &buffertype, buffer, &buffersize);
    if (ret != ERROR_SUCCESS)
	return (NULL);

    /* Return it */
    return (strdup (buffer));
}

/* -----------------------
 * - __win9x_ras_getdata -
 * ----------------------- */

/* This parses the data stored in
 * HKEY_CURRENT_USER\RemoteAccess\Profile\<Default phonebook name>. It fills
 * the passed structure with the parsed data. If successful, 1 is returned,
 * else 0. It determines whether the local IP address is fixed (& what it is)
 * and whether DNS addresses were given (& what they are). */

int __win9x_ras_getdata (RAS_DATA * rd)
{
    char *pbname = NULL;
    HKEY hkey = NULL;
    DWORD buffertype = REG_BINARY;
    char buffer[50];		/* 50 bytes is the maximum data size */
    DWORD buffersize = sizeof (buffer);
    DWORD ret;
    long convaddr;		/* Used for IP addr. conversion      */

    /* Duh! No RAS connection active! */
    if (!__win9x_ras_active ())
	return (0);

    /* Get the default phonebook name */
    pbname = __win9x_ras_getdefaultphonebook ();
    if (pbname == NULL)
	return (0);		       /* None! */

    /* Open the key and get the data */
    ret = RegOpenKey (HKEY_CURRENT_USER, "RemoteAccess\\Profile", &hkey);
    if (ret != ERROR_SUCCESS)
	return (0);

    ret = RegQueryValueEx (hkey, pbname, NULL, &buffertype, buffer, &buffersize);
    if (ret != ERROR_SUCCESS)
	return (0);

    /* See what IP data is present */
    memset (rd, 0, sizeof (RAS_DATA));

    if (buffer[0x04] & 0x01)
	rd->ip_fixed = 1;	       /* Local IP addr. */
    if (buffer[0x04] & 0x02)
	rd->ip_dns_specified = 1;      /* DNS IP addr's  */

    if (rd->ip_fixed) {
	memcpy (&convaddr, buffer + 0x08, 4);	/* IP addr. = 4-byte ulong */
	rd->ip_machine.s_addr = htons (convaddr);
    }
    if (rd->ip_dns_specified) {
	/* Hmm...dangerous code? */
	memcpy (&convaddr, buffer + 0x0C, 4);
	if ((rd->ip_dns[rd->ip_dns_count].s_addr = htons (convaddr)) != 0)
	    rd->ip_dns_count++;

	memcpy (&convaddr, buffer + 0x10, 4);
	if ((rd->ip_dns[rd->ip_dns_count].s_addr = htons (convaddr)) != 0)
	    rd->ip_dns_count++;
    }
    /* OK */
    return (1);
}

/* ---------------------------
 * - __win9x_ras_getdnsaddrs -
 * --------------------------- */

char **__win9x_ras_getdnsaddrs (void)
{
    char **p = NULL;
    RAS_DATA rd;
    int i;

    /* Get the RAS data */
    if (!__win9x_ras_getdata (&rd))
	return (NULL);

    /* Allocate pointers first */
    p = malloc ((rd.ip_dns_count + 1) * sizeof (char *));

    if (p == NULL)
	return (NULL);
    memset (p, 0, (rd.ip_dns_count + 1) * sizeof (char *));

    /* Fill with DNS IP addresses */
    for (i = 0; i < rd.ip_dns_count; i++) {
	p[i] = strdup (inet_ntoa (rd.ip_dns[i]));
    }

    p[rd.ip_dns_count + 1] = NULL;

    return (p);
}
