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
#include <string.h>
#include <time.h>

#include <sys/socket.h>
#include <net/if.h>		       /* For IFNAMSIZ definition */

#include <lsck/lsck.h>
#include <lsck/ini.h>
#include <lsck/if.h>
#include "unix.h"

#include "mailslot.h"

/* --- Global variables --- */

/* Interface information */

char UNIX_IF_NAME[IFNAMSIZ] = "Unix->mailslot";
static int __unix_inited = 0;

LSCK_IF __unix_interface =
{
	/* Interface information */
	UNIX_IF_NAME,
	IF_FLAG_HAS_MSG_PEEK,
	(LSCK_IF_ADDR_TABLE *) NULL,
	(LSCK_IF_ROUTE_TABLE *) NULL,

	/* Interface management functions */
	NULL, __unix_init, NULL, __unix_uninit,

	/* General functions */
	__unix_socket, __unix_socketpair, __unix_close,
	__unix_proto_check, __unix_addrlen_check,
	__unix_bind, __unix_listen, __unix_accept,
	__unix_connect,
	__unix_recv, __unix_recvfrom,
	__unix_send, __unix_sendto,
	__unix_getsockname, __unix_getpeername,
	__unix_getsockopt, NULL /* TODO: __unix_setsockopt */,
	__unix_shutdown,

	/* I/O control functions */
	__unix_fcntl, __unix_ioctl, __unix_select, NULL,
	NULL /* sockatmark */
};

static LSCK_IF_ADDR_TABLE __unix_if_addr_table;

/* ---------------
 * - __unix_init -
 * --------------- */

/* Initialise Unix domain sockets. */

LSCK_IF *__unix_init (void)
{
	char *cfg = NULL;
	char enabled_buf[6];	/* 5 letters/digits + nul = 5 bytes */

	if (__unix_inited)
		return (&__unix_interface);

	/* Check that this interface is enabled. If the "Enabled" entry is
	 * present in the configuration file, presume that it's enabled unless
	 * one of the values false, no or 0 is used. */
	cfg = __lsck_config_getfile ();
	if (cfg != NULL)
		GetPrivateProfileString ("unix", "Enabled", "true",
					 enabled_buf, sizeof (enabled_buf),
					 cfg);
	else
		strcpy(enabled_buf, "true");

	if ((stricmp (enabled_buf, "false") == 0)
	    || (stricmp (enabled_buf, "no") == 0)
	    || (strcmp (enabled_buf, "0") == 0)) {
		if (__lsck_debug_enabled ())
			printf ("unix: Interface disabled\n");
		return (NULL);
	}
	
	/* Install the File System Extension for mailslots. If this fails,
	 * then we can't have Unix domain sockets! */
	if (!__mailslot_install_handler ()) return (NULL);

	/* TODO: Why doesn't this seem to work? */
	/* Ensure random numbers don't start from zero */
	srandom (rawclock ());

	/* Set up the address table */
	bzero (&__unix_if_addr_table, sizeof (__unix_if_addr_table));
	__unix_interface.addr = &__unix_if_addr_table;
	__unix_interface.addr->family = AF_INET;

	/* Done */
	__unix_inited = 1;
	return (&__unix_interface);
}

/* -----------------
 * - __unix_uninit -
 * ----------------- */

/* Nothing is needed here - the mailslot code should automatically destroy
 * any mailslots via an atexit() routine. */

int __unix_uninit (void)
{
	return (1);
}

/* ----------------------
 * - __unix_proto_check -
 * ---------------------- */

int __unix_proto_check (int domain, int type, int protocol)
{
	if (domain != AF_UNIX) return (0);
	if ((type != SOCK_DGRAM) && (type != SOCK_STREAM)) return (0);
    
	/* Only allow Unix domain sockets with a protocol of 0 (default). */
	if (protocol != 0) return(0);
    
	return (1);
}

/* ------------------------
 * - __unix_addrlen_check -
 * ------------------------ */

int __unix_addrlen_check (LSCK_SOCKET *lsd, size_t addrlen)
{
	if (lsd->family != AF_UNIX) return(0);
	if ((lsd->type != SOCK_STREAM) && (lsd->type != SOCK_DGRAM)) return(0);
	if (addrlen < sizeof(struct sockaddr_un)) return(0);	
	return(1);
}
