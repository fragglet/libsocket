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
 * w3x_sysi.c
 * 
 * System.ini networking information was provided by Ove Kaaven.
 * This file was written by Richard Dawe.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <lsck/win3x.h>
#include <lsck/ini.h>
#include <lsck/hostname.h>
#include <lsck/domname.h>

/* ---------------------------------
 * - __win3x_systemini_getdnsaddrs -
 * --------------------------------- */

/* This uses the GetPrivateProfileString to read in DNSServers from the DNS
 * section of system.ini, i.e. settings like:
 * 
 * [DNS]
 * DNSServers=192.168.0.2,192.168.0.3
 * 
 * The IP address are returned as an array of IP address strings.
 */

char **__win3x_systemini_getdnsaddrs (void)
{
    char buf[256];
    char systemini_filename[FILENAME_MAX], *p = NULL, **addrs = NULL;
    int no_addrs, i;

    /* Build system.ini's name */
    *systemini_filename = '\0';

    p = getenv ("windir");
    if (p != NULL)
	sprintf (systemini_filename, "%s/", p);
    strcat (systemini_filename, "system.ini");

    /* If the call fails, then return NULL */
    if (GetPrivateProfileString ("DNS", "DNSServers", NULL, buf, sizeof (buf),
				 systemini_filename) == 0)
	return (NULL);

    if (*buf == '\0')
	return (NULL);

    /* Count the number of addresses */
    for (i = 0, no_addrs = 1; i < strlen (buf); i++) {
	if (buf[i] == ',')
	    no_addrs++;
    }

    /* No addresses found */
    if (no_addrs == 0)
	return (NULL);

    /* Create array */
    addrs = (char **) malloc ((no_addrs + 1) * sizeof (char *));

    if (addrs == NULL)
	return (NULL);

    /* Copy addresses into it */
    for (i = 0; i < no_addrs + 1; i++) {
	addrs[i] = NULL;
    }
    addrs[0] = strtok (buf, ",");
    for (i = 1; i < no_addrs; i++) {
	addrs[i] = strtok (NULL, ",");
    }

    return (addrs);
}

/* -----------------------------------
 * - __win3x_systemini_getinterfaces -
 * ----------------------------------- */

/* This looks in the 'MSTCP' section for a key called 'Interfaces' to obtain
 * the available network interfaces. These are returned in a NULL-terminated
 * list of strings. */

char **__win3x_systemini_getinterfaces (void)
{
    char buf[256];
    char systemini_filename[FILENAME_MAX];
    char *p = NULL, **masks = NULL;
    int no_masks, i;

    /* Build system.ini's name */
    *systemini_filename = '\0';

    p = getenv ("windir");
    if (p != NULL)
	sprintf (systemini_filename, "%s/", p);
    strcat (systemini_filename, "system.ini");

    /* Get the contents of 'Interfaces'. */
    if (GetPrivateProfileString ("MSTCP", "Interfaces", NULL, buf, sizeof (buf),
				 systemini_filename) == 0)
	return (NULL);

    /* Count the number of interfaces available */
    if (strtok (buf, "=") == NULL)
	return (NULL);
    for (no_masks = 0; strtok (NULL, ",") != NULL; no_masks++) ;

    /* Create a list long enough */
    masks = (char **) malloc (sizeof (char *) * (no_masks + 1));

    if (masks == NULL)
	return (NULL);
    for (i = 0; i <= no_masks; i++) {
	masks[i] = NULL;
    }

    /* Now actually add the keys to the list */
    strtok (buf, "=");
    for (i = 0; (p = strtok (NULL, ",")) != NULL; i++) {
	masks[i] = strdup (p);
    }
    masks[i] = strdup (p);

    return (masks);
}

/* --------------------------------------
 * - __win3x_systemini_getinterfaceaddr -
 * -------------------------------------- */

int __win3x_systemini_getinterfaceaddr (char *interface, char *keyname,
					struct in_addr *addr)
{
    char buf[256];
    char systemini_filename[FILENAME_MAX];
    char *p = NULL;

    /* Build system.ini's name */
    *systemini_filename = '\0';

    p = getenv ("windir");
    if (p != NULL)
	sprintf (systemini_filename, "%s/", p);
    strcat (systemini_filename, "system.ini");

    /* Get the desired key */
    if (GetPrivateProfileString (interface, keyname, NULL, buf, sizeof (buf),
				 systemini_filename) == 0)
	return (0);

    /* Convert to IP address from string */
    if (inet_aton (buf, addr) == 0)
	return (0);		       /* Failed */
    return (1);
}

/* Convenience functions */
int __win3x_systemini_getipaddr (char *interface, struct in_addr *addr)
{
    int ret = __win3x_systemini_getinterfaceaddr (interface, "IPAddress", addr);

    if (ret && (addr->s_addr == 0))
	return (0);
    else
	return (ret);
}

int __win3x_systemini_getnetmask (char *interface, struct in_addr *addr)
{
    int ret = __win3x_systemini_getinterfaceaddr (interface, "IPMask", addr);

    if (ret && (addr->s_addr == 0))
	return (0);
    else
	return (ret);
}

int __win3x_systemini_getgwaddr (char *interface, struct in_addr *addr)
{
    int ret = __win3x_systemini_getinterfaceaddr (interface, "DefaultGateway",
						  addr);

    if (ret && (addr->s_addr == 0))
	return (0);
    else
	return (ret);
}

int __win3x_systemini_getdnsaddr (char *interface, int n, struct in_addr *addr)
{
    char buf[12];		/* 'NameServer' + digit + nul */

    /* Only 1-9 allowed */
    if ((n < 1) || (n > 9))
	return (0);

    /* Get the DNS server IP address */
    sprintf (buf, "NameServer%i", n);
    return (__win3x_systemini_getinterfaceaddr (interface, buf, addr));
}

/* ---------------------------------
 * - __win3x_systemini_gethostname -
 * --------------------------------- */

char *__win3x_systemini_gethostname (void)
{
    char buf[MAXGETHOSTNAME];
    char systemini_filename[FILENAME_MAX];
    char *p = NULL;

    /* Build system.ini's name */
    *systemini_filename = '\0';

    p = getenv ("windir");
    if (p != NULL)
	sprintf (systemini_filename, "%s/", p);
    strcat (systemini_filename, "system.ini");

    /* Get the contents of 'HostName'. */
    if (GetPrivateProfileString ("DNS", "HostName", NULL, buf, sizeof (buf),
				 systemini_filename) == 0)
	return (NULL);

    /* OK */
    return (strdup (buf));
}

/* -----------------------------------
 * - __win3x_systemini_getdomainname -
 * ----------------------------------- */

char *__win3x_systemini_getdomainname (void)
{
    char buf[MAXGETDOMAINNAME];
    char systemini_filename[FILENAME_MAX];
    char *p = NULL;

    /* Build system.ini's name */
    *systemini_filename = '\0';

    p = getenv ("windir");
    if (p != NULL)
	sprintf (systemini_filename, "%s/", p);
    strcat (systemini_filename, "system.ini");

    /* Get the contents of 'DomainName'. */
    if (GetPrivateProfileString ("DNS", "DomainName", NULL, buf, sizeof (buf),
				 systemini_filename) == 0)
	return (NULL);

    /* OK */
    return (strdup (buf));
}
