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
    if.h
*/

#ifndef __libsocket_if_h__
#define __libsocket_if_h__

#include <sys/socket.h>

/* --- Defines --- */

/* Interfaces */
#define LSCK_MIN_IF         1       /* Minimum interface */

#define LSCK_IF_NONE        0       /* No interface      */
#define LSCK_IF_WSOCK       1       /* Via WSOCK.VXD     */
#define LSCK_IF_PKTDRV      2       /* Via packet driver */

#define LSCK_MAX_IF         2       /* Maximum interface */

/* Flag constants for LSCK_SOCKET struct */
#define LSCK_FLAG_BLOCKING  1

/* --- Structures --- */
typedef struct _LSCK_INETADDRESS
{
  short family;
  short port;
  int address;
  char zero [8];
} LSCK_WSOCK_INETADDRESS;

typedef struct _LSCK_SOCKET_WSOCK {
    int _Socket;
    int _SocketHandle;
    LSCK_WSOCK_INETADDRESS inetaddr;
} LSCK_SOCKET_WSOCK;

typedef struct _LSCK_SOCKET_PKTDRV {
    int dummy;
} LSCK_SOCKET_PKTDRV;

typedef struct _LSCK_SOCKET {
    int AddressFamily;
    int SocketType;
    int Protocol;
    int flags;
    int fd;                     /* File descriptor */
    int interface;              /* Interface       */

    LSCK_SOCKET_WSOCK  wsock;
    LSCK_SOCKET_PKTDRV pktdrv;
} LSCK_SOCKET;

/* --- Interface includes --- */
#include <lsck/wsock.h>
#include <lsck/pktdrv.h>

/* --- Functions --- */
int lsck_interface (void);
int lsck_init (void);
int lsck_uninit (void);

#endif  /* __libsocket_if_h__ */
