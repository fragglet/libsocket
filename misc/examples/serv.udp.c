/*
 *          		S E R V . U D P
 *
 *	This is an example program that demonstrates the use of
 *	datagram sockets as an IPC mechanism.  This contains the server,
 *	and is intended to operate in conjunction with the client
 *	program found in client.udp.  Together, these two programs
 *	demonstrate many of the features of sockets, as well as good
 *	conventions for using these features.
 *
 *	This program provides a service called "example".  It is an
 *	example of a simple name server.  In order for
 *	it to function, an entry for it needs to exist in the
 *	/etc/services file.  The port address for this service can be
 *	any port number that is likely to be unused, such as 22375,
 *	for example.  The host on which the client will be running
 *	must also have the same entry (same port number) in its
 *	/etc/services file.
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>

int s;				/* socket descriptor */

#define BUFFERSIZE	1024	/* maximum size of packets to be received */
int cc;				/* contains the number of bytes read */
char buffer[BUFFERSIZE];	/* buffer for packets to be read into */

struct hostent *hp;		/* pointer to host info for requested host */
struct servent *sp;		/* pointer to service information */

struct sockaddr_in myaddr_in;	/* for local socket address */
struct sockaddr_in clientaddr_in;	/* for client's socket address */
struct in_addr reqaddr;		/* for requested host's address */

#define ADDRNOTFOUND	0xffffffff	/* return address for unfound host */

/*
 *			M A I N
 *
 *	This routine starts the server.  It forks, leaving the child
 *	to do all the work, so it does not have to be run in the
 *	background.  It sets up the socket, and for each incoming
 *	request, it returns an answer.  Each request consists of a
 *	host name for which the requester desires to know the
 *	internet address.  The server will look up the name in its
 *	/etc/hosts file, and return the internet address to the
 *	client.  An internet address value of all ones will be returned
 *	if the host name is not found.
 *
 */
main(argc, argv)
int argc;
char *argv[];
{
	int addrlen;

		/* clear out address structures */
	memset ((char *)&myaddr_in, 0, sizeof(struct sockaddr_in));
	memset ((char *)&clientaddr_in, 0, sizeof(struct sockaddr_in));

		/* Set up address structure for the socket. */
	myaddr_in.sin_family = AF_INET;
		/* The server should receive on the wildcard address,
		 * rather than its own internet address.  This is
		 * generally good practice for servers, because on
		 * systems which are connected to more than one
		 * network at once will be able to have one server
		 * listening on all networks at once.  Even when the
		 * host is connected to only one network, this is good
		 * practice, because it makes the server program more
		 * portable.
		 */
	myaddr_in.sin_addr.s_addr = INADDR_ANY;
		/* Find the information for the "example" server
		 * in order to get the needed port number.
		 */
	sp = getservbyname ("example", "udp");
	if (sp == NULL) {
		printf("%s: example not found in /etc/services\n",
				argv[0]);
		exit(1);
	}
	myaddr_in.sin_port = sp->s_port;

		/* Create the socket. */
	s = socket (AF_INET, SOCK_DGRAM, 0);
	if (s == -1) {
		perror(argv[0]);
		printf("%s: unable to create socket\n", argv[0]);
		exit(1);
	}
		/* Bind the server's address to the socket. */
	if (bind(s, &myaddr_in, sizeof(struct sockaddr_in)) == -1) {
		perror(argv[0]);
		printf("%s: unable to bind address\n", argv[0]);
		exit(1);
	}

		/* Now, all the initialization of the server is
		 * complete, and any user errors will have already
		 * been detected.  Now we can fork the daemon and
		 * return to the user.  We need to do a setpgrp
		 * so that the daemon will no longer be associated
		 * with the user's control terminal.  This is done
		 * before the fork, so that the child will not be
		 * a process group leader.  Otherwise, if the child
		 * were to open a terminal, it would become associated
		 * with that terminal as its control terminal.  It is
		 * always best for the parent to do the setpgrp.
		 */
	setpgrp();

	switch (fork()) {
	case -1:		/* Unable to fork, for some reason. */
		perror(argv[0]);
		printf("%s: unable to fork daemon\n", argv[0]);
		exit(1);

	case 0:			/* The child process (daemon) comes here. */
			/* Close stdin, stdout, and stderr so that they will
			 * not be kept open.  From now on, the daemon will
			 * not report any error messages.  This daemon
			 * will loop forever, waiting for requests and
			 * responding to them.
			 */
		close(stdin);
		close(stdout);
		close(stderr);
			/* This will open the /etc/hosts file and keep
			 * it open.  This will make accesses to it faster.
			 */
		sethostent(1);
		for(;;) {
				/* Note that addrlen is passed as a pointer
				 * so that the recvfrom call can return the
				 * size of the returned address.
				 */
			addrlen = sizeof(struct sockaddr_in);
				/* This call will block until a new
				 * request arrives.  Then, it will
				 * return the address of the client,
				 * and a buffer containing its request.
				 * BUFFERSIZE - 1 bytes are read so that
				 * room is left at the end of the buffer
				 * for a null character.
				 */
			cc = recvfrom(s, buffer, BUFFERSIZE - 1, 0,
						&clientaddr_in, &addrlen);
			if ( cc == -1) exit(1);
				/* Make sure the message received is
				 * null terminated.
				 */
			buffer[cc]='\0';
				/* Treat the message as a string containing
				 * a hostname.  Search for the name in
				 * /etc/hosts.
				 */
			hp = gethostbyname (buffer);
			if (hp == NULL) {
					/* Name was not found.  Return a
					 * special value signifying the
					 * error.
					 */
				reqaddr.s_addr = ADDRNOTFOUND;
			} else {
					/* Copy address of host into the
					 * return buffer.
					 */
				reqaddr.s_addr =
				       ((struct in_addr *)(hp->h_addr))->s_addr;
			}
				/* Send the response back to the
				 * requesting client.  The address
				 * is sent in network byte order. Note that
				 * all errors are ignored.  The client
				 * will retry if it does not receive
				 * the response.
				 */
			sendto (s, &reqaddr, sizeof(struct in_addr),
					0, &clientaddr_in, addrlen);
		}

	default:		/* Parent process comes here. */
		exit(0);
	}
}
