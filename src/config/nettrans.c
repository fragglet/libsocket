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
    nettrans.c

    This file was written by Richard Dawe.

    ---

    Description:

        This file obtains key network transport settings from the registry
    using the RegDos Group's registry-access code. The keys dealt with are from
    the "System\CurrentControlSet\Services\Class\NetTrans\xxx" keys, where 'xxx'
    are digits.

        Please note that functions that return char *'s use strdup to allocate
    memory, so any code using these functions should free up this memory after
    usage.

        These functions return integer references to subkeys of NetTrans rather
    than HKEYs. This is so it is abstracted away from the registry. The
    returned integers are just a conversion of the subkey numbers, but this
    could be changed sometime.

    NOTE: At the moment this doesn't do too much because of my lack of
          knowledge of dial-up connections :(

    Q:    Are the child keys of NETTRANS_KEY numbered in hexidecimal? If so,
          this code needs modifying.
*/

/* Include some files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lsck/nettrans.h>
#include "regdos.h"

/* Save some typing - this is the root key for all the settings that we need */
#define NETTRANS_KEY    "System\\CurrentControlSet\\Services\\Class\\NetTrans"

/* This is the number of digits the keys under NETTRANS_KEY have */
#define NETTRANS_DIGITS 4

/* According to MS SDK headers - used for maximum registry key length */
#ifndef MAX_PATH
#define MAX_PATH 260
#endif

/* -----------------------
   - nettrans_getdefault -
   ----------------------- */

/* This function returns an integer indicating the default transport number.
   This number is a zero or a positive integer. A return value of -1 indicates
   an error has occurred. */

int nettrans_getdefault (void)
{
    HKEY  hNetTrans = NULL;             /* NETTRANS_KEY key handle  */
    HKEY  hDefault = NULL;              /* Default key handle       */
    char  subkey[NETTRANS_DIGITS + 1];  /* String name + NULL       */
    int   subkey_len = sizeof(subkey);  /* subkey string length     */
    int   nettrans_default = -1;        /* Default subkey number    */
    char  defaultkey[MAX_PATH + 1];     /* Default subkey name      */
    char  defaultval[256];              /* Default key value        */
    DWORD defaultval_len;               /* Default key buffer len   */
    DWORD defaultval_type;              /*    "     "  value type   */
    int   ret, i;

    memset(defaultval, 0, sizeof(defaultval));
    defaultval_len = (DWORD) sizeof(defaultval);

    /* NB: nettrans_default = -1 = fail by default */

    /* Open the NetTrans key */
    if (RegOpenKey(HKEY_LOCAL_MACHINE, NETTRANS_KEY, &hNetTrans) != ERROR_SUCCESS)
        return(-1);

    /* Enumerate each subkey of NetTrans to find the default one */
    i = 0;

    while ( (ret = RegEnumKey(hNetTrans, i++, subkey, subkey_len)) == ERROR_SUCCESS)
    {
        /* Build & get handle for the subkey for searching */
        sprintf(defaultkey, "%s\\Ndi\\Default", subkey);
        if (RegOpenKey(hNetTrans, defaultkey, &hDefault) != ERROR_SUCCESS) continue;

        /* Get default key's value */
        if (RegQueryValueEx(hDefault, NULL, NULL, &defaultval_type, defaultval, &defaultval_len) != ERROR_SUCCESS)
        {
            RegCloseKey(hDefault);
            continue;
        }

        if ( (defaultval_type == REG_DWORD) && (*((int *) defaultval) != 0) ) {
            /* Use the returned subkey name as the keys may not be returned
               in increasing numerical order */
            nettrans_default = atoi(subkey);
            RegCloseKey(hDefault);
            break;
        }
    }

    /* Close the NetTrans key */
    RegCloseKey(hNetTrans);

    /* Return the default value */
    return(nettrans_default);
}

/* ----------------------
   - nettrans_getkeystr -
   ---------------------- */

char *nettrans_getkeystr (int transport, char *name)
{
    HKEY  hNetTrans = NULL;
    HKEY  hTransport = NULL;
    char  transportkey[MAX_PATH + 1];    
    char  buffer[256];
    DWORD buffertype = REG_SZ;
    DWORD bufferlen = sizeof(buffer);
    DWORD ret;

    /* Open the NetTrans key */
    if (RegOpenKey(HKEY_LOCAL_MACHINE, NETTRANS_KEY, &hNetTrans) != ERROR_SUCCESS)
        return(NULL);

    /* Open the transport's key */
    /* Q: sprintf in hex? */
    sprintf(transportkey, "%04i", transport);

    if (RegOpenKey(hNetTrans, transportkey, &hTransport) != ERROR_SUCCESS)
        return(NULL);

    /* Query the string's value */
    ret = RegQueryValueEx(hTransport, name, NULL, &buffertype, buffer, &bufferlen);

    /* Tidy up & return copied string */
    RegCloseKey(hTransport);
    RegCloseKey(hNetTrans);
    if (ret != ERROR_SUCCESS) return(NULL); else return(strdup(buffer));
}
