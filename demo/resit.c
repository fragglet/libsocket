/*
 * Demo programme for libsocket for DJGPP.
 * Sample programme shows how to resolve names.
 * This file will compile and work also on Linux.
 *
 * Copyright 1997, 1998 by Indrek Mandre
 */

/*
   Some modifications by Richard Dawe (RD).
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

int main (int argc, char *argv[])
{
    struct hostent *hpke;
    char *x;

    /* RD: Allow command-line params */
    if (argc > 1) {
        x = argv[1];
    /* RD: Back to Indrek's code */
    } else {
        x = malloc ( 100 );
        printf ("Please give me the name to resolve: ");
        gets (x);
    }

    /* RD: This is no longer in the library! */
    /* WS_DNS_name ( "your.dns.ip.address" ); */

    hpke = gethostbyname ( x );

    if ( hpke == NULL ) { 
        printf ("Unable to resolve hostname '%s'\n", x);
        herror ( "gethostbyname" );
        return -1;
    }

    printf ("H name: %s\n", hpke->h_name );

    printf ("Aliases: ");
    while ( *(hpke->h_aliases ) ) {
        printf ("%s ", *hpke->h_aliases );
        hpke->h_aliases ++;
    }

    printf ("\nAddresses: ");
     while ( *(hpke->h_addr_list ) ) {
        printf ("%s ", inet_ntoa ( *((struct in_addr *)*(hpke->h_addr_list)) ));
        hpke->h_addr_list ++;
    }

    /*
     * When you want to use resolving in real life - resolve a name and then
     * connect to it then do so:
     * first gethostbyname and after that in struct sockaddr_in
     * to element sin_addr.s_addr = ((struct in_addr *)hpke->h_addr))->s_addr;
     * where hpke is the name of your hostent * structure.
     * Little messy wording but I hope you understand. You can also have a look on
     * the examples in example directory.
     */

    printf ("\n");

    return 0;
}

