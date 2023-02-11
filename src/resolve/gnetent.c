/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* RD: Some modifications by Richard Dawe for the libsocket project.
   Modifications indicated by 'RD' comments like this one. */

#include "i_priv.h"         /* RD: inetprivate.h -> i_priv.h  */
#include <lsck/config.h>    /* RD: Use configuration routines */

#define	MAXALIASES	35

static char NETDB[1024];
static FILE *netf = NULL;
static char line[BUFSIZ+1];
static struct netent net;
static char *net_aliases[MAXALIASES];

int _net_stayopen;

struct netent *getnetent( void );
static char *any( register char *, char * );
void endnetent( void );
void setnetent( int f );

void
setnetent( int f )
{
 /* IM: added getenv */
      char *x;
    /* RD: Use configuration routine now */
    /*if ( ( x = getenv ( "windir" ) ) == NULL ) return;*/
    if ( (x = lsck_config_getdir()) == NULL ) return;
	sprintf ( NETDB, "%s/networks", x );

	if (netf == NULL)
        netf = fopen(NETDB, "rt" );
	else
		rewind(netf);
	_net_stayopen |= f;
}

void
endnetent( void )
{
	if (netf) {
		fclose(netf);
		netf = NULL;
	}
	_net_stayopen = 0;
}

/* -----------
   - gnetent -
   ----------- */

/* RD: Modified not to use gotos, added comments. */

struct netent *getnetent( void )
{
	char *p;
	register char *cp, **q;
    char *x;

    /* RD: Use configuration routine now */
    /*if ( ( x = getenv ( "windir" ) ) == NULL ) return(NULL);*/
    if ( (x = lsck_config_getdir()) == NULL ) return(NULL);
	sprintf ( NETDB, "%s/networks", x );

    if ((netf == NULL) && (netf = fopen(NETDB, "rt" )) == NULL) return (NULL);

    while ((p = fgets(line, BUFSIZ, netf)) != NULL) {
        /* Comment */
        if (*p == '#') continue;

        /* Nuke end-of-line (comments) */
        cp = any(p, "#\n");
        if (cp == NULL) continue;
        *cp = '\0';

        /* Network name */
        net.n_name = p;

        /* Look for network aliases */
        cp = any(p, " \t");
        if (cp == NULL) continue;
        *cp++ = '\0';

        while (*cp == ' ' || *cp == '\t') cp++;
        p = any(cp, " \t");
        if (p != NULL) *p++ = '\0';

        net.n_net = inet_network(cp);
        net.n_addrtype = AF_INET;
        q = net.n_aliases = net_aliases;

        if (p != NULL) cp = p;
        while (cp && *cp) {
            if (*cp == ' ' || *cp == '\t') {
                cp++;
                continue;
            }
            if (q < &net_aliases[MAXALIASES - 1]) *q++ = cp;
            cp = any(cp, " \t");
            if (cp != NULL) *cp++ = '\0';
        }

        /* Found match */
        *q = NULL;
        return (&net);
    }

    /* EOF with no match = failure */
    return(NULL);
}

/* End RD */

static char *
any( register char *cp, char *match )
{
	register char *mp, c;

	while ((c = *cp)) {
		for (mp = match; *mp; mp++)
			if (*mp == c)
				return (cp);
		cp++;
	}
	return ((char *)0);
}
