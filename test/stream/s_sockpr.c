/*
 * d_sockpr.c - Test socketpair() for datagrams.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>

void die (char *msg) {
	fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}

int main (int argc, char *argv[])
{
	char msg[] = "Hello via a socket-pair.\n";
	char buf[256];
	int u_fd[2];
	int ret;
	
	/* Unix datagrams */
	ret = socketpair(AF_UNIX, SOCK_STREAM, 0, u_fd);
	if (ret == -1) die("Unable to create AF_UNIX socket-pair.");

	ret = send(u_fd[0], msg, sizeof(msg), 0);
	if (ret == -1) die("send() through u_fd[0] failed.");
	printf("send through u_fd[0] returned %i\n", ret);
	printf("Sent message: '%s'\n", msg);

	bzero(buf, sizeof(buf));
	ret = recv(u_fd[1], buf, sizeof(buf), 0);
	if (ret == -1) die("recv() through u_fd[1] failed.");
	printf("recv through u_fd[1] returned %i\n", ret);
	printf("Received message: '%s'\n", buf);

	/* Close Unix sockets */
	close(u_fd[0]);
	close(u_fd[1]);

	return(EXIT_SUCCESS);
}
