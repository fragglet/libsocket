/*
    unixserv.c
    Copyright 1999 by Richard Dawe

    Unix domain socket server demo for libsocket
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

int main (int argc, char *argv[])
{
    struct sockaddr_un sun, newsun;
    int sock, newsock, ret;
    size_t newsunlen;
    char buf[32768];

    /* Create the socket */
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1) { perror(argv[0]); exit(EXIT_FAILURE); }

    /* Bind to a local address */
    sun.sun_family = AF_UNIX;
    strcpy(sun.sun_path, "/tmp/unixdemo");
    ret = bind(sock, (struct sockaddr *) &sun, sizeof(sun));
    if (ret == -1) { perror(argv[0]); exit(EXIT_FAILURE); }

    /* Listen */
    ret = listen(sock, 0);
    if (ret == -1) { perror(argv[0]); exit(EXIT_FAILURE); }
    
    /* Accept connections */
    newsunlen = sizeof(newsun);
    printf("Accepting connections...\n");

    while ((newsock = accept(sock, (struct sockaddr *) &newsun, &newsunlen)) != -1) {
        /* Connection info */
        printf("Accepted connection from %s\n", newsun.sun_path);

        /* Get a message */
        ret = recv(newsock, buf, sizeof(buf), 0);
        if (ret > 0) printf("Message: '%s'\n", buf);

        /* Next! */
        close(newsock);
        newsunlen = sizeof(newsun);
    }

    if (newsock == -1) { perror(argv[0]); }

    /* Done */
    close(sock);
    return(EXIT_SUCCESS);
}
