/*
 * iovec.c - readv() & writev() tests.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <sys/uio.h>

/* writev() test string. */
char test[] = "1234567890\n";

struct iovec test_iov[] = {
	{ &test[0],  1 },
	{ &test[1],  1 },
	{ &test[2],  1 },
	{ &test[3],  1 },
	{ &test[4],  1 },
	{ &test[5],  1 },
	{ &test[6],  1 },
	{ &test[7],  1 },
	{ &test[8],  1 },
	{ &test[9],  1 },
	{ &test[10], 1 }
};

int test_iovcnt = 11;

int main (int argc, char *argv[])
{
	int ret, fd;

	/* Test writev(). */
	ret = writev(fileno(stdout), test_iov, test_iovcnt);
	printf("writev() returned %i\n", ret);

	/* Clear out text string for next test. */
	bzero(test, sizeof(test));
	
	/* Test readv() by reading test file. */
	fd = open("iovectst.txt", O_RDONLY | O_TEXT);
	if (fd == 1) { perror(argv[0]); return(EXIT_FAILURE); }
	
	ret = readv(fd, test_iov, test_iovcnt);
	printf("readv() returned %i\n", ret);
	ret = writev(fileno(stdout), test_iov, test_iovcnt);
	printf("writev() returned %i\n", ret);

	close(fd);
	
	return(EXIT_SUCCESS);
}
