/*
 * Demo programme for libsocket for DJGPP
 * This demonstrates the use of libsocket.
 * This is server.
 * This file will compile also under Linux.
 *
 * Copyright 1997, 1998 by Indrek Mandre
 */

/*
    1998-12-15: Richard Dawe: Modified this to use non-blocking sockets, as
                a demonstration of how to use them. Also added checking of
                return status of bind() & listen().
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

/* RD: This should compile under Linux with this define. We need to include
   lsck/lsck.h for lsck_perror(). */
#ifdef DJGPP
#include <lsck/lsck.h>
#else
#define lsck_perror perror
#endif
/* End RD */

#include <fcntl.h>      /* RD: For fcntl() -> non-blocking */

int
main (void)
{
  struct sockaddr_in in;
  struct sockaddr_in peer_in;
  int sock;
  char PORT[256];
  int newsock;
  size_t len;
  char buf[256];

  int t;                          /* RD: Length of received input */
  int star_printed;               /* RD: Print a star after to show
				   * non-blocking */

  /*
   * Now we read from user on which port he wants the server crawl and
   * which message will be sent to clients.
   */

  *PORT = '\0';
  while (atoi(PORT) == 0) {
    printf ("Enter port number: ");
    gets (PORT);
  }

  /*
   * At first we have to create socket.
   * After that we bind it on desired port and after that we begin
   * to listen it.
   */

  memset ( &in, 0, sizeof ( struct sockaddr_in ) );

  sock = socket ( AF_INET, SOCK_STREAM, 0 );

  in.sin_family = AF_INET;
  in.sin_addr.s_addr = INADDR_ANY;
  in.sin_port = htons ( atoi ( PORT ) );

  /* RD: Always check return values! */
  if (bind(sock, (struct sockaddr *) &in, sizeof (struct sockaddr_in)) == -1) {
    lsck_perror("bind");
    exit(1);
  }

  /* RD: Always check return values! */
  if (listen (sock, 5) == -1) {
    lsck_perror("listen");
    exit(1);
  }

  /*
   * Infinite loop.
   * We use accept to get new client. The socket is blocking, so accept
   * waits while new client arrives. Accept returns the new socket's
   * descriptor. peer_in will be filled with the address of the client.
   * We receive the message that client send and send our message back.
   * After that we close the connection.
   */

  /*
   * RD: Actually, let's accept in a non-blocking manner, to allow the
   *     program to be killed by Ctrl+C.
   */

  fcntl(sock, F_SETFL, O_NONBLOCK);
  printf("Waiting to accept() connection\n");

  for (;;) {
    len = sizeof ( struct sockaddr_in );
    newsock = accept ( sock, (struct sockaddr *)&peer_in, &len );
    
    /* Do we have a connection or is it actually an error? */
    if (newsock == -1) {
      if (errno == EWOULDBLOCK) {
	continue;
      } else {
	lsck_perror("servern");
	break;
      }
    }

    printf ("From %s:\n", inet_ntoa(peer_in.sin_addr));

    /* Do receive as non-blocking */
    fcntl(newsock, F_SETFL, O_NONBLOCK);
    star_printed = 0;

    while( (t = recv(newsock, buf, 256, 0)) ) {
      if (t == -1) {
	if (errno == EWOULDBLOCK) {
	  if (!star_printed) {
	    printf("*");
	    fflush(stdout);
	    star_printed = 1;
	  }
	  continue;
	} else {
	  break;
	}
      }

      if (t <= 0) continue;
      buf[t] = 0;
      printf("%s", buf);
      fflush(stdout);
      star_printed = 0;
    }

    /* Tidy up */
    close (newsock);

    /* Print status message */	
    printf("\n"
	   "Connection closed\n"
	   "Waiting to accept() connection\n");
  }

  /* Done */
  return(0);
}
