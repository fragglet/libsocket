/*
 * netprototest <C> 1997 Indrek Mandre
 * This test programme finds protocols by name from file protocol in windows
 * root directory.
 */

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int main()
{
 char x[100];
 struct protoent *s;

 for (;;) {

  printf ("Enter protocol name: ");
  gets (x);

  if ( ( s = getprotobyname ( x ) ) != NULL ) {

 	 printf ("Aliases:[ ");

 	 while ( *(s->p_aliases) ) {
 	 	printf ("%s ", *s->p_aliases );
 	 	s->p_aliases++;
 	 }

 	 printf ("] p_name: %s p_proto: %d\n", s->p_name, s->p_proto );

  } else printf ("NULL\n");
 }
 return 0;
}

