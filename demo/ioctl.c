/*
    ioctl.c
    Copyright 1998, 1999 by Richard Dawe

    Demonstration of libsocket's support of some BSD-ish and other Unices'
    socket ioctls. This was added at a request and the support isn't total
    yet.
*/

#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>

#include <sys/ioctl.h>
#include <ioctls.h>         /* For Linux ioctl def's */
#include <net/if.h>         /* For Linux ioctl def's */

/* RD: This should compile under Linux with this define. We need to include
   lsck/lsck.h for lsck_perror(). */
#ifdef DJGPP
#include <lsck/lsck.h>
#else
#define lsck_perror perror
#endif
/* End RD */

int main (int argc, char *argv[])
{
    char name[IFNAMSIZ];    /* IFNAMSIZ defined by sys/socket.h     */
    struct ifreq ifr;       /* struct ifreq defined by sys/socket.h */
    int sock, ret;
    char               *webserver;
    struct hostent     *webserver_h;
    struct sockaddr_in  webserver_sin;

    /* --- Socket functions --- */

    /* If a command-line parameter specified, use it as a web server name. */
    webserver = (argc > 1) ? argv[1] : NULL;

    /* Create a socket */
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    /* Try and connect the socket, if a name given. */
    if (webserver != NULL) {
        /* Find the address */
        webserver_h = gethostbyname(webserver);

        if (webserver_h != NULL) {
            /* Build up a socket address */
            webserver_sin.sin_family = AF_INET;
            webserver_sin.sin_addr   = * ((struct in_addr *) webserver_h->h_addr);
            webserver_sin.sin_port   = htons(80);       /* Assume port 80 */

            /* Connect */
            connect(sock, (struct sockaddr *) &webserver_sin, sizeof(webserver_sin));
        }
    }

    /* --- Test the Linux ioctls --- */
    /* Sorry about the ugly typecasts needed */

    /* Get the interface name */
    if ((ret = ioctl(sock, SIOCGIFNAME, (int *) name)) == -1)
        lsck_perror("ioctl");
    else
        printf("libsocket interface name: '%s'\n", name);

    /* Get the socket address at this end */
    if ((ret = ioctl(sock, SIOCGIFADDR, (int *) &ifr)) == -1)
        lsck_perror("ioctl");
    else
        printf("Local address:            '%s'\n",
               inet_ntoa( ((struct sockaddr_in *) &ifr.ifr_ifru.ifru_addr)->sin_addr ));

    /* Get the socket address at the other end (the peer) */
    if ((ret = ioctl(sock, SIOCGIFDSTADDR, (int *) &ifr)) == -1)
        lsck_perror("ioctl");
    else
        printf("Peer address:             '%s'\n",
               inet_ntoa( ((struct sockaddr_in *) &ifr.ifr_ifru.ifru_dstaddr)->sin_addr ));

    /* Get the netmask */
    if ((ret = ioctl(sock, SIOCGIFNETMASK, (int *) &ifr)) == -1)
        lsck_perror("ioctl");
    else
        printf("Netmask:                  '%s'\n",
               inet_ntoa( ((struct sockaddr_in *) &ifr.ifr_ifru.ifru_netmask)->sin_addr ));

    /* Done */
    close(sock);
    return( (ret == -1) ? 0 : 1);
}
