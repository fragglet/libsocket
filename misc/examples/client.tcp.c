/*
 *			C L I E N T . T C P
 *
 *	This is an example program that demonstrates the use of
 *	stream sockets as an IPC mechanism.  This contains the client,
 *	and is intended to operate in conjunction with the server
 *	program found in serv.tcp.  Together, these two programs
 *	demonstrate many of the features of sockets, as well as good
 *	conventions for using these features.
 *
 *	This program requests a service called "example".  In order for
 *	it to function, an entry for it needs to exist in the
 *	/etc/services file.  The port address for this service can be
 *	any port number that is likely to be unused, such as 22375,
 *	for example.  The host on which the server will be running
 *	must also have the same entry (same port number) in its
 *	/etc/services file.
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>

int s;				/* connected socket descriptor */

struct hostent *hp;		/* pointer to host info for remote host */
struct servent *sp;		/* pointer to service information */

long timevar;			/* contains time returned by time() */
char *ctime();			/* declare time formatting routine */

struct sockaddr_in myaddr_in;	/* for local socket address */
struct sockaddr_in peeraddr_in;	/* for peer socket address */

/*
 *			M A I N
 *
 *	This routine is the client which request service from the remote
 *	"example server".  It creates a connection, sends a number of
 *	requests, shuts down the connection in one direction to signal the
 *	server about the end of data, and then receives all of the responses.
 *	Status will be written to stdout.
 *
 *	The name of the system to which the requests will be sent is given
 *	as a parameter to the command.
 */
main(argc, argv)
int argc;
char *argv[];
{
	int addrlen, i, j;

		/* This example uses 10 byte messages. */
	char buf[10];

	if (argc != 2) {
		fprintf(stderr, "Usage:  %s <remote host>\n", argv[0]);
		exit(1);
	}

		/* clear out address structures */
	memset ((char *)&myaddr_in, 0, sizeof(struct sockaddr_in));
	memset ((char *)&peeraddr_in, 0, sizeof(struct sockaddr_in));

		/* Set up the peer address to which we will connect. */
	peeraddr_in.sin_family = AF_INET;
		/* Get the host information for the hostname that the
		 * user passed in.
		 */
	hp = gethostbyname (argv[1]);
	if (hp == NULL) {
		fprintf(stderr, "%s: %s not found in /etc/hosts\n",
				argv[0], argv[1]);
		exit(1);
	}
	peeraddr_in.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
		/* Find the information for the "example" server
		 * in order to get the needed port number.
		 */
	sp = getservbyname ("example", "tcp");
	if (sp == NULL) {
		fprintf(stderr, "%s: example not found in /etc/services\n",
				argv[0]);
		exit(1);
	}
	peeraddr_in.sin_port = sp->s_port;

		/* Create the socket. */
	s = socket (AF_INET, SOCK_STREAM, 0);
	if (s == -1) {
		perror(argv[0]);
		fprintf(stderr, "%s: unable to create socket\n", argv[0]);
		exit(1);
	}
		/* Try to connect to the remote server at the address
		 * which was just built into peeraddr.
		 */
	if (connect(s, &peeraddr_in, sizeof(struct sockaddr_in)) == -1) {
		perror(argv[0]);
		fprintf(stderr, "%s: unable to connect to remote\n", argv[0]);
		exit(1);
	}
		/* Since the connect call assigns a random address
		 * to the local end of this connection, let's use
		 * getsockname to see what it assigned.  Note that
		 * addrlen needs to be passed in as a pointer,
		 * because getsockname returns the actual length
		 * of the address.
		 */
	addrlen = sizeof(struct sockaddr_in);
	if (getsockname(s, &myaddr_in, &addrlen) == -1) {
		perror(argv[0]);
		fprintf(stderr, "%s: unable to read socket address\n", argv[0]);
		exit(1);
	}

		/* Print out a startup message for the user. */
	time(&timevar);
		/* The port number must be converted first to host byte
		 * order before printing.  On most hosts, this is not
		 * necessary, but the ntohs() call is included here so
		 * that this program could easily be ported to a host
		 * that does require it.
		 */
	printf("Connected to %s on port %u at %s",
			argv[1], ntohs(myaddr_in.sin_port), ctime(&timevar));


		/* This sleep simulates any preliminary processing
		 * that a real client might do here.
		 */
	sleep(5);

		/* Sent out all the requests to the remote server.
		 * In this case, five are sent, but any random number
		 * could be used.  Note that the first four bytes of
		 * buf are set up to contain the request number.  This
		 * number will be returned unmodified in the reply from
		 * the server.
		 */
			/* CAUTION:
			 * If you increase the number of requests sent,
			 * or the size of the requests, you should be
			 * aware that you could encounter a deadlock
			 * situation.  Both the client's and server's
			 * sockets can only queue a limited amount of
			 * data on their receive queues.
			 */
	for (i=1; i<=5; i++) {
		*buf = i;
		if (send(s, buf, 10, 0) != 10) {
			fprintf(stderr, "%s: Connection aborted on error ",
					argv[0]);
			fprintf(stderr, "on send number %d\n", i);
			exit(1);
		}
	}

		/* Now, shutdown the connection for further sends.
		 * This will cause the server to receive an end-of-file
		 * condition after it has received all the requests that
		 * have just been sent, indicating that we will not be
		 * sending any further requests.
		 */
	if (shutdown(s, 1) == -1) {
		perror(argv[0]);
		fprintf(stderr, "%s: unable to shutdown socket\n", argv[0]);
		exit(1);
	}

		/* Now, start receiving all of the replys from the server.
		 * This loop will terminate when the recv returns zero,
		 * which is an end-of-file condition.  This will happen
		 * after the server has sent all of its replies, and closed
		 * its end of the connection.
		 */
	while (i = recv(s, buf, 10, 0)) {
		if (i == -1) {
errout:			perror(argv[0]);
			fprintf(stderr, "%s: error reading result\n", argv[0]);
			exit(1);
		}
			/* The reason this while loop exists is that there
			 * is a remote possibility of the above recv returning
			 * less than 10 bytes.  This is because a recv returns
			 * as soon as there is some data, and will not wait for
			 * all of the requested data to arrive.  Since 10 bytes
			 * is relatively small compared to the allowed TCP
			 * packet sizes, a partial receive is unlikely.  If
			 * this example had used 2048 bytes requests instead,
			 * a partial receive would be far more likely.
			 * This loop will keep receiving until all 10 bytes
			 * have been received, thus guaranteeing that the
			 * next recv at the top of the loop will start at
			 * the begining of the next reply.
			 */
		while (i < 10) {
			j = recv(s, &buf[i], 10-i, 0);
			if (j == -1) goto errout;
			i += j;
		}
			/* Print out message indicating the identity of
			 * this reply.
			 */
		printf("Received result number %d\n", *buf);
	}

		/* Print message indicating completion of task. */
	time(&timevar);
	printf("All done at %s", ctime(&timevar));
}
