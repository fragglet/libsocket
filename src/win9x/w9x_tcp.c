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
 * w9x_tcp.c
 * 
 * This file was written by Richard Dawe.
 * 
 * ---
 * 
 * Description:
 * 
 * This file obtains key TCP/IP settings from the registry using Alfons
 * Hoogervorst's registry-access code. This allows functions such as
 * getdomainname and the resolving library to work.
 * 
 * Please note that functions that return char *'s use strdup to allocate
 * memory, so any code using these functions should free up this memory after
 * usage.
 */

/* Include some files */
#include <stdlib.h>
#include <string.h>

#include <lsck/win9x.h>
#include <lsck/registry.h>

char *__win9x_mstcp_getkeystr (char *name);

/* Save some typing - this is the root key for all the settings that we need */
#define MSTCP_KEY "System\\CurrentControlSet\\Services\\VxD\\MSTCP"

/* ----------------------------
 * - __win9x_mstcp_dnsenabled -
 * ---------------------------- */

/* If the DNS is enabled, 1 is returned. If it isn't, 0 is returned.
 * On error, -1 is returned. */

int __win9x_mstcp_dnsenabled (void)
{
    int ret;
    char *p = NULL;

    /* Get the EnableDNS key under the MSTCP one */
    p = __win9x_mstcp_getkeystr ("EnableDNS");

    /* If there is no key, assume it's OK to use the DNS */
    if (p == NULL)
	return (1);

    /* Else convert to a string and return */
    ret = atoi (p);
    free (p);
    return (ret);
}

/* -----------------------------
 * - __win9x_mstcp_getdnsaddrs -
 * ----------------------------- */

/* This function will return the IP addresses for the DNS. On error NULL is
 * returned. */

char **__win9x_mstcp_getdnsaddrs (void)
{
    HKEY hkey = NULL;		/* IM: for warnings */
    DWORD keytype = REG_SZ;
    char buffer[256];
    char *q;			/* Temp. pointers */
    char **mstcp_dnsaddrs;	/* MSTCP DNS addresses */
    DWORD buffersize = sizeof (buffer);
    DWORD ret;
    int nodns;			/* No. of DNS's */
    int i;

    /* Check that the DNS is enabled */
    if (__win9x_mstcp_dnsenabled () != 1)
	return (NULL);

    /* Get the MSTCP registry key handle */
    ret = RegOpenKey (HKEY_LOCAL_MACHINE, MSTCP_KEY, &hkey);	/* IM: added & before hkey */
    if (ret != ERROR_SUCCESS)
	return (NULL);

    /* Get the NameServer subkey */
    ret = RegQueryValueEx (hkey, "NameServer", NULL, &keytype, (LPBYTE) buffer, &buffersize);
    RegCloseKey (hkey);

    if (ret != ERROR_SUCCESS)
	return (NULL);

    /* Build a list of strings */
    q = buffer;
    nodns = 0;
    while ((q = strchr (q, ',')) != NULL) {
	nodns++;
	q++;
    }
    nodns++;			       /* One more DNS than commas */

    mstcp_dnsaddrs = malloc ((nodns + 1) * sizeof (char *));

    if (mstcp_dnsaddrs == NULL)
	return (NULL);
    memset (mstcp_dnsaddrs, 0, (nodns + 1) * sizeof (char *));

    /* The DNS addresses are stored in a comma separated list, so unseparate
     * them & copy into mstcp_dnsaddrs */
    q = strtok (buffer, ",");
    mstcp_dnsaddrs[0] = strdup (q);

    for (i = 1;
	 (q = strtok (NULL, ",")) != NULL;
	 mstcp_dnsaddrs[i] = strdup (q), i++) {;
    }

    /* Finito */
    return (mstcp_dnsaddrs);
}

/* ---------------------------
 * - __win9x_mstcp_getkeystr -
 * --------------------------- */

/* This returns the string stored under a MSTCP key with the specified
 * 'name'. */

char *__win9x_mstcp_getkeystr (char *name)
{
    HKEY hkey = NULL;		/* IM: for warnings */
    DWORD keytype = REG_SZ;
    char buffer[256];
    DWORD buffersize = sizeof (buffer);
    DWORD ret;

    /* Get the MSTCP registry key handle */
    ret = RegOpenKey (HKEY_LOCAL_MACHINE, MSTCP_KEY, &hkey);	/* IM: Added & before hkey */
    if (ret != ERROR_SUCCESS)
	return (NULL);

    /* Get the NameServer subkey */
    ret = RegQueryValueEx (hkey, name, NULL, &keytype, (LPBYTE) buffer, &buffersize);
    RegCloseKey (hkey);

    if (ret != ERROR_SUCCESS)
	return (NULL);
    else
	return (strdup (buffer));
}

/* -----------------------------
 * - __win9x_mstcp_gethostname -
 * ----------------------------- */

/* This function finds the host name from the MSTCP subkey, if possible. */
char *__win9x_mstcp_gethostname (void)
{
    return (__win9x_mstcp_getkeystr ("HostName"));
}

/* -------------------------------
 * - __win9x_mstcp_getdomainname -
 * ------------------------------- */

/* This function finds the domain name from the MSTCP subkey, if possible. */
char *__win9x_mstcp_getdomainname (void)
{
    return (__win9x_mstcp_getkeystr ("Domain"));
}
