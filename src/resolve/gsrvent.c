/*
 * Copyright (c) 1983, 1993
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

/* RD: Modifications by Richard Dawe for the libsocket project. Modifications
   marked with 'RD' comments like this one. */

#include "i_priv.h"         /* RD: inetprivate.h -> i_priv.h  */
#include <lsck/config.h>    /* RD: Use configuration routines */

#define	MAXALIASES	35

static char SERVDB[1024];
static FILE *servf = NULL;
static char line[BUFSIZ+1];
static struct servent serv;
static char *serv_aliases[MAXALIASES];
int _serv_stayopen;

void
setservent(f)
	int f;
{
 /* IM: added getenv to fix SERVDB */
 	char *x;

    /* RD: Use configuration routine now */
    /*if ( ( x = getenv ( "windir" ) ) == NULL ) return;*/
    if ( (x = lsck_config_getdir()) == NULL ) return;
 	sprintf ( SERVDB, "%s/services", x );

	if (servf == NULL)
        servf = fopen(SERVDB, "rt" );
	else
		rewind(servf);
	_serv_stayopen |= f;
}

void
endservent()
{
	if (servf) {
		fclose(servf);
		servf = NULL;
	}
	_serv_stayopen = 0;
}

/* --------------
   - getservent -
   -------------- */

/* RD: Modified not to use gotos & commented. */

struct servent *getservent()
{
	char *p;
	register char *cp, **q;
 	char *x;

    /* RD: Use configuration routine now */
    /*if ( ( x = getenv ( "windir" ) ) == NULL ) return(NULL);*/
    if ( (x = lsck_config_getdir()) == NULL ) return(NULL);
 	sprintf ( SERVDB, "%s/services", x );

    if ((servf == NULL) && (servf = fopen(SERVDB, "rt" )) == NULL)
        return (NULL);

    while ((p = fgets(line, BUFSIZ, servf)) != NULL) {
        /* Comment */
        if (*p == '#') continue;

        /* Comment at EOL or EOL */
        cp = strpbrk(p, "#\n");
        if (cp == NULL) continue;        
        *cp = '\0';

        /* Service name */
        serv.s_name = p;

        p = strpbrk(p, " \t");
        if (p == NULL) continue;
        *p++ = '\0';

        while (*p == ' ' || *p == '\t') p++;
        cp = strpbrk(p, ",/");
        if (cp == NULL) continue;        
        *cp++ = '\0';

        /* Port and protocol */
        serv.s_port = htons((u_short)atoi(p));
        serv.s_proto = cp;

        /* Aliases */
        q = serv.s_aliases = serv_aliases;
        cp = strpbrk(cp, " \t");
        if (cp != NULL) *cp++ = '\0';
        while (cp && *cp) {
            if (*cp == ' ' || *cp == '\t') {
                cp++;
                continue;
            }
            if (q < &serv_aliases[MAXALIASES - 1]) *q++ = cp;
            cp = strpbrk(cp, " \t");
            if (cp != NULL) *cp++ = '\0';
        }
        *q = NULL;

        /* Matched */    
        return (&serv);
    }

    /* EOF and no match */
    return (NULL);
}
