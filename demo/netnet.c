/*
 * netnettest Copyright 1997, 1998 by Indrek Mandre and Richard Dawe
 * Based on Indrek Mandre's netprototest program. Modified by Richard Dawe.
 * This test programme finds networks by name from file networks in windows
 * root directory.
 */

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int main()
{
 char x[100];
 struct netent *s;

 for (;;) {

  printf ("Enter network name: ");
  gets (x);

  if ( ( s = getnetbyname ( x ) ) != NULL ) {

 	 printf ("Aliases:[ ");

     while ( *(s->n_aliases) ) {
        printf ("%s ", *s->n_aliases );
        s->n_aliases++;
 	 }

     printf ("] n_name: %s n_net: %lx\n", s->n_name, s->n_net );

  } else printf ("NULL\n");
 }
 return 0;
}

