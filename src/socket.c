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

/* This code was originally rewritten by Indrek Mandre, but was completely
   rewritten by Richard Dawe to support different network interfaces. */

#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <stdlib.h>
#include <sys/fsext.h>
#include <sys/socket.h>
#include <unistd.h>

#include <lsck/lsck.h>
#include <lsck/if.h>

/* Moved this function to fsext.c -> func. decl. here */
int sock_functions(__FSEXT_Fnumber func_number, int *rv, va_list args);

/* Use this union because it's transport independent, i.e. from WSOCK.VXD
   and packet drivers */
LSCK_SOCKET *lsckDescriptor[256];

int c_domain = 0;
int c_type = 0;
int c_protocol = 0;

/* ----------
   - socket -
   ---------- */

int socket (int domain, int type, int protocol)
{
    int s;      /* Socket descriptor */
    int ret;    /* Return value      */

    /* Fail if we can't initialise */
    if (!lsck_init()) {
        errno = ENODEV;
        return(-1);
    }

    /* Save copies of parameters as __FSEXT_alloc_fd seems to nuke them */
    c_domain = domain;
    c_type = type;
    c_protocol = protocol;

    /* Allocate a file descriptor */
    s = __FSEXT_alloc_fd (sock_functions);

    if (s < 0) {
        /* No free file descriptors? */
        errno = EMFILE;
        return (-1);
    }

    setmode (s, O_BINARY);

    /* Allocate a libsocket socket for the descriptor */
    if (lsckDescriptor[s] != NULL) free(lsckDescriptor[s]);
    lsckDescriptor[s] = malloc(sizeof(LSCK_SOCKET));

    if (lsckDescriptor[s] == NULL) {
        _close(s);          /* Free the socket descriptor */
        errno = ENOBUFS;
        return(-1);
    }

    memset(lsckDescriptor[s], 0, sizeof(LSCK_SOCKET));

    lsckDescriptor[s]->AddressFamily = c_domain;
    lsckDescriptor[s]->SocketType    = c_type;
    lsckDescriptor[s]->Protocol      = c_protocol;
    lsckDescriptor[s]->flags         = LSCK_FLAG_BLOCKING;/* Block by def'lt */
    lsckDescriptor[s]->fd            = s;                 /* Store file desc.*/

    /* Create the socket */
    if (lsck_interface() == LSCK_IF_WSOCK)
        ret = wsock_socket(lsckDescriptor[s]);
    else
        ret = 0;
    
    if (ret == -1) {
        _close(s);
        free(lsckDescriptor[s]);

        /* errno should be set by xxx_socket() call */
        return(-1);
    }

    /* Return the descriptor */
    return(s);
}
