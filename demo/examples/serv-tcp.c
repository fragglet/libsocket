/*
 *          		S E R V . T C P
 *
 *	This is an example program that demonstrates the use of
 *	stream sockets as an IPC mechanism.  This contains the server,
 *	and is intended to operate in conjunction with the client
 *	program found in client.tcp.  Together, these two programs
 *	demonstrate many of the features of sockets, as well as good
 *	conventions for using these features.
 *
 *	This program provides a service called "example".  In order for
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
#include <signal.h>
#include <stdio.h>
#include <netdb.h>

int s;				/* connected socket descriptor */
int ls;				/* listen socket descriptor */

struct hostent *hp;		/* pointer to host info for remote host */
struct servent *sp;		/* pointer to service information */

long timevar;			/* contains time returned by time() */
char *ctime();			/* declare time formatting routine */

long linger = 1;		/* allow a lingering, graceful close; */
				/* used when setting SO_LINGER */

struct sockaddr_in myaddr_in;	/* for local socket address */
struct sockaddr_in peeraddr_in;	/* for peer socket address */

/*
 *			M A I N
 *
 *	This routine starts the server.  It forks, leaving the child
 *	to do all the work, so it does not have to be run in the
 *	background.  It sets up the listen socket, and for each incoming
 *	connection, it forks a child process to process the data.  It
 *	will loop forever, until killed by a signal.
 *
 */
main(argc, argv)
int argc;
char *argv[];
{
	int addrlen;

		/* clear out address structures */
	memset ((char *)&myaddr_in, 0, sizeof(struct sockaddr_in));
	memset ((char *)&peeraddr_in, 0, sizeof(struct sockaddr_in));

		/* Set up address structure for the listen socket. */
	myaddr_in.sin_family = AF_INET;
		/* The server should listen on the wildcard address,
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
	sp = getservbyname ("example", "tcp");
	if (sp == NULL) {
		fprintf(stderr, "%s: example not found in /etc/services\n",
				argv[0]);
		exit(1);
	}
	myaddr_in.sin_port = sp->s_port;

		/* Create the listen socket. */
	ls = socket (AF_INET, SOCK_STREAM, 0);
	if (ls == -1) {
		perror(argv[0]);
		fprintf(stderr, "%s: unable to create socket\n", argv[0]);
		exit(1);
	}
		/* Bind the listen address to the socket. */
	if (bind(ls, &myaddr_in, sizeof(struct sockaddr_in)) == -1) {
		perror(argv[0]);
		fprintf(stderr, "%s: unable to bind address\n", argv[0]);
		exit(1);
	}
		/* Initiate the listen on the socket so remote users
		 * can connect.  The listen backlog is set to 5, which
		 * is the largest currently supported.
		 */
	if (listen(ls, 5) == -1) {
		perror(argv[0]);
		fprintf(stderr, "%s: unable to listen on socket\n", argv[0]);
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
		fprintf(stderr, "%s: unable to fork daemon\n", argv[0]);
		exit(1);

	case 0:			/* The child process (daemon) comes here. */
			/* Close stdin and stderr so that they will not
			 * be kept open.  Stdout is assumed to have been
			 * redirected to some logging file, or /dev/null.
			 * From now on, the daemon will not report any
			 * error messages.  This daemon will loop forever,
			 * waiting for connections and forking a child
			 * server to handle each one.
			 */
		close(stdin);
		close(stderr);
			/* Set SIGCLD to SIG_IGN, in order to prevent
			 * the accumulation of zombies as each child
			 * terminates.  This means the daemon does not
			 * have to make wait calls to clean them up.
			 */
		signal(SIGCLD, SIG_IGN);
		for(;;) {
				/* Note that addrlen is passed as a pointer
				 * so that the accept call can return the
				 * size of the returned address.
				 */
			addrlen = sizeof(struct sockaddr_in);
				/* This call will block until a new
				 * connection arrives.  Then, it will
				 * return the address of the connecting
				 * peer, and a new socket descriptor, s,
				 * for that connection.
				 */
			s = accept(ls, &peeraddr_in, &addrlen);
			if ( s == -1) exit(1);
			switch (fork()) {
			case -1:	/* Can't fork, just exit. */
				exit(1);
			case 0:		/* Child process comes here. */
				server();
				exit(0);
			default:	/* Daemon process comes here. */
					/* The daemon needs to remember
					 * to close the new accept socket
					 * after forking the child.  This
					 * prevents the daemon from running
					 * out of file descriptor space.  It
					 * also means that when the server
					 * closes the socket, that it will
					 * allow the socket to be destroyed
					 * since it will be the last close.
					 */
				close(s);
			}

		}

	default:		/* Parent process comes here. */
		exit(0);
	}
}

/*
 *				S E R V E R
 *
 *	This is the actual server routine that the daemon forks to
 *	handle each individual connection.  Its purpose is to receive
 *	the request packets from the remote client, process them,
 *	and return the results to the client.  It will also write some
 *	logging information to stdout.
 *
 */
server()
{
	int reqcnt = 0;		/* keeps count of number of requests */
	char buf[10];		/* This example uses 10 byte messages. */
	char *inet_ntoa();
	char *hostname;		/* points to the remote host's name string */
	int len, len1;

		/* Close the listen socket inherited from the daemon. */
	close(ls);

		/* Look up the host information for the remote host
		 * that we have connected with.  Its internet address
		 * was returned by the accept call, in the main
		 * daemon loop above.
		 */
	hp = gethostbyaddr ((char *) &peeraddr_in.sin_addr,
				sizeof (struct in_addr),
				peeraddr_in.sin_family);

	if (hp == NULL) {
			/* The information is unavailable for the remote
			 * host.  Just format its internet address to be
			 * printed out in the logging information.  The
			 * address will be shown in "internet dot format".
			 */
		hostname = inet_ntoa(peeraddr_in.sin_addr);
	} else {
		hostname = hp->h_name;	/* point to host's name */
	}
		/* Log a startup message. */
	time (&timevar);
		/* The port number must be converted first to host byte
		 * order before printing.  On most hosts, this is not
		 * necessary, but the ntohs() call is included here so
		 * that this program could easily be ported to a host
		 * that does require it.
		 */
	printf("Startup from %s port %u at %s",
		hostname, ntohs(peeraddr_in.sin_port), ctime(&timevar));

		/* Set the socket for a lingering, graceful close.
		 * Since linger was set to 1 above, this will cause
		 * a final close of this socket to wait until all of the
		 * data sent on it has been received by the remote host.
		 */
	if (setsockopt(s, SOL_SOCKET, SO_LINGER, (char *)&linger,
					sizeof(long)) == -1) {
errout:		printf("Connection with %s aborted on error\n", hostname);
		exit(1);
	}

		/* Go into a loop, receiving requests from the remote
		 * client.  After the client has sent the last request,
		 * it will do a shutdown for sending, which will cause
		 * an end-of-file condition to appear on this end of the
		 * connection.  After all of the client's requests have
		 * been received, the next recv call will return zero
		 * bytes, signalling an end-of-file condition.  This is
		 * how the server will know that no more requests will
		 * follow, and the loop will be exited.
		 */
	while (len = recv(s, buf, 10, 0)) {
		if (len == -1) goto errout; /* error from recv */
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
			 * the begining of the next request.
			 */
		while (len < 10) {
			len1 = recv(s, &buf[len], 10-len, 0);
			if (len1 == -1) goto errout;
			len += len1;
		}
			/* Increment the request count. */
		reqcnt++;
			/* This sleep simulates the processing of the
			 * request that a real server might do.
			 */
		sleep(1);
			/* Send a response back to the client. */
		if (send(s, buf, 10, 0) != 10) goto errout;
	}

		/* The loop has terminated, because there are no
		 * more requests to be serviced.  As mentioned above,
		 * this close will block until all of the sent replies
		 * have been received by the remote host.  The reason
		 * for lingering on the close is so that the server will
		 * have a better idea of when the remote has picked up
		 * all of the data.  This will allow the start and finish
		 * times printed in the log file to reflect more accurately
		 * the length of time this connection was used.
		 */
	close(s);

		/* Log a finishing message. */
	time (&timevar);
		/* The port number must be converted first to host byte
		 * order before printing.  On most hosts, this is not
		 * necessary, but the ntohs() call is included here so
		 * that this program could easily be ported to a host
		 * that does require it.
		 */
	printf("Completed %s port %u, %d requests, at %s\n",
		hostname, ntohs(peeraddr_in.sin_port), reqcnt, ctime(&timevar));
}
