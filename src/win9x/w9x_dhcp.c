/*
 *  libsocket - BSD socket like library for DJGPP
 *  Copyright (C) 1997, 1998 by Indrek Mandre
 *  Copyright (C) 1997-2000 by Richard Dawe
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
 * w9x_dhcp.c
 * Windows '9x DHCP settings for network cards, written by Richard Dawe.
 */

/*
 * NOTES:
 * 
 * - The registry byte values are in network order.
 * 
 * - Gateway address obtained - is it really the gateway address, or does
 *   the name "LastGateWay" mean something else? Later: LastGateway is
 *   something different - the router option in the vendor-specific data
 *   appears to contain the gateway, but winipcfg doesn't see this! If the
 *   gateway table is disabled globally for TCP/IP then the DHCP gateway
 *   address is used, but is placed in "LastGateway".
 * 
 * TODO:
 * 
 * - Make __win9x_dhcp_gethostname() and __win9x_dhcp_getdomainname() use a
 *   function that retrieves strings for the vendor info -> easier to add other
 *   options later.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <lsck/win9x.h>
#include <lsck/registry.h>

#define DHCP_INFO_KEY "System\\CurrentControlSet\\Services\\VxD\\DHCP"

/* ----------------------------
 * - __win9x_dhcp_getkeyvalue -
 * ---------------------------- */

/* This returns the string stored under in the nth DHCP info key with the
 * specified name. */

char *__win9x_dhcp_getkeyvalue (int n, char *valuename, int *valuesize)
{
    HKEY hkey = NULL, hsubkey = NULL;
    char subkey[256];
    DWORD keytype = REG_BINARY;
    char buffer[256];
    DWORD buffersize = sizeof (buffer);
    DWORD ret;
    unsigned long valid;
    DWORD validsize = sizeof (valid);
    char *p = NULL;

    /* Build the subkey name */
    sprintf (subkey, "DhcpInfo%02i", n);

    /* Get the DHCP info registry key handle */
    ret = RegOpenKey (HKEY_LOCAL_MACHINE, DHCP_INFO_KEY, &hkey);
    if (ret != ERROR_SUCCESS)
	return (NULL);

    /* Get the value called 'name' for this subkey of the info key */
    ret = RegOpenKey (hkey, subkey, &hsubkey);
    if (ret != ERROR_SUCCESS) {
	RegCloseKey (hkey);
	return (NULL);
    }
    /* Check for valid DHCP info subkey */
    ret = RegQueryValueEx (hsubkey, "DhcpIndex", NULL, &keytype,
			   (LPBYTE) & valid, &validsize);

    if (ret != ERROR_SUCCESS) {
	RegCloseKey (hkey);
	return (NULL);
    }
    if (valid == 0xFFFFFFFF) {
	RegCloseKey (hsubkey);
	RegCloseKey (hkey);
	return (NULL);
    }
    /* Get the value */
    ret = RegQueryValueEx (hsubkey, valuename, NULL, &keytype,
			   (LPBYTE) & buffer, &buffersize);
    if (ret == ERROR_SUCCESS) {
	p = (char *) malloc (buffersize);
	if (p != NULL) {
	    memcpy (p, buffer, buffersize);
	    *valuesize = buffersize;
	}
    }
    RegCloseKey (hsubkey);
    RegCloseKey (hkey);
    return (p);
}

/* ---------------------------
 * - __win9x_dhcp_getkeyaddr -
 * --------------------------- */

int __win9x_dhcp_getkeyaddr (int n, char *valuename, struct in_addr *addr)
{
    int valuesize;
    char *p = __win9x_dhcp_getkeyvalue (n, valuename, &valuesize);

    if (p == NULL)
	return (0);
    if (valuesize != sizeof (struct in_addr)) {
	free (p);
	return (0);
    }
    memcpy (addr, p, valuesize);
    free (p);
    return (1);
}

/* --------------------------
 * - __win9x_dhcp_getipaddr -
 * -------------------------- */

int __win9x_dhcp_getipaddr (int n, struct in_addr *addr)
{
    int ret = __win9x_dhcp_getkeyaddr (n, "DhcpIPAddress", addr);

    if (ret && (addr->s_addr == 0))
	return (0);
    else
	return (ret);
}

/* ---------------------------
 * - __win9x_dhcp_getnetmask -
 * --------------------------- */

int __win9x_dhcp_getnetmask (int n, struct in_addr *addr)
{
    int ret = __win9x_dhcp_getkeyaddr (n, "DhcpSubnetMask", addr);

    if (ret && (addr->s_addr == 0))
	return (0);
    else
	return (ret);
}

/* --------------------------
 * - __win9x_dhcp_gethwaddr -
 * -------------------------- */

int __win9x_dhcp_gethwaddr (int n, char *hw_addr, int *hw_addrlen)
{
    int valuesize;
    char *p;

    p = __win9x_dhcp_getkeyvalue (n, "HardwareAddress", &valuesize);
    if (p == NULL)
	return (0);

    if (*hw_addrlen < valuesize) {
	free (p);
	return (0);
    }
    memcpy (hw_addr, p, valuesize);
    *hw_addrlen = valuesize;
    free (p);
    return (1);
}

/* ----------------------------
 * - __win9x_dhcp_getoptaddrs -
 * ---------------------------- */

int __win9x_dhcp_getoptaddrs (int opt, int n, struct in_addr *ip, int ip_max)
{
    int valuesize;
    int addrlen;
    char *p;
    int i, j;

    /* Get the value */
    p = __win9x_dhcp_getkeyvalue (n, "OptionInfo", &valuesize);
    if (p == NULL)
	return (0);

    /*
     * Now convert the IP addresses. The format of vendor extensions
     * with IP addresses is:
     * 
     * <option> <length> <4 bytes of IP address> ... <4 bytes of IP address>
     * Number of IP addresses = length / 4
     */
    for (i = 0, addrlen = 0; i < valuesize; i++) {
	if (p[i] == opt) {
	    addrlen = p[i + 1];
	    break;
	}
    }

    /* No addresses present */
    if (addrlen == 0) {
	free (p);
	return (0);
    }
    /* Non-quad address len */
#define CHECK_LEN (sizeof(struct in_addr))
    if ((CHECK_LEN * addrlen / CHECK_LEN) != addrlen) {
	free (p);
	return (0);
    }
    for (i += 2, j = 0;
	 (addrlen > 0) && (j < ip_max);
	 i += 4, addrlen -= 4, j++) {
	memcpy (&ip[j].s_addr, (const void *) &p[i], 4);
    }

    return (1);
}

/* ----------------------------
 * - __win9x_dhcp_getdnsaddrs -
 * ---------------------------- */

int __win9x_dhcp_getdnsaddrs (int n, struct in_addr *dns, int dns_max)
{
  return(__win9x_dhcp_getoptaddrs(DHCP_OPT_DNS, n, dns, dns_max));
}

/* --------------------------
 * - __win9x_dhcp_getgwaddr -
 * -------------------------- */

int __win9x_dhcp_getgwaddr (int n, struct in_addr *addr)
{
  struct in_addr dummy[1] = { { 0 } };
  int ret;

  /* Retrieve only one router (aka gateway) address. */
  /* TODO: Allow more than one gateway to be returned. This isn't really that
   * important, since we're doing this just for fun (well, information). */
  ret = __win9x_dhcp_getoptaddrs(DHCP_OPT_ROUTER, n, dummy, 1);
  if (ret)
    *addr = dummy[0];

  return(ret);
}

/* ----------------------------
 * - __win9x_dhcp_gethostname -
 * ---------------------------- */

int __win9x_dhcp_gethostname (int n, char *buffer, int *bufferlen)
{
    int valuesize, namelen;
    char *p;
    int i;

    /* Get the value */
    p = __win9x_dhcp_getkeyvalue (n, "OptionInfo", &valuesize);
    if (p == NULL)
	return (0);

    /* Now convert the addresses. The format of the DNS vendor extension is:
     * 
     * 0x0F <length> <nul terminated domain name>
     */
    for (i = 0; i < valuesize; i++) {
	if (p[i] == DHCP_OPT_HOST_NAME) {
	    namelen = p[i + 1];
	    if (namelen <= 0)
		break;

	    /* Copy the domain name */
	    p[i + 2 + namelen] = '\0'; /* Terminate it anyway */
	    if (namelen < *bufferlen)
		*bufferlen = namelen;
	    strncpy (buffer, &p[i + 2], *bufferlen);
	    p[*bufferlen - 1] = '\0';
	    break;
	}
    }

    free (p);
    return (1);
}

/* ------------------------------
 * - __win9x_dhcp_getdomainname -
 * ------------------------------ */

int __win9x_dhcp_getdomainname (int n, char *buffer, int *bufferlen)
{
    int valuesize, namelen;
    char *p;
    int i;

    /* Get the value */
    p = __win9x_dhcp_getkeyvalue (n, "OptionInfo", &valuesize);
    if (p == NULL)
	return (0);

    /* Now convert the addresses. The format of the DNS vendor extension is:
     * 
     * 0x0F <length> <nul terminated domain name>
     */
    for (i = 0; i < valuesize; i++) {
	if (p[i] == DHCP_OPT_DOMAIN_NAME) {
	    namelen = p[i + 1];
	    if (namelen <= 0)
		break;

	    /* Copy the domain name */
	    p[i + 2 + namelen] = '\0'; /* Terminate it anyway */
	    if (namelen < *bufferlen)
		*bufferlen = namelen;
	    strncpy (buffer, &p[i + 2], *bufferlen);
	    p[*bufferlen - 1] = '\0';
	    break;
	}
    }

    free (p);
    return (1);
}
