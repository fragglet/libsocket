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
    ras.h
*/

#ifndef __libsocket_ras_h__
#define __libsocket_ras_h__

#include <sys/socket.h>         /* Needed for in_addr typedef              */

typedef struct {
    int ip_fixed;               /* Local IP addr. fixed? 1 = yep, 0 = nope */
    struct in_addr ip_machine;  /* Machine's IP address                    */
    int ip_dns_specified;       /* DNS IP addr. spec'd? 1 = yep, 0 = nope  */
    int ip_dns_count;           /* No. DNS IP addr. spec'd                 */
    struct in_addr ip_dns[2];   /* DNS IP addresses                        */
} ras_data;

int    ras_active (void);
char  *ras_getdefaultphonebook (void);
int    ras_getdata (ras_data *rd);
char **ras_getdnsaddrs (void);

#endif  /* __libsocket_ras_h__ */
