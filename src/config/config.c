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
    config.c - libsocket configuration functions
*/

#include <stdlib.h>
#include <string.h>

#include <lsck/config.h>
#include "regdos.h"

/* This is the maximum key length according to SDK headers */
#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#define LIBSOCKET_KEY "SOFTWARE\\DJGPP\\libsocket"

char *configdir = NULL;     /* Where the configuration files are     */
int configdirset = 0;       /* 1 => configdir has been set, 0 => not */

/* ------------------------
   - lsck_config_getentry -
   ------------------------ */

/* This returns the value of the registry key value:

   HKEY_LOCAL_MACHINE\SOFTWARE\DJGPP\libsocket\<filename>\entry
*/

char *lsck_config_getentry(char *filename, char *entry)
{
    HKEY  hkey = NULL;
    HKEY  hfilekey = NULL;
    DWORD entrytype = REG_SZ;
    char  buffer[MAX_PATH];
    DWORD buffersize = sizeof(buffer);
    DWORD ret;

    /* Open the main libsocket key */
    ret = RegOpenKey(HKEY_LOCAL_MACHINE, LIBSOCKET_KEY, &hkey);
    if (ret != ERROR_SUCCESS) return(NULL);

    /* Open the file subkey */
    ret = RegOpenKey(hkey, filename, &hfilekey);
    if (ret != ERROR_SUCCESS) return(NULL);

    /* Get the key contents */
    ret = RegQueryValueEx(hfilekey, entry, NULL, &entrytype, buffer, &buffersize);
    if (ret != ERROR_SUCCESS) return(NULL);

    /* Close keys */
    RegCloseKey(hfilekey);
    RegCloseKey(hkey);

    /* Return the entry */
    return(strdup(buffer));
}

/* ------------------------
   - lsck_config_setentry -
   ------------------------ */

/* This sets the value of the key in the same way that lsck_config_getentry
   gets its value. If the call is successful, 1 is returned, else 0 is
   returned. */

int lsck_config_setentry(char *filename, char *entry, char *entryvalue)
{
    HKEY  hkey = NULL;
    HKEY  hfilekey = NULL;
    DWORD entrytype = REG_SZ;
    DWORD ret;

    /* Open the main libsocket key */
    ret = RegOpenKey(HKEY_LOCAL_MACHINE, LIBSOCKET_KEY, &hkey);
    if (ret != ERROR_SUCCESS) return(0);

    /* Open the file subkey */
    ret = RegOpenKey(hkey, filename, &hfilekey);
    if (ret != ERROR_SUCCESS) return(0);

    /* Get the key contents */
    ret = RegSetValueEx(hfilekey, entry, NULL,
                        entrytype, entryvalue, strlen(entryvalue) + 1);

    /* Close keys */
    RegCloseKey(hfilekey);
    RegCloseKey(hkey);

    /* Return the entry */
    return( (ret == ERROR_SUCCESS) ? 1 : 0 );
}

/* ----------------------
   - lsck_config_getdir -
   ---------------------- */

/* This returns the directory name that libsocket will look in for its config.
   files. */

char *lsck_config_getdir (void)
{
    /* Have we set lsck_config_dir already? */
    if (!configdirset) {
        /* Try the libsocket environment variable, then the Windows dir, else
           leave NULL */
        configdir = getenv("LIBSOCKET");
        if (configdir == NULL) configdir = getenv("windir");

        /* Done this now */
        configdirset = 1;
    }

    return(configdir);
}

/* ----------------------
   - lsck_config_setdir -
   ---------------------- */

/* This sets the directory that libsocket looks for configuration files. I
   guess this isn't particularly useful, but it's here for symmetry (spot the
   physicist :) ). The old directory is returned. */

char *lsck_config_setdir (char *newdir)
{
    char *p;

    p = configdir;          /* Save the current directory     */
    configdir = newdir;     /* Store the new directory        */
    configdirset = 1;       /* Set it now                     */
    return(p);              /* Return the saved old directory */
}
