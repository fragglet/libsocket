
/*
 * Demo programme for libsocket for DJGPP <C> 1998 Indrek Mandre.
 * Sample programme shows how to use select. This is C language specific.
 * That means it is described in all C books.
 * Select lets you to check if there's anything to read from some
 * file descriptor or wait until something arrives or wait some time
 * something to arrive. In libsocket there's no difference between
 * socket descriptor and file descriptor. You can use the same functions on
 * both of them (eg. select). To test if it's right to do accept use the same
 * way. And please read documentation of djgpp. It's explained there more
 * closely.
 * This file will compile and work also on Linux.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>     /* RD: Needed for memset */
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>

int main()
{
 fd_set fd;
 struct timeval tv;
 char str[256];

 printf ("Demo program for libsocket. Demonstrates the basic use of select.\n");
 printf ("It waits from user input and prints dots at the same time.\n");
 printf ("Here we go...\n");

 for (;;)
 {
     FD_ZERO ( &fd ); /* We make all the bits to zero */
     FD_SET ( fileno ( stdin ), &fd ); /* We want to check if there's something to read */

 tv.tv_usec = 0;
 tv.tv_sec = 1;     /* We wait 1 seconds for the data. If all are zeros then
                       we wait 0 seconds and 0 milliseconds. If we pass
                       NULL to select as tv, then we'll wait for ever */

     select ( fileno (stdin) + 1, &fd, 0, 0, &tv );

     if ( FD_ISSET ( fileno ( stdin ), &fd ) )
     {
     	gets ( str );
     	printf ( "Got it: [%s]\n", str );
     }

     printf (".");
     fflush ( stdout );
 }

}

