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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dir.h>

#include <sys/socket.h>
#include <sys/un.h>

#include <lsck/if.h>
#include "unix.h"

/* ---------------------
 * - __unix_socketpair -
 * --------------------- */

int __unix_socketpair (LSCK_SOCKET *lsd[2])
{
	LSCK_SOCKET_UNIX *usock[2] = { lsd[0]->idata, lsd[1]->idata };
	struct sockaddr_un sun[2];
	char dest_path[MAXPATH];
	int fd_w = -1;	
	int ret = -1;
	int i;

	/* Generate two random local addresses. Use a hopefully unique
	 * address-space to avoid clashes with other programs. */
	sun[0].sun_family = sun[1].sun_family = AF_UNIX;		
	sprintf(sun[0].sun_path, "/tmp/lsck/sockpair/%08lx", random());
	sprintf(sun[1].sun_path, "/tmp/lsck/sockpair/%08lx", random());

	/* Bind the sockets to local addresses. */
	ret = __unix_bind(lsd[0], (struct sockaddr *) &sun[0], sizeof(sun[0]));
	if (ret == -1) return(-1);	
	ret = __unix_bind(lsd[1], (struct sockaddr *) &sun[1], sizeof(sun[1]));
	if (ret == -1) return(-1);	

	/* Now connect the two sockets appropriately. */
	switch(lsd[0]->type) {
	case SOCK_STREAM:
		/* For streams, we must make it look like the sockets have
		 * been through socket(), bind(), listen() + accept() or
		 * connect() to produce a pair of connected sockets. */

		/* Generate secrets for each socket. This is used in
		 * message exchanges to indicate that the data is coming from
		 * or going to the right socket. */
		usock[0]->secret.n   = usock[1]->secret.n   = random();
		usock[0]->secret.pid = usock[1]->secret.pid = getpid();

		/* Set the sequence numbers up. */
		usock[0]->seq_w = usock[1]->seq_r = random();
		usock[1]->seq_w = usock[0]->seq_r = random();
		
		/* Point the sockets at each other. */
		strcpy(usock[0]->peerpath, sun[1].sun_path);
		strcpy(usock[1]->peerpath, sun[0].sun_path);

		/* Create file descriptors so the two sockets can write to each
		 * other. */
		for (i = 0; i < 2; i++) {
			sprintf(dest_path, "%s%s",
				UNIX_LOCAL_PATH,
				sun[1 - i].sun_path);
			
			fd_w = open(dest_path, O_WRONLY | O_BINARY, S_IWUSR);
			
			if (fd_w == -1) {
				ret = -1;
				break;
			}
			
			usock[i]->fd_w = fd_w;
		}
		
		/* Set up the status bits appropriately. */
		lsd[0]->connected = lsd[1]->connected = 1;
		lsd[0]->bound     = lsd[1]->bound     = 1;
		
		ret = 0;
		break;

	case SOCK_DGRAM:
		/* For datagrams, it's just a question of pointing the sockets
		 * at each other by name. */
		strcpy(usock[0]->peerpath, sun[1].sun_path);
		strcpy(usock[1]->peerpath, sun[0].sun_path);

		/* Set up the status bits appropriately. */
		lsd[0]->bound = lsd[1]->bound = 1;
		
		ret = 0;
		break;
		
	default:
		errno = EPROTONOSUPPORT;
		ret   = -1;
	}
	
	return(ret);
}
