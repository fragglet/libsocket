/*
 * netservtest <C> 1997 Indrek Mandre
 * This test programme finds services by name from file services in windows
 * root directory.
 */

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int main()
{
 char x[100];
 struct servent *s;

 for (;;) {

  printf ("Enter service name: ");
  gets (x);

  if ( ( s = getservbyname ( x, "tcp" ) ) != NULL ) {

 	 printf ("Aliases:[ ");

 	 while ( *(s->s_aliases) ) {
 	 	printf ("%s ", *s->s_aliases );
 	 	s->s_aliases++;
 	 }

 	 printf ("] s_name: %s s_port: %d s_proto: %s\n", s->s_name, htons( s->s_port ), s->s_proto );

  } else printf ("NULL\n");
 }
 return 0;
}

