/*
 * Demo programme for libsocket for DJGPP
 * This demonstrates the use of libsocket.
 * This is client.
 * This file will compile also under Linux.
 *
 * Copyright 1997, 1998 by Indrek Mandre
 */

/*
    1998-12-15: Richard Dawe: Modified it to work like a teletype program to
                work with the non-blocking server, servern.
*/    

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>

/* RD: This should compile under Linux with this define. We need to include
   lsck/lsck.h for lsck_perror(). */
#ifdef DJGPP
#include <lsck/lsck.h>
#else
#define lsck_perror perror
#endif
/* End RD */

#include <conio.h>      /* RD: For getch() */

int main()
{
    struct sockaddr_in in;
    int sock;
    char HOST[256];
    char PORT[256];
    char buf[256];
    struct hostent *hent;
    unsigned long int haddress;
    int ret;

    /*
     * Now we read from user where the server is situated and on what port and
     * the message wanted to send to server.
     */

    /* RD: Allow names to be given */
    printf ("Enter server name or IP address: ");
    gets (HOST);

    /* RD: Resolve it */
    hent = gethostbyname(HOST);
    if (hent == NULL) { herror("client"); exit(0); }
    haddress = ((struct in_addr *) hent->h_addr)->s_addr;

    printf ("Enter port number: ");
    gets (PORT);

    /*
     * At first we have to create socket.
     * After that we connect it to server.
     */

    memset ( &in, 0, sizeof ( struct sockaddr_in ) );

    sock = socket ( AF_INET, SOCK_STREAM, 0 );

    in.sin_family = AF_INET;
    in.sin_addr.s_addr = haddress;
    /*in.sin_addr.s_addr = inet_addr ( HOST );*/
    in.sin_port = htons ( atoi ( PORT ) );

    /* RD: Status message */
    printf("Connecting to %s (%s)...\n", HOST, inet_ntoa(in.sin_addr));

    if (connect(sock,
		(struct sockaddr *) &in, sizeof(struct sockaddr_in)) == -1) {
	    lsck_perror("clientn");
	    return(1);
    }

    /* RD: Status message */
    printf("Connected - please enter some data for the server.\n");

    /*
     * Now we send the message as the user types it.
     */

    for (;;) {
        ret = getch();
        if (ret == -1) continue;

        buf[0] = (char) ret;
        buf[1] = '\0';
        printf("%s", buf);
        fflush(stdout);

        ret = send(sock, buf, strlen(buf) + 1, 0);
        if (ret == -1) {
            lsck_perror("clientn");
            break;
        }
    }

    close(sock);

    return 0;
}

