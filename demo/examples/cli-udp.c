/*
 *			C L I E N T . U D P
 *
 *	This is an example program that demonstrates the use of
 *	datagram sockets as an IPC mechanism.  This contains the client,
 *	and is intended to operate in conjunction with the server
 *	program found in serv.udp.  Together, these two programs
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
 *	The "example" service is an example of a simple name server
 *	application.  The host that is to provide this service is
 *	required to be in the /etc/hosts file.  Also, the host
 *	providing this service presumably knows the internet addresses
 *	of many hosts which the local host does not.  Therefore,
 *	this program will request the internet address of a target
 *	host by name from the serving host.  The serving host
 *	will return the requested internet address as a response,
 *	and will return an address of all ones if it does not recognize
 *	the host name.
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <signal.h>
#include <netdb.h>

extern int errno;

int s;				/* socket descriptor */

struct hostent *hp;		/* pointer to host info for nameserver host */
struct servent *sp;		/* pointer to service information */

struct sockaddr_in myaddr_in;	/* for local socket address */
struct sockaddr_in servaddr_in;	/* for server socket address */
struct in_addr reqaddr;		/* for returned internet address */

#define ADDRNOTFOUND	0xffffffff	/* value returned for unknown host */
#define RETRIES	5		/* number of times to retry before givin up */

/*
 *			H A N D L E R
 *
 *	This routine is the signal handler for the alarm signal.
 *	It simply re-installs itself as the handler and returns.
 */
handler()
{
	signal(SIGALRM, handler);
}

/*
 *			M A I N
 *
 *	This routine is the client which requests service from the remote
 *	"example server".  It will send a message to the remote nameserver
 *	requesting the internet address corresponding to a given hostname.
 *	The server will look up the name, and return its internet address.
 *	The returned address will be written to stdout.
 *
 *	The name of the system to which the requests will be sent is given
 *	as the first parameter to the command.  The second parameter should
 *	be the the name of the target host for which the internet address
 *	is sought.
 */
main(argc, argv)
int argc;
char *argv[];
{
	int i;
	int retry = RETRIES;		/* holds the retry count */
	char *inet_ntoa();

	if (argc != 3) {
		fprintf(stderr, "Usage:  %s <nameserver> <target>\n", argv[0]);
		exit(1);
	}

		/* clear out address structures */
	memset ((char *)&myaddr_in, 0, sizeof(struct sockaddr_in));
	memset ((char *)&servaddr_in, 0, sizeof(struct sockaddr_in));

		/* Set up the server address. */
	servaddr_in.sin_family = AF_INET;
		/* Get the host information for the server's hostname that the
		 * user passed in.
		 */
	hp = gethostbyname (argv[1]);
	if (hp == NULL) {
		fprintf(stderr, "%s: %s not found in /etc/hosts\n",
				argv[0], argv[1]);
		exit(1);
	}
	servaddr_in.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
		/* Find the information for the "example" server
		 * in order to get the needed port number.
		 */
	sp = getservbyname ("example", "udp");
	if (sp == NULL) {
		fprintf(stderr, "%s: example not found in /etc/services\n",
				argv[0]);
		exit(1);
	}
	servaddr_in.sin_port = sp->s_port;

		/* Create the socket. */
	s = socket (AF_INET, SOCK_DGRAM, 0);
	if (s == -1) {
		perror(argv[0]);
		fprintf(stderr, "%s: unable to create socket\n", argv[0]);
		exit(1);
	}
		/* Bind socket to some local address so that the
		 * server can send the reply back.  A port number
		 * of zero will be used so that the system will
		 * assign any available port number.  An address
		 * of INADDR_ANY will be used so we do not have to
		 * look up the internet address of the local host.
		 */
	myaddr_in.sin_family = AF_INET;
	myaddr_in.sin_port = 0;
	myaddr_in.sin_addr.s_addr = INADDR_ANY;
	if (bind(s, &myaddr_in, sizeof(struct sockaddr_in)) == -1) {
		perror(argv[0]);
		fprintf(stderr, "%s: unable to bind socket\n", argv[0]);
		exit(1);
	}
		/* Set up alarm signal handler. */
	signal(SIGALRM, handler);
		/* Send the request to the nameserver. */
again:	if (sendto (s, argv[2], strlen(argv[2]), 0, &servaddr_in,
				sizeof(struct sockaddr_in)) == -1) {
		perror(argv[0]);
		fprintf(stderr, "%s: unable to send request\n", argv[0]);
		exit(1);
	}
		/* Set up a timeout so I don't hang in case the packet
		 * gets lost.  After all, UDP does not guarantee
		 * delivery.
		 */
	alarm(5);
		/* Wait for the reply to come in.  We assume that
		 * no messages will come from any other source,
		 * so that we do not need to do a recvfrom nor
		 * check the responder's address.
		 */
	if (recv (s, &reqaddr, sizeof(struct in_addr), 0) == -1) {
		if (errno == EINTR) {
				/* Alarm went off and aborted the receive.
				 * Need to retry the request if we have
				 * not already exceeded the retry limit.
				 */
			if (--retry) {
				goto again;
			} else {
				printf("Unable to get response from");
				printf(" %s after %d attempts.\n",
						argv[1], RETRIES);
				exit(1);
			}
		} else {
			perror(argv[0]);
			fprintf(stderr, "%s: unable to receive response\n",
								argv[0]);
			exit(1);
		}
	}
	alarm(0);
		/* Print out response. */
	if (reqaddr.s_addr == ADDRNOTFOUND) {
		printf("Host %s unknown by nameserver %s.\n", argv[2],
								argv[1]);
		exit(1);
	} else {
		printf("Address for %s is %s.\n", argv[2],
			inet_ntoa(reqaddr));
	}
}
