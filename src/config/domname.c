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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <lsck/ws.h>
#include <lsck/mstcp.h>      /* Include mstcp functions       */
#include <lsck/config.h>     /* Use lsck_config_*() functions */
#include <lsck/domname.h>    /* Ensure consistency            */

char *resolv_conf_getdomainname (void);

/* -----------------
   - getdomainname -
   ----------------- */

int getdomainname(char *name, size_t len)
{
    /* RD: The replacement code obtains the domain name from the registry */
    char *p;

    /* Fail automatically if len <= 0 */
    /* TODO: Set errno? What to? See comment for strncpy below. */
    if (len <= 0)
    {
        errno = EINVAL;	/* IM: einval */
        return(-1);
    }

    /* The domain name is obtained from the following places in this order:

        1. the LOCALDOMAIN environment variable, to allow the user to override
           the other methods;

        2. the resolv.conf configuration file;

        3. Windows's registry settings for the TCP/IP transport (which may or
           may not be present - if the DNS server information has been filled
           in then it will have been.
    */

    p = strdup(getenv("LOCALDOMAIN"));
    if (p == NULL) p = resolv_conf_getdomainname();
    if (p == NULL) p = mstcp_getdomainname();

    if (p == NULL) {
        /* Function failed - return function not implemented error */
        /*errno = ENOSYS;*/
        /* *name = '\0'; IM: no need for that */     /* As suggested by IM */ 
        errno = EINVAL;     /* As suggested by IM */
        return(-1);
    }

    /* Copy as much as possible to the spec'd buffer */
    /* TODO: Return error if larger buffer needed?   */
    strncpy(name, p, len);

    /* Free the memory alloc'd by mstcp_getdomainname() - reason: the string
       returned is a strdup'd copy of the registry entry. */
    free(p);

    return 0;
    /* RD: End */

    /* Old code */
    /*strncpy ( name, "cant.find.it", len );*/ /* Not italian... */
}

/* -----------------
   - setdomainname -
   ----------------- */

int setdomainname(const char *name, size_t len)
{
 errno = EPERM;
 return -1;
}

/* -----------------------------
   - resolv_conf_getdomainname -
   ----------------------------- */

/* This reads the domain name from the resolv.conf file. The domain name is
   taken from the line reading "domain <domain name>". A copy of the string is
   returned (NB: needs freeing) if successful, else NULL. */

char *resolv_conf_getdomainname (void)
{
    char *p = NULL;
    char *x = NULL;
    FILE *f = NULL;
    char _PATH_RESCONF[1024];
    char buffer[256];

    /* Build resolv.conf's filename and open the file - cope with LFN+SFN
       combinations, i.e. created with LFN, read with SFN */
    if ( (x = lsck_config_getdir()) == NULL) return(NULL);
    sprintf(_PATH_RESCONF, "%s/resolv.conf", x);
    if (access(_PATH_RESCONF, R_OK)) sprintf(_PATH_RESCONF, "%s/resolv~1.con", x);
    if (access(_PATH_RESCONF, R_OK)) sprintf(_PATH_RESCONF, "%s/resolv.con", x);

    if ( (f = fopen(_PATH_RESCONF, "rt")) == NULL) return(NULL);

    /* Parse the file */
    while (fgets(buffer, sizeof(buffer), f) != NULL) {
        /* Skip whitespace in line */
        for (x = buffer; (*x == ' ') || (*x == '\t'); x++) {;}

        /* We have a match! */
        if (strstr(buffer, "domain") == x) {
            x += strlen("domain");
            for (x = buffer; (*x == ' ') || (*x == '\t'); x++) {;}
            p = strdup(x);
        }
    }

    /* Close the file */
    fclose(f);

    return(p);
}
