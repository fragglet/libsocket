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
    hostname.c

    ---

    Description:

        This file include the function lsck_gethostname() which is used instead
    of the Unix-style gethostname() function. lsck_gethostname() uses the mstcp
    registry functions to get the actual hostname, rather than the DJGPP
    library function's value (which I think is either the NetBIOS name or the
    descriptive 'pc').
*/

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>

#include <lsck/mstcp.h>

/* --------------------
   - lsck_gethostname -
   -------------------- */

int lsck_gethostname (char *buf, int size)
{
    char *p, *q;

    /* Try and get the host name. Try and obtain it from the following
       sources:

       1. from the environment, to allow the user to override settings;
       2. from the Windows's TCP/IP registry settings (may not be present);
       3. from DJGPP's gethostname() function, which does similar stuff to
          1 & 2!
    */

    p = strdup(getenv("HOSTNAME"));
    if (p == NULL) p = mstcp_gethostname();

    /* Failed - try DJGPP gethostname */
    if (p == NULL)
    {
        p = (char *) malloc(MAXGETHOSTNAME);
        if (p == NULL) return(-1);              /* malloc failed */
        gethostname(p, MAXGETHOSTNAME);
    }

    /* Don't allow spaces in the host name! Terminate the string at the first
       one found */
    q = strchr(p, ' ');
    if (q != NULL) *q = '\0';

    /* Copy the host name */
    if ( (strlen(p) + 1) > size ) return(-1);   /* Not enough space! */
    strcpy(buf, p);

    /* Free p */
    free(p);

    /* OK */
    return(0);
}
