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
    ini.c - Windows .INI file parsing functions
    Written by Richard Dawe
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lsck/ini.h>

/* ---------------------------
   - GetPrivateProfileString -
   --------------------------- */

/* This does the same as the Windows API call of the same name. */

int GetPrivateProfileString (char *appname, char *keyname, char *deflt,
                             char *dest, size_t destsize, char *filename)
{
    FILE *fp;
    char buf[256], *line = NULL, *p = NULL;
    int in_section = 0, got_key = 0;

    /* Check appname, keyname != NULL */
    if ( (appname == NULL) || (keyname == NULL) ) return(0);

    /* Put the default value in dest */
    if (strlen(deflt) + 1 > destsize) {
        strncpy(dest, deflt, destsize - 1);
        dest[destsize - 1] = '\0';
    }
        strcpy(dest, deflt);

    if (deflt == NULL) *dest = '\0';

    /* Open file, return 0 bytes copied if this fails */
    fp = fopen(filename, "rt");
    if (fp == NULL) return(0);

    /* Parse the file */
    while(fgets(buf, sizeof(buf), fp) != NULL) {
        if (got_key) break;

        /* Find start of line; skip comment lines */
        line = buf;
        while ( (*line == ' ') || (*line == '\t') ) line++;
        if (*line == ';') continue;

        /* Remove trailing spaces, tabs, newlines, returns, etc. */        
        for (p = line + strlen(line) - 1;
             (*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n');
             p--) {
            *p = '\0';
        }

        /* Blank line? */
        if (*line == '\0') continue;           

        /* Do we have a "application", i.e. section, name? */
        if ( (*line == '[') && (*(line + strlen(line) - 1) == ']') ) {
            line++;
            *(line + strlen(line) - 1) = '\0';

            if (strcmp(line, appname) == 0)
                /* Yes */
                in_section = 1;
            else
                /* No */
                in_section = 0;

            continue;
        }

        /* In the right section? */
        if (!in_section) continue;

        /* Is this a key line? */
        if ( (p = strchr(line, '=')) != NULL) {
            /* Split on the equals sign -> line = keyname, p = value */
            *p = '\0';
            p++;

            /* Got the right key? */
            if (strcmp(keyname, line) != 0) continue;

            /* Check buffer size OK */
            if ( (strlen(p) + 1) > destsize ) {
                strncpy(dest, p, destsize - 1);
                dest[destsize - 1] = '\0';                
            } else
                strcpy(dest, p);

            got_key = 1;
        }
    }        

    /* Done */
    fclose(fp);
    return(strlen(dest));
}

/* -------------------------
   - systemini_getdnsaddrs -
   ------------------------- */

/* This uses the GetPrivateProfileString to read in DNSServers from the DNS
   section of system.ini, i.e. settings like:

   [DNS]
   DNSServers=192.168.0.2,192.168.0.3

   The IP address are returned as an array of IP address strings.
*/

char **systemini_getdnsaddrs (void)
{
    char buf[256];
    char systemini_filename[FILENAME_MAX], *p, **addrs;
    int no_addrs, i;

    /* Build system.ini's name */
    *systemini_filename = '\0';

    p = getenv("windir");
    if (p != NULL) sprintf(systemini_filename, "%s/", p);
    strcat(systemini_filename, "system.ini");

    /* If the call fails, then return NULL */
    if (GetPrivateProfileString("DNS", "DNSServers", NULL, buf, sizeof(buf),
                                systemini_filename) == 0)
        return(NULL);

    /* Count the number of addresses */
    no_addrs = 0;
    if (strtok(",", buf) != NULL) {
        no_addrs++;
        while(strtok(",", NULL) != NULL) { no_addrs++; }
    }

    /* No addresses found */
    if (no_addrs == 0) return(NULL);

    /* Create array */
    addrs = (char **) malloc( (no_addrs + 1) * sizeof(char *) );
    if (addrs == NULL) return(NULL);

    /* Copy addresses into it */
    for (i = 0; i < no_addrs + 1; i++) { addrs[i] = NULL; }
    addrs[0] = strtok(",", buf);
    for (i = 1; i < no_addrs; i++) { addrs[i] = strtok(",", NULL); }

    return(addrs);
}
