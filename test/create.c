/*
 * create.c - Socket creation tests
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>

typedef struct {
	int domain;
	int type;
	int protocol;
} CREATE_COMBO;

CREATE_COMBO cc_wsock[] = {
	{ AF_INET,	SOCK_STREAM,	0 },
	{ AF_INET,	SOCK_STREAM,	IPPROTO_TCP },
	{ AF_INET,	SOCK_DGRAM,	0 },
	{ AF_INET,	SOCK_DGRAM,	IPPROTO_UDP },
	{ 0,		0,		0 }
};

CREATE_COMBO cc_csock[] = {
	{ AF_INET,	SOCK_STREAM,	0 },
	{ AF_INET,	SOCK_STREAM,	IPPROTO_TCP },
	{ AF_INET,	SOCK_DGRAM,	0 },
	{ AF_INET,	SOCK_DGRAM,	IPPROTO_UDP },
	{ 0,		0,		0 }
};

CREATE_COMBO cc_unix[] = {
	{ AF_UNIX,	SOCK_STREAM,	0 },
	{ AF_UNIX,	SOCK_DGRAM,	0 },
	{ 0,		0,		0 }
};

void die (char *msg) {
	fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}

int main (int argc, char *argv[])
{
#define MAX_CREATE_COMBOS 64
	CREATE_COMBO cc[MAX_CREATE_COMBOS];
	char buf[256];
	char *name = NULL;
	int ret, i;

	/* Get the test name */
	if (argc < 2) die("Syntax: create <interface>");
	name = argv[1];

	if (!stricmp(name, "wsock")) memcpy(&cc, &cc_wsock, sizeof(cc_wsock));
	if (!stricmp(name, "csock")) memcpy(&cc, &cc_csock, sizeof(cc_csock));
	if (!stricmp(name, "unix"))  memcpy(&cc, &cc_unix,  sizeof(cc_unix));

	for (i = 0; ; i++) {
		/* Done last test? */
		if (   (cc[i].domain   == 0)
		    && (cc[i].type     == 0)
		    && (cc[i].protocol == 0))
			break;

		/* Try creating the socket */
		ret = socket(cc[i].domain, cc[i].type, cc[i].protocol);
		if (ret == -1) {
			sprintf(buf,
			        "create: %s test %i failed: %s\n",
				name, i, strerror(errno));
			die(buf);
		}

		/* Try destroying the socket */
		ret = close(ret);
		if (ret == -1) {
			sprintf(buf,
			        "create: %s test %i failed: %s\n",
				name, i, strerror(errno));
			die(buf);
		}
	}

	return(EXIT_SUCCESS);
}
