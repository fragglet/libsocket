/*
 * Copyright (c) 1980, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "i_priv.h"     /* RD: inetprivate.h -> i_priv.h */

#include <netdb.h>

extern void ruserpass( const char *, char **, char ** );

int	rexecoptions;
  
int
rexec( char **ahost, int rport, const char *name, const char *pass,
		const char *cmd, int *fd2p )
{
	int s, timo = 1, s3;
	struct sockaddr_in sin, sin2, from;
	char c;
	u_short port;
	struct hostent *hp;
	static char savename[256];

#if NLS
	libc_nls_init();
#endif
	hp = gethostbyname(*ahost);
	if (hp == 0 || sizeof (sin.sin_addr) < hp->h_length) {
#if NLS
		fprintf(stderr, "%s: %s\n", *ahost,
                              catgets(_libc_cat, HerrorListSet,
				      2, "unknown host"));
#else
		fprintf(stderr, "%s: unknown host\n", *ahost);
#endif
		return (-1);
	}
	sin.sin_family = hp->h_addrtype;
	sin.sin_port = rport;
	memcpy ((caddr_t)&sin.sin_addr, hp->h_addr, hp->h_length);
	strncpy((*ahost = savename), hp->h_name, sizeof(savename) -1);
	ruserpass(*ahost, (char **)&name, (char **)&pass);
retry:
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0) {
#if NLS
		perror(catgets(_libc_cat, NetMiscSet,
			       NetMiscRexecSocket,
			       "rcmd: socket"));
#else
		perror("rexec: socket");
#endif
		return (-1);
	}
	if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		if (errno == ECONNREFUSED && timo <= 16) {
			(void) close(s);
			sleep(timo);
			timo *= 2;
			goto retry;
		}
		perror(*ahost);
		return (-1);
	}
	port = 0; /* For GCC */
	if (fd2p == 0) {
		(void) write(s, "", 1);
		port = 0;
	} else {
		char num[32];
		int s2;
		/* Richard Dawe (libsocket): int -> size_t */
		size_t sin2len;
		
		s2 = socket(AF_INET, SOCK_STREAM, 0);
		if (s2 < 0) {
			(void) close(s);
			return (-1);
		}
		sin2len = sizeof (sin2);
		sin2.sin_addr.s_addr = INADDR_ANY;
		sin2.sin_family = sin.sin_family;
		bind(s2, (struct sockaddr *)&sin2, sin2len);
		listen(s2, 1);
		if (getsockname(s2, (struct sockaddr *)&sin2, &sin2len) < 0 ||
		  sin2len != sizeof (sin2)) {
#if NLS
			perror(catgets(_libc_cat, NetMiscSet,
				       NetMiscGetsockname,
				       "getsockname"));
#else
			perror("getsockname");
#endif
			(void) close(s2);
			goto bad;
		}
		port = ntohs((u_short)sin2.sin_port);
		(void) sprintf(num, "%u", port);
		(void) write(s, num, strlen(num)+1);
		{
                  /* Richard Dawe (libsocket): int -> size_t */
                  size_t len = sizeof (from);
		  s3 = accept(s2, (struct sockaddr *)&from, &len);
		  close(s2);
		  if (s3 < 0) {
#if NLS
			perror(catgets(_libc_cat, NetMiscSet,
				       NetMiscAccept,
				       "accept"));
#else
			perror("accept");
#endif
			port = 0;
			goto bad;
		  }
		}
		*fd2p = s3;
	}
	(void) write(s, name, strlen(name) + 1);
	/* should public key encypt the password here */
	(void) write(s, pass, strlen(pass) + 1);
	(void) write(s, cmd, strlen(cmd) + 1);
	if (read(s, &c, 1) != 1) {
		perror(*ahost);
		goto bad;
	}
	if (c != 0) {
		while (read(s, &c, 1) == 1) {
			(void) write(2, &c, 1);
			if (c == '\n')
				break;
		}
		goto bad;
	}
	return (s);
bad:
	if (port)
		(void) close(*fd2p);
	(void) close(s);
	return (-1);
}
