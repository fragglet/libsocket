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

/*
 * Original code from Dan Hedlund's wsock library, modified by Indrek Mandre
 * (IM) and heavily modified by Richard Dawe (RD).
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <dpmi.h>
#include <sys/farptr.h>
#include <sys/segments.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>		       /* For IFNAMSIZ def */

#include <lsck/windows.h>
#include <lsck/if.h>
#include <lsck/vxd.h>
#include "wsock.h"
#include "wsockvxd.h"

#include <lsck/lsck.h>
#include <lsck/ini.h>
#include <lsck/win9x.h>
#include <lsck/win3x.h>

/* --- Global variables --- */

/* Interface information */
char WSOCK_IF_NAME[IFNAMSIZ] = "WSOCK.VXD";

LSCK_IF __wsock_interface =
{
	/* Interface information */
	WSOCK_IF_NAME,
	IF_FLAG_HAS_NONBLOCKING
	| IF_FLAG_HAS_MSG_PEEK /* Flaky support though? */
	| IF_FLAG_HAS_MSG_WAITALL /* Flaky support though? */,
	(LSCK_IF_ADDR_TABLE *) NULL,
	(LSCK_IF_ROUTE_TABLE *) NULL,

	/* Interface management functions */
	NULL, __wsock_init, __wsock_preuninit, __wsock_uninit,

	/* General functions */
	__wsock_socket, __wsock_socketpair, __wsock_close,
	__wsock_proto_check, __wsock_addrlen_check,
	__wsock_bind, __wsock_listen, __wsock_accept,
	__wsock_connect,
	__wsock_recv, __wsock_recvfrom,
	__wsock_send, __wsock_sendto,
	__wsock_getsockname, __wsock_getpeername,
	__wsock_getsockopt, __wsock_setsockopt,
	__wsock_shutdown,

	/* I/O control functions */
	__wsock_fcntl, __wsock_ioctl, __wsock_select,
	__wsock_nonblocking_check,
	NULL /* sockatmark */
};

/* IP information */
#define DHCP_IP "0.0.0.0"

/* VxD entry points */
int WSockEntry[2];
int WSockEntry2[2];	/* Only used to check if Winsock 2 is present */

/* Used to allocate DPMI memory */
__dpmi_meminfo _SocketP;
__dpmi_meminfo _SocketD;
__dpmi_meminfo _SocketSP;	/* For select() call parameters   */
__dpmi_meminfo _SocketSP2;	/* For select() call parameters 2 */
__dpmi_meminfo _SocketSD;	/* For select() call data         */
__dpmi_meminfo _SocketSD2;	/* For select() call data 2       */

/* DPMI selectors */
int SocketP;	/* Winsock parameters                                   */
int SocketD;	/* Winsock data (used as well as param's in some func's */
int SocketSP;	/* Winsock select() parameters                          */
int SocketSP2;	/* Winsock select() parameters 2                        */
int SocketSD;	/* Winsock select() data                                */
int SocketSD2;	/* Winsock select() data 2                              */

/* Initialisation status */
static int wsock_inited = 0;

/* Private functions */
extern void __wsock_select_wait_kill (void);

/* ----------------
 * - __wsock_init -
 * ---------------- */

/* RD: __wsock_init now returns 1 on success, 0 on failure. If it's previously
 * init'd OK, then it returns 1. */

LSCK_IF *__wsock_init (void)
{
	LSCK_IF_ADDR_INET *inet = NULL;
	char *cfg = NULL;
	char enabled_buf[6];	/* 5 letters/digits + null = 5 bytes */
	int win_version;

	/* Done this before, so it's OK */
	if (wsock_inited)
		return (&__wsock_interface);

	/* Check that this interface is enabled. If the "Enabled" entry is
	 * present in the configuration file, presume that it's enabled unless
	 * one of the values false, no or 0 is used. */
	cfg = __lsck_config_getfile ();
	if (cfg != NULL)
		GetPrivateProfileString ("wsock", "Enabled", "true",
					 enabled_buf, sizeof (enabled_buf),
					 cfg);
	else
		strcpy(enabled_buf, "true");

	if ((stricmp (enabled_buf, "false") == 0)
	    || (stricmp (enabled_buf, "no") == 0)
	    || (strcmp (enabled_buf, "0") == 0)) {
		if (__lsck_debug_enabled ())
			printf ("wsock: Interface disabled\n");
		return (NULL);
	}    

	win_version = __get_windows_version();
    
	if (__lsck_debug_enabled ()) {
		switch (win_version) {
		case WIN_311:
			printf("wsock: Windows 3.x detected\n");
			break;

		case WIN_95:
			printf("wsock: Windows '95 & later detected\n");
			break;

		case WIN_98:
			printf("wsock: Windows '98 detected\n");
			break;

		case WIN_NT:
			printf("wsock: Windows NT detected\n");
			break;

		case WIN_NONE:
		default:
			printf("wsock: Windows not detected\n");
			break;
		}
	}
	
	/* No Windows => no Winsock => fail! */
	switch(win_version) {
	/* Unsupported */
	case WIN_NONE:
	case WIN_NT:		
	    return (NULL);
	    break;

	default:
	    break;
	}

	/* - Load Winsock VxDs - */

	if (win_version == WIN_311) {
		/* Windows 3.11 */
		VxdLdrLoadDevice ("WSOCK.386");
	} else {
		/* Windows '95, '98 */
		VxdLdrLoadDevice ("WSOCK.VXD");
		VxdLdrLoadDevice ("WSOCK2.VXD");
	}

	VxdGetEntry (WSockEntry, 0x003E);
	VxdGetEntry (WSockEntry2, 0x3B0A);

	/* Check the WSOCK.VXD entry point is valid - 0:0 = invalid, others
	 * OK */
	if ((WSockEntry[0] == 0) && (WSockEntry[1] == 0)) {
		errno = ENODEV; /* "No such device" */
		return (NULL);
	}
	
	/* If the WSOCK2.VXD entry point is valid then return, as the code
	 * currently doesn't work with Winsock 2's WSOCK.VXD */
	if ((WSockEntry2[0] != 0) || (WSockEntry2[1] != 0)) {
		errno = ENOSYS;
		return (NULL);
	}

	/* TODO: I really should put a comment here describing what all these
	 * descriptors are used for. */
	
	/* Modify _Socket[P|D|C].size at your peril! *Most* of the library
	 * looks at the size, but I've inadvertently introduced bugs by
	 * fiddling with these before. */

	_SocketP.handle = 0;
	_SocketP.size = 4096;	       /* RD: 65536 is big */
	_SocketP.address = 0;

	if (__dpmi_allocate_memory (&_SocketP) == -1)
		return (NULL); /* RD: failed */

	_SocketD.handle = 0;
	_SocketD.size = 65536;	       /* RD: 65536 is big. Later: No, OK! */
	_SocketD.address = 0;

	if (__dpmi_allocate_memory (&_SocketD) == -1)
		return (NULL); /* RD: failed */

	_SocketSP.handle = 0;
	_SocketSP.size = 4096;
	_SocketSP.address = 0;

	if (__dpmi_allocate_memory (&_SocketSP) == -1)
		return (NULL);

	_SocketSP2.handle = 0;
	_SocketSP2.size = 4096;
	_SocketSP2.address = 0;

	if (__dpmi_allocate_memory (&_SocketSP2) == -1)
		return (NULL);

	_SocketSD.handle = 0;
	_SocketSD.size = 4096;
	_SocketSD.address = 0;

	if (__dpmi_allocate_memory (&_SocketSD) == -1)
		return (NULL);

	_SocketSD2.handle = 0;
	_SocketSD2.size = 4096;
	_SocketSD2.address = 0;

	if (__dpmi_allocate_memory (&_SocketSD2) == -1)
		return (NULL);

	SocketP = __dpmi_allocate_ldt_descriptors (1);
	SocketD = __dpmi_allocate_ldt_descriptors (1);
	SocketSP = __dpmi_allocate_ldt_descriptors (1);
	SocketSP2 = __dpmi_allocate_ldt_descriptors (1);
	SocketSD = __dpmi_allocate_ldt_descriptors (1);
	SocketSD2 = __dpmi_allocate_ldt_descriptors (1);

	/* Descriptors allocated OK? */
	if ((SocketP == -1) || (SocketD == -1)
	    || (SocketSP == -1) || (SocketSP2 == -1)
	    || (SocketSD == -1) || (SocketSD2 == -1))
		return (NULL);

	__dpmi_set_segment_base_address (SocketP, _SocketP.address);
	__dpmi_set_segment_base_address (SocketD, _SocketD.address);
	__dpmi_set_segment_base_address (SocketSP, _SocketSP.address);
	__dpmi_set_segment_base_address (SocketSP2, _SocketSP2.address);
	__dpmi_set_segment_base_address (SocketSD, _SocketSD.address);
	__dpmi_set_segment_base_address (SocketSD2, _SocketSD2.address);

	__dpmi_set_segment_limit (SocketP, _SocketP.size);
	__dpmi_set_segment_limit (SocketD, _SocketD.size);
	__dpmi_set_segment_limit (SocketSP, _SocketSP.size);
	__dpmi_set_segment_limit (SocketSP2, _SocketSP2.size);
	__dpmi_set_segment_limit (SocketSD, _SocketSD.size);
	__dpmi_set_segment_limit (SocketSD2, _SocketSD2.size);
	
	/* - Get the IP details - */

	/* Config override? */
	inet = __lsck_if_addr_inet_getconfig ("wsock");
	
	if (inet != NULL) {
		/* If malloc fails, fall back on auto - use just loopback. */
		__wsock_interface.addr =
			(LSCK_IF_ADDR_TABLE *)
			malloc (sizeof (LSCK_IF_ADDR_TABLE));
		
		if (__wsock_interface.addr != NULL) {
			__wsock_interface.addr->family = AF_INET;
			__wsock_interface.addr->table.inet[0] =
				__lsck_if_addr_inet_loopback ();
			__wsock_interface.addr->table.inet[1] = inet;
			__wsock_interface.addr->table.inet[2] = NULL;
		}
	}
	
	/* No config override, so try to configure automatically */
	if (__wsock_interface.addr == NULL) {
		switch(win_version) {
		/* Default to Windows '95 */
		default:			
			__wsock_interface.addr = __win9x_getaddrtable();
			break;

		case WIN_311:
			__wsock_interface.addr = __win3x_getaddrtable();
			break;
		}
	}

	if (__wsock_interface.addr == NULL) {
		wsock_inited = 1;
		__wsock_uninit ();
		return (NULL);
	}
	
	/* Set this now we've done everything successfully */
	wsock_inited = 1;

	/* Success */
	return (&__wsock_interface);
}

/* -------------------
 * - __wsock_uninit2 -
 * ------------------- */

/* This frees DPMI memory and unloads the VxDs. */

int __wsock_uninit2 (void)
{
	if (!wsock_inited) return (0);

	/* Use the normal uninit function first */
	if (!__wsock_uninit ())
		return (0);
	
	wsock_inited = 0;

	VxdLdrUnLoadDevice ("WSOCK.386");
	VxdLdrUnLoadDevice ("WSOCK.VXD");
	VxdLdrUnLoadDevice ("WSOCK2.VXD"); /* RD: Winsock 2 too! */

	/* OK */
	return (1);
}

/* ------------------
 * - __wsock_uninit -
 * ------------------ */

/* This frees only DPMI memory. */

int __wsock_uninit (void)
{
	if (!wsock_inited) return (0);
	
	wsock_inited = 0;

	/* Free DPMI memory */
	__dpmi_free_memory (_SocketSD2.handle);
	__dpmi_free_memory (_SocketSD.handle);
	__dpmi_free_memory (_SocketSP2.handle);
	__dpmi_free_memory (_SocketSP.handle);
	__dpmi_free_memory (_SocketP.handle);
	__dpmi_free_memory (_SocketD.handle);

	/* Free descriptors */
	__dpmi_free_ldt_descriptor (SocketSD2);
	__dpmi_free_ldt_descriptor (SocketSD);
	__dpmi_free_ldt_descriptor (SocketSP2);
	__dpmi_free_ldt_descriptor (SocketSP);
	__dpmi_free_ldt_descriptor (SocketP);
	__dpmi_free_ldt_descriptor (SocketD);
	
	/* OK */
	return (1);
}

/* ---------------------
 * - __wsock_preuninit -
 * --------------------- */

/* Stuff that needs to be done before the uninitialisation can take place. */

int __wsock_preuninit (void)
{
	/* Kill any sockets being select()'d */
	__wsock_select_wait_kill ();

	return (1);
}

/* -----------------------
 * - __wsock_proto_check -
 * -----------------------  */

/* wsock.vxd supported protocols. This was found empirically, but may have to
 * be changed if Winsock 2 support can be added. */

int __wsock_proto_check (int domain, int type, int protocol)
{
	/* Winsock support inited? */
	if (!wsock_inited) return (0);

	/* Only internet sockets */
	if (domain != AF_INET) return (0);

	/* Only TCP streams, UDP datagrams */
	if (((type == SOCK_STREAM) && (protocol == IPPROTO_TCP))
	    || ((type == SOCK_DGRAM) && (protocol == IPPROTO_UDP)))
		return (1);

	/* Fail all others */
	return (0);
}

/* -------------------------
 * - __wsock_addrlen_check -
 * ------------------------- */

int __wsock_addrlen_check (LSCK_SOCKET *lsd, size_t addrlen)
{
	if (lsd->family != AF_INET) return(0);
	if ((lsd->type != SOCK_STREAM) && (lsd->type != SOCK_DGRAM)) return(0);
	if (addrlen < sizeof(struct sockaddr_in)) return(0);
	return(1);
}
