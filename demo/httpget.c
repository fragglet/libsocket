
/*
 * Demo programme for libsocket for DJGPP.
 * Sample programme shows how to get html files from www servers.
 * This file will compile and work also on Linux.
 * News: added name resolving by RD
 *
 * Copyright 1997, 1998 by Indrek Mandre
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

static char REQUEST[2048];
static int PORT;
static unsigned long int HOST;
static int sock;

void show_usage ( char * );
void init_sockets ( void );
void send_request ( void );
void get_headers ( void );
void read_file ( void );

int main( int argc, char *argv[] )
{
 char address[256];
 char port[256];
 char file[1024];

 /* RD: Added host lookup var */
 struct hostent *hostlookup;
 /* End RD */

 if ( argc == 1 ) show_usage ( argv[0] );

 if ( strncasecmp ( argv[1], "http://", 7 ) != 0 ) show_usage ( argv[0] );

 argv[1] += 7;

 strncpy ( address, argv[1], strcspn ( argv[1], ":/" ) );
 address[strcspn ( argv[1], ":/" )] = 0;
 argv[1] += strcspn ( argv[1], ":/" );

 if ( *argv[1] == ':' ) {
 	argv[1]++;
 	strncpy ( port, argv[1], strcspn ( argv[1], "/" ) );
 	port[strcspn ( argv[1], "/" )] = 0;
 	argv[1] += strcspn ( argv[1], "/" );
 } else {
 	strcpy ( port, "80" );
 }

 strcpy ( file, argv[1] );
 if (*file == '\0') strcpy(file, "/");

 sprintf ( REQUEST, "GET %s HTTP/1.0\r\nUser-Agent: httpget/1.0\r\nHost: %s:%s\r\nAccept: */*\r\n\r\n", file, address, port );

 PORT = atoi ( port );

 /* RD: Modified to perform name lookup */
 hostlookup = gethostbyname(address);

 if (hostlookup == NULL)
 {
	fprintf (stderr, "Unable to resolve host name '%s'!\n", address);
    herror("httpget");
	exit(0);		/* IM: Todo: add herror */
 }
 
 HOST = ( (struct in_addr *) hostlookup->h_addr)->s_addr;
 /*fprintf(stderr, "HOST = %lu\n", HOST);*/

 /*HOST = inet_addr ( address );*/
 /* End RD */

 init_sockets ();

 send_request ();

 get_headers ();

 read_file ();

 close (sock);

 return 0;

}

void show_usage ( char *prname )
{
 printf ("Usage: %s http://host.ip.add.ress[:port]/file.name\n", prname );
 exit (0);
}

void init_sockets ()
{
 struct sockaddr_in in;

 memset ( &in, 0, sizeof ( struct sockaddr_in ) );

 if ( ( sock = socket ( AF_INET, SOCK_STREAM, 0 ) ) == -1 ) {
 	perror("socket");
 	exit (-1);
 }

 in.sin_family = AF_INET;
 in.sin_port = htons ( PORT );
 in.sin_addr.s_addr = HOST;

 if ( connect ( sock, (struct sockaddr *)&in, sizeof ( struct sockaddr_in ) ) == -1 )
 {
 	perror ( "connect" );
 	exit (-1);
 }

}

void send_request ()
{
  if ( send ( sock, REQUEST, strlen ( REQUEST ), 0 ) <= 0 ) {
  	perror ( "send" );
  	exit (0);
  }
}

void get_headers ()
{
 /*char BUF[0x10000];*/
 char BUF[1024];    /* RD: Try a smaller buffer */
 int t = 0;         /* RD: Safety */
 char *x = NULL;    /* RD: Safety */

 /* RD: sizeof(BUF) safer than fixed num. */
 while ((t = recv ( sock, BUF, sizeof(BUF) - 1, 0 )) > 0) {

 	BUF[t] = 0;

    x = strstr (BUF, "\r\n\r\n");
 			/* IM: This is wrong way, I know but I don't mind, it
 			   sometimes even works.. ok mostly */

    if (x != NULL) {
        x += 4, printf ("%s", x);
        return;
	}

    x = strstr(BUF, "\n\n");

    if (x != NULL) {
        x += 2, printf ("%s", x);
        return;
	}
 }
}

void read_file ()
{
 /*char BUF[0x10000];*/
 char BUF[1024];    /* RD: Try a smaller buffer */
 int t = 0;

 fflush ( stdout );

 /* RD: sizeof(BUF) safer than fixed num. Also, need sizeof(BUF) -1 to put
    in null terminator at each of buffer. */
 while ((t = recv ( sock, BUF, sizeof(BUF) - 1, 0 )) > 0) {
 	BUF[t] = 0;
    printf("%s", BUF);
    fflush(stdout);
 }
}
