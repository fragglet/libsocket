/*
 * solist.c
 * Copyright (C) 1999, 2000 by Richard Dawe <richdawe@bigfoot.com>
 *
 * Create & dump socket options for TCP/IP and UDP/IP unconnected sockets.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>

typedef struct {
	unsigned int opt;
	char *desc;
} SO_INFO;

/* These socket options return integer values. */
SO_INFO int_sos[] = {
#ifdef SO_DEBUG
	{ SO_DEBUG, 		"SO_DEBUG:\tDebugging info recording" },
#endif
#ifdef SO_ACCEPTCONN
	{ SO_ACCEPTCONN, 	"SO_ACCEPTCONN:\tlisten() performed" },
#endif
#ifdef SO_REUSEADDR
	{ SO_REUSEADDR,		"SO_REUSEADDR:\tLocal address re-use" },
#endif
#ifdef SO_KEEPALIVE
	{ SO_KEEPALIVE,		"SO_KEEPALIVE:\tKeep connections alive" },
#endif
#ifdef SO_DONTROUTE
	{ SO_DONTROUTE,
	  "SO_DONTROUTE:\tUse interface addresses only" },
#endif
#ifdef SO_BROADCAST
	{ SO_BROADCAST,
	  "SO_BROADCAST:\tPermit sending of broadcast messages" },
#endif
#ifdef SO_USELOOPBACK
	{ SO_USELOOPBACK,
	  "SO_USELOOPBACK:\tBypass hardware when possible" },
#endif
#ifdef SO_SNDBUF
	{ SO_SNDBUF,		"SO_SNDBUF:\tSend buffer size" },
#endif
#ifdef SO_RCVBUF
	{ SO_RCVBUF,		"SO_RCVBUF:\tReceive buffer size" },
#endif
#ifdef SO_SNDLOWAT
	{ SO_SNDLOWAT,		"SO_SNDLOWAT:\tSend low-water mark" },
#endif
#ifdef SO_RCVLOWAT
	{ SO_RCVLOWAT,		"SO_RCVLOWAT:\tReceive low-water mark" },
#endif
#ifdef SO_ERROR
	{ SO_ERROR,
	  "SO_ERROR:\tGet pending error status & clear it" },
#endif
#ifdef SO_TYPE
	{ SO_TYPE,	  	"SO_TYPE:\tGet socket type" },
#endif
	{ 0, NULL }
};

SO_INFO timeval_sos[] = {
#ifdef SO_SNDTIMEO
	{ SO_SNDTIMEO,		"SO_SNDTIMEO:\tSend time-out" },
#endif
#ifdef SO_RCVTIMEO
	{ SO_RCVTIMEO,		"SO_RCVTIMEO:\tReceive time-out" },
#endif
	{ 0, NULL }	
};

SO_INFO linger_sos[] = {
#ifdef SO_LINGER
	{ SO_LINGER,		"SO_LINGER:\tLinger when socket closed" },
#endif
	{ 0, NULL }
};

void dump_sos (int sock);

/* --------
 * - main -
 * -------- */

int main (int argc, char *argv[])
{
	int sock;

	/* Create a stream socket. */
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == -1) {
		printf("- Failed to create a TCP stream socket.\n");
		perror("solist");
	} else {
		printf(". TCP stream socket default options:\n");
		dump_sos(sock);
	}
	close(sock);

	/* Create a datagram socket. */
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == -1) {
		printf("- Failed to create a UDP datagram socket.\n");
		perror("solist");
	} else {
		printf(". UDP datagram socket default options:\n");
		dump_sos(sock);
	}
	close(sock);	
	
	return(EXIT_SUCCESS);
}

/* ------------
 * - dump_sos -
 * ------------ */

void dump_sos (int sock)
{
	int i, ret;
	long val;
	struct timeval tv;
	struct linger l;
	size_t val_len;

	/* Integer values */
	for (i = 0; int_sos[i].opt > 0; i++) {
		/* Get the socket option. */
		val     = 0;
		val_len = sizeof(val);		
		ret     = getsockopt(sock, SOL_SOCKET, int_sos[i].opt,
				     &val, &val_len);

		printf("%s: ", int_sos[i].desc);
		if (ret == -1) {
			printf("%s\n",
			       (errno == ENOPROTOOPT)
			       ? "UNSUPPORTED"
			       : "FAILED");
		} else {
			printf("%li\n", val);
		}
	}

	/* struct timeval values */
	for (i = 0; timeval_sos[i].opt > 0; i++) {
		/* Get the socket option. */
		bzero(&tv, sizeof(tv));
		val_len = sizeof(tv);		
		ret     = getsockopt(sock, SOL_SOCKET, timeval_sos[i].opt,
				     (void *) &tv, &val_len);

		printf("%s: ", timeval_sos[i].desc);
		if (ret == -1) {
			printf("%s\n",
			       (errno == ENOPROTOOPT)
			       ? "UNSUPPORTED"
			       : "FAILED");
		} else {
			if ((tv.tv_sec == 0)
			    && (tv.tv_usec == 0)) {
				printf("No time-out\n");
			} else {
				printf("%i seconds %li microseconds\n",
				       tv.tv_sec, tv.tv_usec);
			}
		}
	}

	/* struct linger values */
	for (i = 0; linger_sos[i].opt > 0; i++) {
		/* Get the socket option. */
		bzero(&l, sizeof(l));
		val_len = sizeof(l);		
		ret     = getsockopt(sock, SOL_SOCKET, linger_sos[i].opt,
				     (void *) &l, &val_len);

		printf("%s: ", linger_sos[i].desc);
		if (ret == -1) {
			printf("%s\n",
			       (errno == ENOPROTOOPT)
			       ? "UNSUPPORTED"
			       : "FAILED");
		} else {
			if (l.l_onoff == 0) {
				printf("Disabled - normal close\n");
			} else {
				if (l.l_linger == 0) {
					printf("Enabled - forcibly close\n");
				} else {
					printf("Enabled - forcibly close"
					       "after %i time units "
					       "(seconds?)\n",
					       l.l_linger);
				}
			}
		}
	}
}
