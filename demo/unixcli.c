/*
    unixcli.c
    Copyright 1999 by Richard Dawe

    Unix domain sockets client
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define TESTMSG "Hello from unixcli"

int main (int argc, char *argv[])
{
    struct sockaddr_un sun;
    int sock, ret;

    /* Create the socket */
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1) { perror(argv[0]); exit(EXIT_FAILURE); }

    /* Connect to the server */
    sun.sun_family = AF_UNIX;
    strcpy(sun.sun_path, "/tmp/unixdemo");
    ret = connect(sock, (struct sockaddr *) &sun, sizeof(sun));
    if (ret == -1) { perror(argv[0]); exit(EXIT_FAILURE); }
    printf("Connected to server\n");

    ret = send(sock, TESTMSG, sizeof(TESTMSG) + 1, 0);
    printf( (ret != -1) ? "Sent message OK\n" : "Failed to send message\n");

    /* Finished */
    close(sock);
    printf("Connection closed\n");
    return(EXIT_SUCCESS);
}
