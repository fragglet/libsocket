/*
 *  libsocket - BSD socket like library for DJGPP
 *  Copyright 1997, 1998 by Indrek Mandre
 *  Copyright 1997-2000 by Richard Dawe
 *
 *  Portions of libsocket Copyright 1985-1993 Regents of the University of 
 *  California.
 *  Portions of libsocket Copyright 1991, 1992 Free Software Foundation, Inc.
 *  Portions of libsocket Copyright 1997, 1998 by the Regdos Group.
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

#ifdef __cplusplus
extern "C"
{
#endif

#include <string.h>		/* For NULL */
#include <dpmi.h>
#include <sys/types.h>

#include <sys/socket.h>

#include <lsck/if.h>

/* --- Defines --- */
extern char WSOCK_IF_NAME[];

/* --- Private structures --- */

/* Error mapping */
typedef struct _LSCK_WSOCK_ERROR
{
  int ErrorNum;
  char *Error;
  int error;
} LSCK_WSOCK_ERROR;

/* WSOCK.VXD stores addresses like this: */
typedef struct _LSCK_WSOCK_INETADDRESS
{
	short family __attribute__((packed));
	short port   __attribute__((packed));
	int address  __attribute__((packed));
	char zero[8] __attribute__((packed));
} LSCK_WSOCK_INETADDRESS;

/* WSOCK.VXD socket information */
typedef struct _LSCK_SOCKET_WSOCK {
	int _Socket;
	int _SocketHandle;
	LSCK_WSOCK_INETADDRESS inetaddr;
} LSCK_SOCKET_WSOCK;

/* --- Functions --- */
extern LSCK_IF * __wsock_init (void);

extern int __wsock_uninit (void);

extern int __wsock_uninit2 (void);

extern int __wsock_preuninit (void);

extern int __wsock_socket (LSCK_SOCKET * /* lsd */);

extern int __wsock_socketpair (LSCK_SOCKET * /* lsd */ [2]);

extern int __wsock_bind (LSCK_SOCKET *     /* lsd */,
                         struct sockaddr * /* my_addr */,
			 size_t            /* addrlen */);

extern int __wsock_accept (LSCK_SOCKET *     /* lsd */,
			   LSCK_SOCKET *     /* nsd */,
                           struct sockaddr * /* sockaddr */,
			   size_t *          /* addrlen */);

extern int __wsock_listen (LSCK_SOCKET * /* lsd */, int /* backlog */);

extern ssize_t __wsock_send (LSCK_SOCKET * /* lsd */,
			     void *        /* msg */,
			     size_t        /* len */,
                             unsigned int  /* flags */);

extern ssize_t __wsock_sendto (LSCK_SOCKET *     /* lsd */,
			       void *            /* msg */,
			       size_t            /* len */,
                               unsigned int      /* flags */,
                               struct sockaddr * /* to */,
			       size_t            /* tolen */);

extern ssize_t __wsock_recv (LSCK_SOCKET * /* lsd */,
			     void *        /* buf */,
			     size_t        /* len */,
                             unsigned int  /* flags */);

extern ssize_t __wsock_recvfrom (LSCK_SOCKET *     /* lsd */,
				 void *            /* buf */,
				 size_t            /* len */,
                                 unsigned int      /* flags */,
				 struct sockaddr * /* from */,
				 size_t *          /* fromlen */);

extern int __wsock_connect (LSCK_SOCKET *     /* lsd */,
                            struct sockaddr * /* serv_addr */,
			    size_t            /* addrlen */);

extern int __wsock_getpeername (LSCK_SOCKET *     /* lsd */,
                                struct sockaddr * /* name */,
				size_t *          /* namelen */);

extern int __wsock_getsockname (LSCK_SOCKET *     /* lsd */,
                                struct sockaddr * /* name */,
				size_t *          /* namelen */);

extern int __wsock_close (LSCK_SOCKET * /* lsd */);

extern int __wsock_getsockopt (LSCK_SOCKET * /* lsd */,
			       int *         /* rv */,
			       int           /* level */,
			       int           /* optname */,
                               void *        /* optval */,
			       size_t *      /* optlen */);

extern int __wsock_setsockopt (LSCK_SOCKET * /* lsd */,
			       int *         /* rv */,
			       int           /* level */,
			       int           /* optname */,
                               const void *  /* optval */,
			       size_t        /* optlen */);

extern int __wsock_shutdown (LSCK_SOCKET * /* lsd */, int /* how */);

extern int __wsock_fcntl (LSCK_SOCKET * /* lsd */,
			  int *         /* rv */,
			  int           /* command */,
			  int           /* request */);

extern int __wsock_ioctl (LSCK_SOCKET * /* lsd */,
			  int *         /* rv */,
			  int           /* request */,
			  int *         /* param */);

extern int __wsock_select (LSCK_SOCKET * /* lsd */, int /* event */);

extern int __wsock_select_wait (LSCK_SOCKET * /* lsd */, int /* event */);

extern int __wsock_nonblocking_check (LSCK_SOCKET * /* lsd */);

extern void __wsock_callvxd (int /* func */);

extern void __wsock_callvxd2 (int /* func */, int /* sel */);

extern int __wsock_proto_check (int /* domain */,
				int /* type */,
				int /* protocol */);

extern int __wsock_addrlen_check (LSCK_SOCKET * /* lsd */,
				  size_t /* addrlen */);

extern int __wsock_errno (int /* i_errno */);

/* --- Global variables --- */

/* Memory buffers */
extern int SocketP;
extern int SocketD;
extern int SocketSP;
extern int SocketSP2;
extern int SocketSD;
extern int SocketSD2;
extern __dpmi_meminfo _SocketP;
extern __dpmi_meminfo _SocketD;
extern __dpmi_meminfo _SocketSP;
extern __dpmi_meminfo _SocketSP2;
extern __dpmi_meminfo _SocketSD;
extern __dpmi_meminfo _SocketSD2;

/* Error codes */
extern int _VXDError;

#ifdef __cplusplus
};
#endif

#endif /* __libsocket_wsock_h__ */
