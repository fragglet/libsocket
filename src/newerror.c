/*
    newerror.c - New libsocket errors for libc
    Written by Richard Dawe
*/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>

#include <lsck/lsck.h>

/* ---------------
   - lsck_perror -
   --------------- */

void lsck_perror (char *s)
{
    fprintf(stderr, "%s: %s\n", s, lsck_strerror(errno));
}

/* -----------------
   - lsck_strerror -
   ----------------- */

char *lsck_strerror (int errnum)
{
    /* Return our own error message if there is one, else use the libc
       routine. */
    switch(errnum) {
        case ELOOP:
            return("(ELOOP)");

        case EREMOTE:
            return("Object is remote (EREMOTE)");

        case EPROTOTYPE:
            return("(EPROTOTYPE)");

        case EUSERS:
            return("Too many users (EUSERS)");

        case ENOTSOCK:
            return("Not a socket (ENOTSOCK)");

        case EDESTADDRREQ:
            return("(EDESTADDRREQ)");

        case EMSGSIZE:
            return("(EMSGSIZE)");

        case ENOPROTOOPT:
            return("(ENOPROTOOPT)");

        case EPROTONOSUPPORT:
            return("Protocol not supported (EPROTONOSUPPORT)");

        case ESOCKTNOSUPPORT:
            return("Socket type not supported (ESOCKTNOSUPPORT)");

        case EOPNOTSUPP:
            return("(EOPNOTSUPP)");

        case EPFNOSUPPORT:
            return("Protocol family not supported (EPFNOSUPPORT)");

        case EAFNOSUPPORT:
            return("Address family not supported (EAFNOSUPPORT)");

        case EADDRINUSE:
            return("Address already in use (EADDRINUSE)");
    
        case EADDRNOTAVAIL:
            return("Cannot assign requested address (EADDRNOTAVAIL)");

        case ENETDOWN:
            return("Network is down (ENETDOWN)");

        case ENETUNREACH:
            return("Network is unreachable (ENETUNREACH)");

        case ENETRESET:
            return("Network dropped connection because of reset (ENETRESET)");

        case ECONNABORTED:
            return("Software caused connection abort (ECONNABORTED)");

        case ECONNRESET:
            return("Connection reset by peer (ECONNRESET)");

        case ENOBUFS:
            return("No buffer space available (ENOBUFS)");

        case EISCONN:
            return("Transport endpoint is already connected (EISCONN)");

        case ENOTCONN:
            return("Transport endpoint is not connected (ENOTCONN)");

        case ESHUTDOWN:
            return("Cannot send after transport endpoint shutdown (ESHUTDOWN)");

        case ETOOMANYREFS:
            return("Too many references: cannot splice (ETOOMANYREFS)");

        case ETIMEDOUT:
            return("Connection timed out (ETIMEOUT)");

        case ECONNREFUSED:
            return("Connection refused (ECONNREFUSED)");

        case EHOSTDOWN:
            return("Host is down (EHOSTDOWN)");

        case EHOSTUNREACH:
            return("No route to host (EHOSTUNREACH)");

        case ESTALE:
            return("Stale NFS handle (ESTALE)");

        case EDQUOT:
            return("Quota exceeded (EDQUOT)");

        case EALREADY:
        case EINPROGRESS:
        default:
            return(strerror(errnum));
    }

    /* Safety net */
    return(NULL);
}
