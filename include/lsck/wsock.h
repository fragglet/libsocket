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

#ifndef __libsocket_wsock_h__
#define __libsocket_wsock_h__

#include <sys/socket.h>
#include <lsck/if.h>

/* --- Private structures --- */
typedef struct _LSCK_WSOCK_ERROR
{
  int ErrorNum;
  char *Error;
  int error;
} LSCK_WSOCK_ERROR;

/* --- Functions --- */
int wsock_init (void);
void wsock_uninit (void);
void wsock_uninit2 (void);

int wsock_socket (LSCK_SOCKET *lsd);

int wsock_bind (LSCK_SOCKET *lsd, struct sockaddr *my_addr, int addrlen);

int wsock_accept (LSCK_SOCKET *lsd, LSCK_SOCKET *nsd,
                  struct sockaddr *sockaddr, int *addrlen);

int wsock_listen (LSCK_SOCKET *lsd, int backlog);

int wsock_send (LSCK_SOCKET *lsd, void *msg, int len, unsigned int flags);

int wsock_sendto (LSCK_SOCKET *lsd, void *msg, int  len, unsigned int flags,
                  struct sockaddr *to, int tolen);


int wsock_recv (LSCK_SOCKET *lsd, void *buf, int len, unsigned int flags);

int wsock_recvfrom (LSCK_SOCKET *lsd, void *buf, int len,
                    unsigned int flags, struct sockaddr *from, int *fromlen);

int wsock_connect (LSCK_SOCKET *lsd, struct sockaddr *serv_addr, int addrlen);

int wsock_getpeername (LSCK_SOCKET *lsd, struct sockaddr *name, int *namelen);

int wsock_getsockname (LSCK_SOCKET *lsd, struct sockaddr *name, int *namelen);

int wsock_closesocket (LSCK_SOCKET *lsd);

int wsock_getsockopt (LSCK_SOCKET *lsd, int level, int optname,
                      void *optval, int *optlen);

int wsock_setsockopt (LSCK_SOCKET *lsd, int level, int optname,
                      const void *optval, int optlen);

int wsock_shutdown (LSCK_SOCKET *lsd, int how);

int wsock_ioctlsocket (LSCK_SOCKET *lsd, int request, int *param);

int wsock_selectsocket (LSCK_SOCKET *lsd, int event);
int wsock_selectsocket_wait (LSCK_SOCKET *lsd, int event);

#endif /* __libsocket_wsock_h__ */
