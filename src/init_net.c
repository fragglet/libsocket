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
    init_net.c
    Written by Richard Dawe

    Initialise all networking services.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lsck/if.h>

#include "winsock.h"
#include "lsckglob.h"

int lsck_inited = 0;

int lsck_if_inited[LSCK_MAX_IF - LSCK_MIN_IF + 1];
int lsck_if_count = 0;
int lsck_if_default = LSCK_IF_NONE;

/* -------------
   - lsck_init -
   ------------- */

int lsck_init (void) {
    int i;

    if (!lsck_inited) {
        /* Clear all descriptors */
        for (i = 0; i < LSCK_MAX_SOCKET; i++) lsckDescriptor[i] = NULL;

        /* Initialise all interfaces */
        for (i = LSCK_MIN_IF; i < LSCK_MAX_IF; i++) {
            switch(i) {
                case LSCK_IF_WSOCK:
                    lsck_if_inited[i] = wsock_init();
                    break;

                default:
                    lsck_if_inited[i] = 0;
                    break;
            }
        }

        /* Count available interfaces */
        for (i = LSCK_MIN_IF; i < LSCK_MAX_IF; i++)
            if (lsck_if_inited[i]) lsck_if_count++;

        /* Find default interface */
        for (i = LSCK_MIN_IF; i < LSCK_MAX_IF; i++) {
            if (lsck_if_inited[i]) {
                lsck_if_default = i;
                break;
            }
        }

        /* Done */
        lsck_inited = 1;
    }

    return(lsck_inited);
}

/* ---------------
   - lsck_uninit -
   --------------- */

int lsck_uninit (void) {
    int i;

    if (!lsck_inited) return(0);

    /* Free all descriptors */
    for (i = 0; i < LSCK_MAX_SOCKET; i++)
        if (lsckDescriptor[i] != NULL) free(lsckDescriptor[i]);

    /* Initialise all interfaces */
    for (i = 0; i < LSCK_MAX_IF; i++) { ;
    }

    /* OK */
    return(1);
}

/* ------------------
   - lsck_interface -
   ------------------ */

int lsck_interface (void)
{
    lsck_init();
    return(lsck_if_default);
}
