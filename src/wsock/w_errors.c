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
 * Original code from Dan Hedlund's wsock library.
 */

/* Names slightly modified by Richard Dawe. */

#include <lsck/wsock.h>
#include <lsck/ws.h>
#include <winsock.h>

LSCK_WSOCK_ERROR WSA_ERRORS [] = {
{WSAEINTR, "WSAEINTR - Interrupted", EINTR }, 
{WSAEBADF, "WSAEBADF - Bad file number", EBADF}, 
{WSAEFAULT, "WSAEFAULT - Bad address", EFAULT }, 
{WSAEINVAL, "WSAEINVAL - Invalid argument", EINVAL }, 
{WSAEMFILE, "WSAEMFILE - Too many open files", ENFILE}, 
/* 
* Windows Sockets definitions of regular Berkeley error constants 
*/ 
{WSAEWOULDBLOCK, "WSAEWOULDBLOCK - Socket marked as non-blocking", EAGAIN }, 
{WSAEINPROGRESS, "WSAEINPROGRESS - Blocking call in progress", EINPROGRESS }, 
{WSAEALREADY, "WSAEALREADY - Command already completed", EALREADY }, 
{WSAENOTSOCK, "WSAENOTSOCK - Descriptor is not a socket", ENOTSOCK }, 
{WSAEDESTADDRREQ, "WSAEDESTADDRREQ - Destination address required", EDESTADDRREQ}, 
{WSAEMSGSIZE, "WSAEMSGSIZE - Data size too large", EMSGSIZE }, 
{WSAEPROTOTYPE, "WSAEPROTOTYPE - Protocol is of wrong type for this socket", EPROTOTYPE }, 
{WSAENOPROTOOPT, "WSAENOPROTOOPT - Protocol option not supported for this socket type", ENOPROTOOPT }, 
{WSAEPROTONOSUPPORT, "WSAEPROTONOSUPPORT - Protocol is not supported", EPROTONOSUPPORT }, 
{WSAESOCKTNOSUPPORT, "WSAESOCKTNOSUPPORT - Socket type not supported by this address family", ESOCKTNOSUPPORT }, 
{WSAEOPNOTSUPP, "WSAEOPNOTSUPP - Option not supported", EOPNOTSUPP }, 
{WSAEPFNOSUPPORT, "WSAEPFNOSUPPORT - Protocol family not supported", EPFNOSUPPORT }, 
{WSAEAFNOSUPPORT, "WSAEAFNOSUPPORT - Address family not supported by this protocol", EAFNOSUPPORT }, 
{WSAEADDRINUSE, "WSAEADDRINUSE - Address is in use", EADDRINUSE }, 
{WSAEADDRNOTAVAIL, "WSAEADDRNOTAVAIL - Address not available from local machine", EADDRNOTAVAIL }, 
{WSAENETDOWN, "WSAENETDOWN - Network subsystem is down", ENETDOWN }, 
{WSAENETUNREACH, "WSAENETUNREACH - Network cannot be reached", ENETUNREACH }, 
{WSAENETRESET, "WSAENETRESET - Connection has been dropped", ENETRESET }, 
{WSAECONNABORTED, "WSAECONNABORTED -Software caused connection abort", ECONNABORTED }, 
{WSAECONNRESET, "WSAECONNRESET - Connection reset by peer", ECONNRESET }, 
{WSAENOBUFS, "WSAENOBUFS - No buffer space available", ENOBUFS }, 
{WSAEISCONN, "WSAEISCONN - Socket is already connected", EISCONN }, 
{WSAENOTCONN, "WSAENOTCONN - Socket is not connected", ENOTCONN }, 
{WSAESHUTDOWN, "WSAESHUTDOWN - Socket has been shut down", ESHUTDOWN }, 
{WSAETOOMANYREFS, "WSAETOOMANYREFS - Too many references: cannot splice", ETOOMANYREFS }, 
{WSAETIMEDOUT, "WSAETIMEDOUT - Command timed out", ETIMEDOUT }, 
{WSAECONNREFUSED, "WSAECONNREFUSED - Connection refused", ECONNREFUSED }, 
{WSAELOOP, "WSAELOOP - Too many symbolic links encountered", ELOOP }, 
{WSAENAMETOOLONG, "WSAENAMETOOLONG - File name too long", ENAMETOOLONG }, 
{WSAEHOSTDOWN, "WSAEHOSTDOWN - Host is down", EHOSTDOWN }, 
{WSAEHOSTUNREACH, "WSAEHOSTUNREACH - No route to host", EHOSTUNREACH }, 
{WSAENOTEMPTY, "WSAENOTEMPTY - Directory not empty", ENOTEMPTY }, 
{WSAEPROCLIM, "WSAEPROCLIM - ", -1 }, 
{WSAEUSERS, "WSAEUSERS - Too many users", EUSERS }, 
{WSAEDQUOT, "WSAEDQUOT - Quota exceeded", EDQUOT }, 
{WSAESTALE, "WSAESTALE - Stale NFS file handle", ESTALE }, 
{WSAEREMOTE, "WSAEREMOTE - Object is remote", EREMOTE }, 
/* 
* Extended Windows Sockets error constant definitions 
*/ 
{WSASYSNOTREADY, "WSASYSNOTREADY - Network subsystem not ready", 0}, 
{WSAVERNOTSUPPORTED, "WSAVERNOTSUPPORTED - Version not supported", 0}, 
{WSANOTINITIALISED, "WSANOTINITIALISED - WSAStartup() has not been successfully called", 0}, 
/* 
* Other error constants. 
*/ 
{WSAHOST_NOT_FOUND, "WSAHOST_NOT_FOUND - Host not found", 0}, 
{WSATRY_AGAIN, "WSATRY_AGAIN - Host not found or SERVERFAIL", 0}, 
{WSANO_RECOVERY, "WSANO_RECOVERY - Non-recoverable error", 0}, 
{WSANO_DATA, "WSANO_DATA - (or WSANO_ADDRESS) - No data record of requested type", 0}, 
{-1, "UNKNOWN ERROR", -1}
}; 
