/*
 * Demo programme for libsocket for DJGPP
 * This demonstrates the use of libsocket.
 * This is client.
 * This file will compile also under Linux.
 *
 * Copyright 1997, 1998 by Indrek Mandre
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>

int main()
{
 struct sockaddr_in in;
 int sock;
 char HOST[256];
 char PORT[256];
 char MESSAGE[256];
 char buf[256];
 struct hostent *hent;
 unsigned long int haddress;

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

 printf ("Enter message we send to server: ");
 gets (MESSAGE);

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

 connect ( sock, (struct sockaddr *)&in, sizeof ( struct sockaddr_in ) );

 /*
  * Now we send the message, wait server's message, print it and
  * after that we close connection.
  */

 send ( sock, MESSAGE, 256, 0 );
 printf ("Message sent...\n");

 recv ( sock, buf, 256, 0 );
 printf ("Received message: %s\n", buf );

 close ( sock );

 return 0;
}

