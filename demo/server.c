/*
 * Demo programme for libsocket for DJGPP
 * This demonstrates the use of libsocket.
 * This is server.
 * This file will compile also under Linux.
 *
 * Copyright 1997, 1998 by Indrek Mandre
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

int main()
{
 struct sockaddr_in in;
 struct sockaddr_in peer_in;
 int sock;
 char PORT[256];
 char MESSAGE[256];
 int newsock;
 int len;
 char buf[256];

 /*
  * Now we read from user on which port he wants the server crawl and
  * which message will be sent to clients.
  */

 printf ("Enter port number: ");
 gets (PORT);

 printf ("Enter message we send to clients: ");
 gets (MESSAGE);

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

 bind ( sock, (struct sockaddr *)&in, sizeof ( struct sockaddr_in ) );

 listen ( sock, 5 );

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

	newsock = accept ( sock, (struct sockaddr *)&peer_in, &len );

	recv ( newsock, buf, 256, 0 );
	send ( newsock, MESSAGE, 256, 0 );

	printf ("From %s: %s\n", inet_ntoa ( peer_in.sin_addr ), buf );

	close ( newsock );
 }

}

