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
 * w9x_addr.c
 * Find networking information from Win9x's registry.
 */

/*
 * TODO:
 * 
 * - Test with non-MS stacks?
 * - Check type returned on RegQueryValueEx() calls
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <lsck/if.h>
#include <lsck/registry.h>
#include <lsck/win9x.h>

/* --- Defines --- */

/* Max length of registry key names */
#ifndef MAX_PATH
#define MAX_PATH 260
#endif

/* The IP address that implies DHCP is being used. */
#define DHCP_IP "0.0.0.0"

/* Some keys used to find out the IP interface type, i.e. dial-up or Ethernet.
 * These may be highly MS-stack dependent. Note that the *_KEY defines are
 * all off HKEY_LOCAL_MACHINE or HKLM for short. */

#define ROOT_KEY	"Enum\\Root"
#define ISAPNP_KEY      "Enum\\ISAPNP"
#define PCI_KEY		"Enum\\PCI"
#define NETWORK_KEY	"Enum\\Network"
#define CLASS_KEY	"System\\CurrentControlSet\\Services\\Class"

#define NDI_IF_SUBKEY	"Ndi\\Interfaces"

/* MAC "address" for dial-up connections. This is empirical, obtained from
 * winipcfg. This is used to distinguish Ethernet from dial-up DHCP
 * addresses. The last two digits appear to the index of the dial-up
 * device used, i.e 0x0 0x0, 0x0 0x1, ... */
static char DIALUP_MAC[6] =
{0x44, 0x45, 0x53, 0x54, 0x0, 0x0};

#define DIALUP_DESC       "Dial-Up Adapter"
#define DIALUP_ADAPTER    "MS$PPP"
#define DIALUP_VXD        "pppmac.vxd"
#define DIALUP_IF_LOWER   "vcomm"

#define ETHERNET_IF_LOWER "ethernet"

/* Functions */
static int __win9x_enuminterfaces (LSCK_IF_ADDR_TABLE *addr,
				   char *key,
				   int *if_count,
				   int *dialup_count,
				   int *dhcp_count);

static int __win9x_getethcardaddrs (LSCK_IF_ADDR_INET *tif,
				    HKEY hkNet, HKEY hkMSTCP, HKEY hkNetTrans,
				    int *dhcp_count);

static int __win9x_getstaticaddr (HKEY hkNetTrans, LSCK_IF_ADDR_INET *tif);

static int __win9x_getdynamicaddr (HKEY hkNetTrans, int dhcp_num,
				   LSCK_IF_ADDR_INET *tif);

/* ------------------------
 * - __win9x_getaddrtable -
 * ------------------------ */

LSCK_IF_ADDR_TABLE *__win9x_getaddrtable (void)
{	
	/* Interface table building variables */
	LSCK_IF_ADDR_INET *lo = NULL; /* Loopback device - always present. */
	LSCK_IF_ADDR_TABLE *addr = NULL;
	int if_count = 0;
	int dialup_count = -1;	/* 1st index = 0 */
	int dhcp_count = -1;	/* 1st index = 0 */
	int i;

	/* Get the loopback device info */
	lo = __lsck_if_addr_inet_loopback();

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

	/* Enumerate all local devices of the "Net" class off
	 * HKLM\Enum\Root. If RegOpenKey() fails, just skip getting the IP
	 * address data. */
	__win9x_enuminterfaces(addr, ROOT_KEY,
			       &if_count, &dialup_count, &dhcp_count);
	
	__win9x_enuminterfaces(addr, ISAPNP_KEY,
			       &if_count, &dialup_count, &dhcp_count);

	__win9x_enuminterfaces(addr, PCI_KEY,
			       &if_count, &dialup_count, &dhcp_count);

	return(addr);
}

/* --------------------------
 * - __win9x_enuminterfaces -
 * -------------------------- */

static int __win9x_enuminterfaces (LSCK_IF_ADDR_TABLE *addr,
				   char *key,
				   int *if_count,
				   int *dialup_count,
				   int *dhcp_count)
{
	/* Registry key variables */
	HKEY hkBase = NULL;
	HKEY hkDev = NULL;
	HKEY hkEnum = NULL;
	HKEY hkBindings = NULL;
	HKEY hkClass = NULL;
	HKEY hkNetwork = NULL;
	HKEY hkMSTCP = NULL;
	HKEY hkNet = NULL;
	HKEY hkNetTrans = NULL;
	HKEY hkInterface = NULL;
	long int res = 0;
	char devname[256];	/* Device name in HKLM\Enum\Root tree */
	char enumname[256];	/* Instance of device in HKLM\Enum\Root\<dev>
				 * tree */
	char valname[256];
	DWORD valname_len;
	DWORD val_type;
	DWORD buf_type;	
	char buf[256];
	DWORD buf_len;
	LSCK_IF_ADDR_INET if_build;
	LSCK_IF_ADDR_INET *tif = NULL;
	char *p, *q;
	long int i, j, k;

	/* Open the base key */
	res = RegOpenKey(HKEY_LOCAL_MACHINE, key, &hkBase);
	
	for (i = 0; res == ERROR_SUCCESS; i++) {
		res = RegEnumKey(hkBase, i, devname, sizeof(devname));
		if (res != ERROR_SUCCESS) break;

		/* Now enumerate all instances of this device, to find one of
		 * the "Net" class. */
		res = RegOpenKey(hkBase, devname, &hkDev);
		if (res != ERROR_SUCCESS) break;

		for (j = 0; res == ERROR_SUCCESS; j++) {
			res = RegEnumKey(hkDev, j, enumname, sizeof(enumname));
			if (res != ERROR_SUCCESS) break;

			res = RegOpenKey(hkDev, enumname, &hkEnum);
			if (res != ERROR_SUCCESS) break;

			buf_len = sizeof(buf);
			res = RegQueryValueEx(hkEnum, "Class", NULL,
					      &buf_type, buf, &buf_len);
			if (res != ERROR_SUCCESS) {
				RegCloseKey(hkEnum);
				hkEnum = NULL;

				/* Some keys may not have a Class entry,
				 * but this is not an error. Continue after
				 * this error. */
				if (res == ERROR_CANTREAD)
				  res = ERROR_SUCCESS;
				break;
			}

			if (strcmp(buf, "Net") != 0) {
				RegCloseKey(hkEnum);
				hkEnum = NULL;
				continue;
			}

			/* This is a network device. Its bindings show which
			 * MSTCP key to use, to determine its NetTrans key.
			 * They also show which Net key to use, to determine
			 * what type of device it is. */

			/* No room for interface data! */
			if (!(*if_count < LSCK_IF_ADDR_TABLE_MAX)) {
				RegCloseKey(hkEnum);
				hkEnum = NULL;
				break;
			}

			/* Get the Net key. */
			buf_len = sizeof(buf);
			res = RegQueryValueEx(hkEnum, "Driver", NULL,
					      &buf_type, buf, &buf_len);
			if (res != ERROR_SUCCESS) {
				RegCloseKey(hkEnum);
				hkEnum = NULL;
				break;
			}

			res = RegOpenKey(HKEY_LOCAL_MACHINE, CLASS_KEY,
					 &hkClass);
			if (res != ERROR_SUCCESS) {
				RegCloseKey(hkEnum);
				hkEnum = NULL;
				break;
			}

			res = RegOpenKey(hkClass, buf, &hkNet);
			if (res != ERROR_SUCCESS) {
				RegCloseKey(hkEnum);
				hkEnum = NULL;
				break;
			}
			
			/* Enumerate the bindings. */
			res = RegOpenKey(hkEnum, "Bindings", &hkBindings);
			if (res != ERROR_SUCCESS) {
				RegCloseKey(hkEnum);
				RegCloseKey(hkNet);
				hkEnum = hkNet = NULL;
				break;
			}

			for (k = 0; res == ERROR_SUCCESS; k++) {
				valname_len = sizeof(valname);
				buf_len = sizeof(buf);
				res = RegEnumValue(hkBindings, k,
						   valname, &valname_len,
						   NULL, &val_type,
						   buf, &buf_len);
				if (res != ERROR_SUCCESS) break;

				/* nul-terminate the value's name */
				valname[valname_len] = '\0';
				
				/* Is this a binding to MSTCP? If not, next! */
				if (strstr(valname, "MSTCP\\") != valname)
					continue;

				/* Open the network key. */
				res = RegOpenKey(HKEY_LOCAL_MACHINE,
						 NETWORK_KEY,
						 &hkNetwork);
				if (res != ERROR_SUCCESS) {
					RegCloseKey(hkNetwork);
					hkNetwork = NULL;
					break;
				}
				
				/* Open the MSTCP key for later. */
				res = RegOpenKey(hkNetwork, valname, &hkMSTCP);

				RegCloseKey(hkNetwork);
				hkNetwork = NULL;
				
				if (res == ERROR_SUCCESS)
					break;
				else
					hkMSTCP = NULL;
			}

			RegCloseKey(hkBindings);			
			hkBindings = NULL;			
			
			if (hkMSTCP == NULL) {
				/* No MSTCP binding found => no good. */
				RegCloseKey(hkNet);
				hkNet = NULL;
				continue;
			}			

			/* Open the NetTrans key using the info obtained from
			 * the MSTCP key. */
			buf_len = sizeof(buf);
			res = RegQueryValueEx(hkMSTCP, "Driver", NULL,
					      &buf_type, buf, &buf_len);
			if (res != ERROR_SUCCESS) {
				RegCloseKey(hkClass);
				RegCloseKey(hkNet);
				RegCloseKey(hkMSTCP);
				RegCloseKey(hkEnum);
				hkClass = hkNet = hkMSTCP = hkEnum = NULL;
				break;
			}
			
			res = RegOpenKey(hkClass, buf, &hkNetTrans);
			if (res != ERROR_SUCCESS) {
				RegCloseKey(hkClass);
				RegCloseKey(hkNet);
				RegCloseKey(hkMSTCP);
				RegCloseKey(hkEnum);
				hkClass = hkNet = hkMSTCP = hkEnum = NULL;
				break;
			}

			RegCloseKey(hkClass);
			hkClass = NULL;

			/* Now interpret the MSTCP, Net and NetTrans keys to
			 * obtain the IP address data. */
			bzero(&if_build, sizeof(if_build));

			/* - Step 1: Interface type - */

			/* TODO: Do all dial-up adapters have a description
			 * "Dial-Up Adapter"? Maybe the PPTP adapter has
			 * something different - check by installing
			 * DUN 1.3. */
			
			/* Get the driver description. */
			buf_len = sizeof(buf);
			res = RegQueryValueEx(hkNet, "DriverDesc", NULL,
					      &buf_type, buf, &buf_len);
			if (res != ERROR_SUCCESS) {
				RegCloseKey(hkNet);
				RegCloseKey(hkMSTCP);
				RegCloseKey(hkNetTrans);
				RegCloseKey(hkEnum);
				hkNet = hkMSTCP = hkNetTrans = hkEnum = NULL;
				break;
			}

			p = strdup(buf);

			/* Get the NDIS driver's lower interface binding. */
			res = RegOpenKey(hkNet, NDI_IF_SUBKEY, &hkInterface);
			if (res != ERROR_SUCCESS) {
				RegCloseKey(hkNet);
				RegCloseKey(hkMSTCP);
				RegCloseKey(hkNetTrans);
				RegCloseKey(hkEnum);
				hkNet = hkMSTCP = hkNetTrans = hkEnum = NULL;
				break;
			}

			buf_len = sizeof(buf);
			res = RegQueryValueEx(hkInterface, "Lower", NULL,
					      &buf_type, buf, &buf_len);
			if (res != ERROR_SUCCESS) {
				RegCloseKey(hkInterface);
				RegCloseKey(hkNet);
				RegCloseKey(hkMSTCP);
				RegCloseKey(hkNetTrans);
				RegCloseKey(hkEnum);
				hkInterface = hkNet = hkMSTCP = hkNetTrans
					= hkEnum = NULL;
				break;
			}

			RegCloseKey(hkInterface);
			hkInterface = NULL;
			q = strdup(buf);

			if (   (strcmp(p, DIALUP_DESC) == 0)
			    && (strcmp(q, DIALUP_IF_LOWER) == 0)) {
				/* Dial-up interface */
				free(p);
				free(q);

				(*dialup_count)++;

				/* TODO: Which dial-up adapter is not active?
				 * BUG BUG BUG! */
				if (!__win9x_ras_active()) {
					/* Not active => no IP address. */
					RegCloseKey(hkNet);
					RegCloseKey(hkMSTCP);
					RegCloseKey(hkNetTrans);
					RegCloseKey(hkEnum);
					hkNet = hkMSTCP = hkNetTrans
						= hkEnum = NULL;
					continue;
				}

				/* Store the hardware details. */
				if_build.hw_type = LSCK_IF_HW_TYPE_DIALUP;
				memcpy (if_build.hw_addr,
					DIALUP_MAC, sizeof (DIALUP_MAC));
				if_build.hw_addrlen = sizeof (DIALUP_MAC);
				*((unsigned short *) &if_build.hw_addr[4])
					= (unsigned short) *dialup_count;
			}

			if (strcmp(q, ETHERNET_IF_LOWER) == 0) {
				/* Ethernet */
				free(p);
				free(q);

				if_build.hw_type = LSCK_IF_HW_TYPE_ETHERNET;
			}
			
			/* Step 2: Address information */
			switch(if_build.hw_type) {
			default:
				/* Er? */
				break;
				
			case LSCK_IF_HW_TYPE_DIALUP:
				/* TODO: How to get RAS IP details? */
				break;

			case LSCK_IF_HW_TYPE_ETHERNET:
				__win9x_getethcardaddrs(&if_build,
							hkNet,
							hkMSTCP,
							hkNetTrans,
							dhcp_count);
				break;
			}
			
			/* Step 3: Add to list */			
			tif =
				(LSCK_IF_ADDR_INET *)
				malloc (sizeof (LSCK_IF_ADDR_INET));

			if (tif != NULL) {
				addr->table.inet[*if_count] = tif;
				memcpy(tif, &if_build, sizeof(if_build));
				(*if_count)++;
				tif = NULL;
			}	    
			
			/* Close all keys. */
			RegCloseKey(hkNet);
			RegCloseKey(hkMSTCP);
			RegCloseKey(hkNetTrans);
			hkNet = hkMSTCP = hkNetTrans = NULL;
			
			RegCloseKey(hkEnum);
			hkEnum = NULL;
		}

		/* End of enum => still successful */
		if (res == ERROR_NO_MORE_ITEMS) res = ERROR_SUCCESS;
		
		RegCloseKey(hkDev);
		hkDev = NULL;
	}

	RegCloseKey(hkBase);
	hkBase = NULL;

	/* TODO: Make this return 0 when no addresses found. */
	return(1);
}

/* -------------------------
 * - __win9x_getstaticaddr -
 * ------------------------- */

static int __win9x_getstaticaddr (HKEY hkNetTrans, LSCK_IF_ADDR_INET * tif)
{
	long int res;
	DWORD buf_type;
	char buf[256];
	DWORD buf_len;
	char *p, **r;
	int i;

	/* IP address */
	buf_len = sizeof(buf);
	res = RegQueryValueEx(hkNetTrans, "IPAddress", NULL,
			      &buf_type, buf, &buf_len);
	if (res != ERROR_SUCCESS) return(0);

	if (inet_aton(buf, &tif->addr) == -1) return(0);
	
	/* Address mask */
	buf_len = sizeof(buf);
	res = RegQueryValueEx(hkNetTrans, "IPMask", NULL,
			      &buf_type, buf, &buf_len);
	if (res != ERROR_SUCCESS) return(0);

	if (inet_aton(buf, &tif->netmask) == -1) return (0);

	/* Gateway - This may or may not be present, so don't fail here. */
	/* TODO: Allow multiple gateways */
	buf_len = sizeof(buf);
	res = RegQueryValueEx(hkNetTrans, "DefaultGateway", NULL,
			      &buf_type, buf, &buf_len);

	/* TODO: Should probably check for other error values too. */
	if (res == ERROR_SUCCESS) {		
		p = strchr (buf, ',');
		if (p != NULL) *p = '\0';

		if (inet_aton(buf, &tif->gw) == -1) return (0);
		tif->gw_present = 1;
	}
	
	/* Host name, domain name, DNS addresses are globally config'd */
	p = __win9x_mstcp_gethostname (); /* NB: Node name! */
	if (p != NULL) {
		strncpy (tif->hostname, p, MAXGETHOSTNAME);
		tif->hostname[MAXGETHOSTNAME - 1] = '\0';
		free (p);
	}

	p = __win9x_mstcp_getdomainname ();
	if (p != NULL) {
		strncpy (tif->domainname, p, MAXGETHOSTNAME);
		tif->domainname[MAXGETHOSTNAME - 1] = '\0';
		free (p);
	}

	/* Append the domain name to the node ("host") name, if necessary */
	if (   (strchr(tif->hostname, '.') == 0)
	       && (strlen(tif->domainname)    != 0) ) {
		if (  (strlen(tif->hostname) + 1 + strlen(tif->domainname))
		      < MAXGETHOSTNAME) {
			strcat(tif->hostname, "."); 
			strcat(tif->hostname, tif->domainname); 
		}
	}

	r = __win9x_mstcp_getdnsaddrs ();

	if (r != NULL) {
		for (i = 0;
		     (r[i] != NULL) && (i < LSCK_IF_ADDR_INET_DNS_MAX);
		     i++) {
			if (inet_aton (r[i], &tif->dns[i]) == -1)
				break;
		}
		
		for (i = 0; r[i] != NULL; i++) { free (r[i]); }
		free (r);
	}

	tif->fixed = 1;
	
	/* Set hardware info up. */
	tif->hw_type = LSCK_IF_HW_TYPE_ETHERNET;	
	
	/* Success */
	return (1);
}

/* --------------------------
 * - __win9x_getdynamicaddr -
 * -------------------------- */

static int __win9x_getdynamicaddr (HKEY hkNetTrans, int dhcp_num,
				   LSCK_IF_ADDR_INET * tif)
{
	long int res;
	DWORD buf_type;
	char buf[256];
	DWORD buf_len;
	int valuesize;
	char *p;

	if (!__win9x_dhcp_getipaddr(dhcp_num, &tif->addr)) return (0);
	if (!__win9x_dhcp_getnetmask(dhcp_num, &tif->netmask)) return (0);

	/* Gateway - even if DHCP hasn't given us a gateway address, it may
	 * have been set through the TCP/IP control panel. */
	if (__win9x_dhcp_getgwaddr (dhcp_num, &tif->gw)) tif->gw_present = 1;

	if (!tif->gw_present) {
		/* TODO: Allow multiple gateways */
		buf_len = sizeof(buf);
		res = RegQueryValueEx(hkNetTrans, "DefaultGateway", NULL,
				      &buf_type, buf, &buf_len);

		/* TODO: Should probably check for other error values too. */
		if (res == ERROR_SUCCESS) {		
			p = strchr (buf, ',');
			if (p != NULL) *p = '\0';

			if (inet_aton(buf, &tif->gw) == -1) return (0);
			tif->gw_present = 1;
		}
	}
	
	/* Host name, domain name, DNS */
	valuesize = MAXGETHOSTNAME;
	__win9x_dhcp_gethostname (dhcp_num, tif->hostname, &valuesize);
	valuesize = MAXGETHOSTNAME;
	__win9x_dhcp_getdomainname (dhcp_num, tif->domainname, &valuesize);

	/* Append the domain name to the node ("host") name, if necessary */
	if (   (strchr(tif->hostname, '.') == 0)
	    && (strlen(tif->domainname)    != 0) ) {
		if (  (strlen(tif->hostname) + 1 + strlen(tif->domainname))
		    < MAXGETHOSTNAME) {
			strcat(tif->hostname, "."); 
			strcat(tif->hostname, tif->domainname); 
		}
	}

	__win9x_dhcp_getdnsaddrs (dhcp_num, tif->dns,
				  LSCK_IF_ADDR_INET_DNS_MAX);
	tif->fixed = 0;

	/* Hardware address */
	tif->hw_addrlen = LSCK_IF_ADDR_INET_HW_ADDRLEN_MAX;
	if (!__win9x_dhcp_gethwaddr (dhcp_num, tif->hw_addr, &tif->hw_addrlen))
		return (0);
	tif->hw_type = LSCK_IF_HW_TYPE_ETHERNET;

	/* OK */
	return (1);
}

/* ---------------------------
 * - __win9x_getethcardaddrs -
 * --------------------------- */

static int __win9x_getethcardaddrs (LSCK_IF_ADDR_INET *tif,
				    HKEY hkNet, HKEY hkMSTCP, HKEY hkNetTrans,
				    int *dhcp_count)
{
	long int res;
	DWORD buf_type;
	char buf[256];
	DWORD buf_len;

	/* Is this a static or a dynamically (DHCP) assigned address? */
	buf_len = sizeof(buf);
	res = RegQueryValueEx(hkNetTrans, "IPAddress", NULL,
			      &buf_type, buf, &buf_len);
	if (res != ERROR_SUCCESS) return(0);

	tif->fixed = !(strcmp(buf, DHCP_IP) == 0);

	if (tif->fixed) {
		/* Static address */
		if (!__win9x_getstaticaddr(hkNetTrans, tif))
			return(0);
	} else {
		/* DHCP-assigned */
		(*dhcp_count)++;
		if (!__win9x_getdynamicaddr(hkNetTrans, *dhcp_count, tif))
			return(0);
	}
	
	/* Success */
	return(1);
}
