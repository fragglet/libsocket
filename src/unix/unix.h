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

#ifndef __libsocket_unix_h__
#define __libsocket_unix_h__

#ifdef __cplusplus
extern "C" {
#endif 

#include <sys/socket.h>
#include <lsck/if.h>
	
/* --- Defines --- */

/* Paths */

/* This should agree with the mailslot naming! */
#define UNIX_LOCAL_PATH "/dev/mailslot/local"

/* Message passing constants */

/* Version number for headers */
#define UNIX_MSG_VERSION		0x01

/* Bitfield: <ack> <nack> <datagram> <server> <4 bits> */
#define UNIX_MSG_ACK_BIT		0x80
#define UNIX_MSG_NACK_BIT		0x40
#define UNIX_MSG_DGRAM_BIT		0x20
#define UNIX_MSG_SERVER_BIT		0x10

#define UNIX_MSG_CLIENT_OPEN		0x01
#define UNIX_MSG_CLIENT_CLOSE		0x02
#define UNIX_MSG_CLIENT_SHUTDOWN	0x03

#define UNIX_MSG_SERVER_OPEN		(0x01 | UNIX_MSG_SERVER_BIT)
#define UNIX_MSG_SERVER_CLOSE		(0x02 | UNIX_MSG_SERVER_BIT)
#define UNIX_MSG_SERVER_SHUTDOWN	(0x03 | UNIX_MSG_SERVER_BIT)

#define UNIX_MSG_DATA			0x08

/* --- Structures --- */

/* Unix interface secret structure */
typedef struct _UNIX_MSG_SECRET {
   long n   __attribute__((packed));	/* Random number */
   long pid __attribute__((packed));	/* Process id    */
} UNIX_MSG_SECRET;
	
/* Unix domain socket data */
typedef struct _LSCK_SOCKET_UNIX {
	/* Local & "remote" paths, to support __unix_getsockname() and
	 * __unix_getpeername(). */
	char sockpath[sizeof(struct sockaddr_un)];
	char peerpath[sizeof(struct sockaddr_un)];
	
	int  fd_w;		/* Write file descriptor     */
	int  fd_r;		/* Read file descriptor      */
	UNIX_MSG_SECRET secret;	/* Client<->server secret    */
	long seq_w;		/* Next send seq. num.       */
	long seq_r;		/* Next receive seq. num.    */
} LSCK_SOCKET_UNIX;
	
/* Header for all messages */
typedef struct _UNIX_MSG_HEADER {
    char  version   __attribute__((packed)); /* Protocol version             */
    char  type      __attribute__((packed)); /* Message type,
                                              * UNIX_MSG_[CLIENT|SERVER]_*   */

    /* Secret shared between ends of connection */
    UNIX_MSG_SECRET secret __attribute__((packed));
	
    long  seq_start __attribute__((packed)); /* First sequence number of
                                              * data                         */
    long  seq_end   __attribute__((packed)); /* Last sequence number of data */
    short offset    __attribute__((packed)); /* Offset of data in packet     */
} UNIX_MSG_HEADER;

/* Message structures */

typedef struct _UNIX_MSG_OPEN_REQUEST {
    UNIX_MSG_HEADER header    __attribute__((packed));
    char            path[128] __attribute__((packed));
} UNIX_MSG_OPEN_REQUEST;

typedef struct _UNIX_MSG_OPEN_REPLY {
    UNIX_MSG_HEADER header    __attribute__((packed));
    char            path[128] __attribute__((packed));
} UNIX_MSG_OPEN_REPLY;

/* Datagrams include originating socket details. */
typedef struct _UNIX_MSG_DGRAM {
    UNIX_MSG_HEADER header    __attribute__((packed));
    char            path[128] __attribute__((packed));
} UNIX_MSG_DGRAM;

/* --- Functions --- */
extern LSCK_IF *__unix_init (void);

extern int __unix_uninit (void);

extern int __unix_proto_check (int /* domain */,
			       int /* type */,
			       int /* protocol */);

extern int __unix_addrlen_check (LSCK_SOCKET * /* lsd */,
				 size_t        /* addrlen */);

extern int __unix_socket (LSCK_SOCKET * /* lsd */);

extern int __unix_socketpair (LSCK_SOCKET * /* lsd */ [2]);

extern int __unix_bind (LSCK_SOCKET *     /* lsd */,
                        struct sockaddr * /* my_addr */,
			size_t            /* addrlen */);

extern int __unix_connect (LSCK_SOCKET *     /* lsd */,
                           struct sockaddr * /* serv_addr */,
			   size_t            /* addrlen */);

extern int __unix_accept (LSCK_SOCKET *     /* lsd */,
			  LSCK_SOCKET *     /* nsd */,
                          struct sockaddr * /* addr */,
			  size_t *          /* addrlen */);

extern int __unix_listen (LSCK_SOCKET * /* lsd */, int /* backlog */);

extern ssize_t __unix_recv (LSCK_SOCKET * /* lsd */,
			    void *        /* buf */,
			    size_t        /* len */,
                            unsigned int  /* flags */);

extern ssize_t __unix_recvfrom (LSCK_SOCKET *     /* lsd */,
				void *            /* buf */,
				size_t            /* len */,
                                unsigned int      /* flags */,
                                struct sockaddr * /* from */,
				size_t *          /* fromlen */);

extern ssize_t __unix_send (LSCK_SOCKET * /* lsd */,
			    void *        /* msg */,
			    size_t        /* len */,
                            unsigned int  /* flags */);

extern ssize_t __unix_sendto (LSCK_SOCKET *     /* lsd */,
			      void *            /* msg */,
			      size_t            /* len */,
                              unsigned int      /* flags */,
                              struct sockaddr * /* to */,
			      size_t            /* tolen */);

extern int __unix_close (LSCK_SOCKET * /* lsd */);

extern int __unix_getsockname (LSCK_SOCKET *     /* lsd */,
                               struct sockaddr * /* name */,
			       size_t *          /* namelen */);

extern int __unix_getpeername (LSCK_SOCKET *       /* lsd */,
				 struct sockaddr * /* name */,
				 size_t *          /* namelen */);

extern int __unix_shutdown (LSCK_SOCKET * /* lsd */, int /* how */);

extern int __unix_fcntl (LSCK_SOCKET * /* lsd */,
			 int *         /* rv */,
			 int           /* command */,
			 int           /* request */);

extern int __unix_ioctl (LSCK_SOCKET * /* lsd */,
			 int *         /* rv */,
			 int           /* request */,
			 int *         /* param */);

extern int __unix_select (LSCK_SOCKET * /* lsd */, int /* event */);

extern int __unix_getsockopt (LSCK_SOCKET * /* lsd */,
			      int *         /* rv  */,
			      int           /* level */,
			      int           /* optname */,
			      void *        /* optval */,
			      size_t *      /* optlen */);

#ifdef __cplusplus
};
#endif

#endif  /* __libsocket_unix_h__ */
