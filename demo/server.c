/*
 * Demo programme for libsocket for DJGPP
 * This demonstrates the use of libsocket.
 * This is server.
 * This file will compile also under Linux.
 *
 * Copyright 1997, 1998 by Indrek Mandre
 */

/*
   Some modification for libsocket 0.7.4 by Richard Dawe (RD).

   This program now uses lsck_perror() to print error messages. This is because
   DJGPP's libc does not print error messages for socket errors. If you have
   installed libsocket's libc patch, then perror() will also work.

   It is important to check for error returns. When using server functions like
   accept(), bind(), listen() with libsocket, one can have strange problems if
   a previous function has failed - the program just freezes.
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

int
main (void) /* RD: void */
{
  struct sockaddr_in in;
  struct sockaddr_in peer_in;
  int sock;
  char PORT[256];
  char MESSAGE[256];
  int newsock;
  size_t len;
  char buf[256];

  /*struct linger pl;
    int pl_len;*/
  int ret;

  /*
   * Now we read from user on which port he wants the server crawl and
   * which message will be sent to clients.
   */

  *PORT = '\0';
  while (atoi(PORT) == 0) {
    printf ("Enter port number: ");
    gets (PORT);
  }

  *MESSAGE = '\0';
  while (*MESSAGE == '\0') {
    printf ("Enter message we send to clients: ");
    gets (MESSAGE);
  }

  /*
   * At first we have to create socket.
   * After that we bind it on desired port and after that we begin
   * to listen it.
   */

  memset ( &in, 0, sizeof ( struct sockaddr_in ) );

  /* RD: 0 is another name for IPPROTO_TCP - it is better to use
   * IPPROTO_TCP! */
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

  for (;;) {
    len = sizeof ( struct sockaddr_in );

    /* RD: Always check return values! */
    newsock = accept ( sock, (struct sockaddr *)&peer_in, &len );
    if (newsock == -1) {
      lsck_perror("accept");
      exit(1);
    }
  
    /* RD: Status message */
    printf("Accepted connection from %s\n", inet_ntoa(peer_in.sin_addr));
    fflush(stdout);

    /* Set lingering to be on */
    /*pl.l_onoff = 1;
      pl.l_linger  = 0xFFFF;
      pl_len = sizeof(pl);
      setsockopt(newsock, SOL_SOCKET, SO_LINGER, &pl, pl_len);

      bzero(&pl, sizeof(pl));
      getsockopt(newsock, SOL_SOCKET, SO_LINGER, &pl, &pl_len);
      printf("onoff = %i, linger = %i, len = %i\n",
      pl.l_onoff, pl.l_linger, pl_len);*/

    ret = recv ( newsock, buf, 256, 0 );
    if (ret >= 0) {
      buf[ret] = '\0';
      printf ("From %s: '%s'\n", inet_ntoa ( peer_in.sin_addr ), buf );
    } else {
      lsck_perror("server");
    }

    ret = send ( newsock, MESSAGE, strlen(MESSAGE) + 1, 0 );
    if (ret >= 0) {
      buf[ret] = '\0';
      printf ("To %s: '%s'\n", inet_ntoa ( peer_in.sin_addr ), MESSAGE);
    } else {
      lsck_perror("server");
    }

    close(newsock);
  }
}
