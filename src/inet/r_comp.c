/*
 * ++Copyright++ 1985, 1993
 * -
 * Copyright (c) 1985, 1993
 *    The Regents of the University of California.  All rights reserved.
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
 * 	This product includes software developed by the University of
 * 	California, Berkeley and its contributors.
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
 * -
 * Portions Copyright (c) 1993 by Digital Equipment Corporation.
 * 
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies, and that
 * the name of Digital Equipment Corporation not be used in advertising or
 * publicity pertaining to distribution of the document or software without
 * specific, written prior permission.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND DIGITAL EQUIPMENT CORP. DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS.   IN NO EVENT SHALL DIGITAL EQUIPMENT
 * CORPORATION BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 * -
 * --Copyright--
 */

#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>

#include <sys/param.h>
#include <sys/socket.h>
#include <resolv.h>

#include "i_priv.h"     /* RD: inetprivate.h -> i_priv.h */

#ifdef __STDC__
int         dn_expand    ( const u_char *, const u_char *, const u_char *,
                      char *, int );
int         dn_comp      ( const char *, u_char *, int, u_char **, u_char ** );
static int  mklower      ( register int );
static int  dn_find      ( u_char *, u_char *, u_char **, u_char ** );
u_int16_t   _getshort    ( register const u_char * );
u_int32_t   _getlong     ( register const u_char * );
#ifndef __GNUC__
int         __dn_skipname( const u_char *, const u_char * );
void        __putshort   ( register u_int16_t, register u_char * );
void        __putlong    ( register u_int32_t, register u_char * );
#endif   /* __GNUC__ */
#else    /* __STDC__ */
int         dn_expand     ();
int         dn_comp       ();
int         __dn_skipname ();
static int  mklower       ();
static int  dn_find       ();
u_int16_t   _getshort     ();
u_int32_t   _getlong      ();
void        __putshort    ();
void        __putlong     ();
#endif	/* __STDC__ */

/* Richard Dawe (libsocket): The commented code below was used. Since the
 * aliasing isn't very useful for DJGPP programs and stops me
 * cross-compiling. So just make the aliases function calls. */

#if (defined(__STDC__) && !defined(__GNUC__)) || !defined(__STDC__)

int dn_skipname (const u_char *a, const u_char *b) {
  return(__dn_skipname(a, b));
}

void putshort (register u_int16_t a, register u_char *b) { __putshort(a, b); }
void putlong (register u_int32_t a, register u_char *b) { __putlong(a, b); }

#endif /* (defined(__STDC__) && !defined(__GNUC__)) || !defined(__STDC__) */

/* RD: gcc 2.95.x dislikes pragmas, so let's keep it happy. */
/*int dn_skipname (const u_char *, const u_char *)
     __attribute__ ((weak, alias("__dn_skipname")));
void putshort (register u_int16_t, register u_char *)
     __attribute__ ((weak, alias("__putshort")));
void putlong (register u_int32_t, register u_char *)
__attribute__ ((weak, alias("__putlong")));*/

/*#pragma weak dn_skipname = __dn_skipname
#pragma weak putlong = __putlong
#pragma weak putshort = __putshort*/

/* End RD */

/*
 * Expand compressed domain name 'comp_dn' to full domain name.
 * 'msg' is a pointer to the begining of the message,
 * 'eomorig' points to the first location after the message,
 * 'exp_dn' is a pointer to a buffer of size 'length' for the result.
 * Return size of compressed name or -1 if there was an error.
 */

#ifdef __STDC__
int
dn_expand( const u_char *msg, const u_char *eomorig, const u_char *comp_dn,
	char *exp_dn, int length )
#else
int
dn_expand(msg, eomorig, comp_dn, exp_dn, length)
	const u_char *msg, *eomorig, *comp_dn;
	char *exp_dn;
	int length;
#endif
{
	register const u_char *cp;
	register char *dn;
	register int n, c;
	char *eom;
	int len = -1, checked = 0;

	dn = exp_dn;
	cp = comp_dn;
	eom = exp_dn + length;
	/*
	 * fetch next label in domain name
	 */
	while ((n = *cp++)) {
		/*
		 * Check for indirection
		 */
		switch (n & INDIR_MASK) {
		case 0:
			if (dn != exp_dn) {
				if (dn >= eom)
					return (-1);
				*dn++ = '.';
			}
			if (dn+n >= eom)
				return (-1);
			checked += n + 1;
			while (--n >= 0) {
				if (((c = *cp++) == '.') || (c == '\\')) {
					if (dn + n + 2 >= eom)
						return (-1);
					*dn++ = '\\';
				}
				*dn++ = c;
				if (cp >= eomorig)	/* out of range */
					return (-1);
			}
			break;

		case INDIR_MASK:
			if (len < 0)
				len = cp - comp_dn + 1;
			cp = msg + (((n & 0x3f) << 8) | (*cp & 0xff));
			if (cp < msg || cp >= eomorig)	/* out of range */
				return (-1);
			checked += 2;
			/*
			 * Check for loops in the compressed name;
			 * if we've looked at the whole message,
			 * there must be a loop.
			 */
			if (checked >= eomorig - msg)
				return (-1);
			break;

		default:
			return (-1);			/* flag error */
		}
	}
	*dn = '\0';
	for (dn = exp_dn; (c = *dn) != '\0'; dn++)
		if (isascii(c) && isspace(c))
			return (-1);
	if (len < 0)
		len = cp - comp_dn;
	return (len);
}

/*
 * Compress domain name 'exp_dn' into 'comp_dn'.
 * Return the size of the compressed name or -1.
 * 'length' is the size of the array pointed to by 'comp_dn'.
 * 'dnptrs' is a list of pointers to previous compressed names. dnptrs[0]
 * is a pointer to the beginning of the message. The list ends with NULL.
 * 'lastdnptr' is a pointer to the end of the arrary pointed to
 * by 'dnptrs'. Side effect is to update the list of pointers for
 * labels inserted into the message as we compress the name.
 * If 'dnptr' is NULL, we don't try to compress names. If 'lastdnptr'
 * is NULL, we don't update the list.
 */

#ifdef __STDC__
int
dn_comp( const char *exp_dn, u_char *comp_dn, int length, u_char **dnptrs,
		  u_char **lastdnptr )
#else
int
dn_comp(exp_dn, comp_dn, length, dnptrs, lastdnptr)
	const char *exp_dn;
	u_char *comp_dn, **dnptrs, **lastdnptr;
	int length;
#endif
{
	register u_char *cp, *dn;
	register int c, l;
	u_char **cpp, **lpp, *sp, *eob;
	u_char *msg;

	dn = (u_char *)exp_dn;
	cp = comp_dn;
	eob = cp + length;
	lpp = cpp = NULL;
	if (dnptrs != NULL) {
		if ((msg = *dnptrs++) != NULL) {
			for (cpp = dnptrs; *cpp != NULL; cpp++)
				;
			lpp = cpp;	/* end of list to search */
		}
	} else
		msg = NULL;
	for (c = *dn++; c != '\0'; ) {
		/* look to see if we can use pointers */
		if (msg != NULL) {
			if ((l = dn_find(dn-1, msg, dnptrs, lpp)) >= 0) {
				if (cp+1 >= eob)
					return (-1);
				*cp++ = (l >> 8) | INDIR_MASK;
				*cp++ = l % 256;
				return (cp - comp_dn);
			}
			/* not found, save it */
			if (lastdnptr != NULL && cpp < lastdnptr-1) {
				*cpp++ = cp;
				*cpp = NULL;
			}
		}
		sp = cp++;	/* save ptr to length byte */
		do {
			if (c == '.') {
				c = *dn++;
				break;
			}
			if (c == '\\') {
				if ((c = *dn++) == '\0')
					break;
			}
			if (cp >= eob) {
				if (msg != NULL)
					*lpp = NULL;
				return (-1);
			}
			*cp++ = c;
		} while ((c = *dn++) != '\0');
		/* catch trailing '.'s but not '..' */
		if ((l = cp - sp - 1) == 0 && c == '\0') {
			cp--;
			break;
		}
		if (l <= 0 || l > MAXLABEL) {
			if (msg != NULL)
				*lpp = NULL;
			return (-1);
		}
		*sp = l;
	}
	if (cp >= eob) {
		if (msg != NULL)
			*lpp = NULL;
		return (-1);
	}
	*cp++ = '\0';
	return (cp - comp_dn);
}

/*
 * Skip over a compressed domain name. Return the size or -1.
 */

#ifdef __STDC__
int
__dn_skipname( const u_char *comp_dn, const u_char *eom )
#else
int
__dn_skipname(comp_dn, eom)
	const u_char *comp_dn, *eom;
#endif
{
	register const u_char *cp;
	register int n;

	cp = comp_dn;
	while (cp < eom && (n = *cp++)) {
		/*
		 * check for indirection
		 */
		switch (n & INDIR_MASK) {
		case 0:			/* normal case, n == len */
			cp += n;
			continue;
		case INDIR_MASK:	/* indirection */
			cp++;
			break;
		default:		/* illegal type */
			return (-1);
		}
		break;
	}
	if (cp > eom)
		return (-1);
	return (cp - comp_dn);
}

#ifdef __STDC__
static int
mklower( register int ch )
#else
static int
mklower(ch)
	register int ch;
#endif
{
	if (isascii(ch) && isupper(ch))
		return (tolower(ch));
	return (ch);
}

/*
 * Search for expanded name from a list of previously compressed names.
 * Return the offset from msg if found or -1.
 * dnptrs is the pointer to the first name on the list,
 * not the pointer to the start of the message.
 */

#ifdef __STDC__
static int
dn_find( u_char *exp_dn, u_char *msg, u_char **dnptrs, u_char **lastdnptr )
#else
static int
dn_find(exp_dn, msg, dnptrs, lastdnptr)
	u_char *exp_dn, *msg;
	u_char **dnptrs, **lastdnptr;
#endif
{
	register u_char *dn, *cp, **cpp;
	register int n;
	u_char *sp;

	for (cpp = dnptrs; cpp < lastdnptr; cpp++) {
		dn = exp_dn;
		sp = cp = *cpp;
		while ((n = *cp++)) {
			/*
			 * check for indirection
			 */
			switch (n & INDIR_MASK) {
			case 0:		/* normal case, n == len */
				while (--n >= 0) {
					if (*dn == '.')
						goto next;
					if (*dn == '\\')
						dn++;
					if (mklower(*dn++) != mklower(*cp++))
						goto next;
				}
				if ((n = *dn++) == '\0' && *cp == '\0')
					return (sp - msg);
				if (n == '.')
					continue;
				goto next;

			case INDIR_MASK:	/* indirection */
				cp = msg + (((n & 0x3f) << 8) | *cp);
				break;

			default:	/* illegal type */
				return (-1);
			}
		}
		if (*dn == '\0')
			return (sp - msg);
	next:	;
	}
	return (-1);
}

/*
 * Routines to insert/extract short/long's.
 */

#ifdef __STDC__
u_int16_t
_getshort( register const u_char *msgp )
#else
u_int16_t
_getshort(msgp)
	register const u_char *msgp;
#endif
{
	register u_int16_t u;

	GETSHORT(u, msgp);
	return (u);
}

#ifdef __STDC__
u_int32_t
_getlong( register const u_char *msgp )
#else
u_int32_t
_getlong(msgp)
	register const u_char *msgp;
#endif
{
	register u_int32_t u;

	GETLONG(u, msgp);
	return (u);
}

#if defined(__STDC__) || defined(__cplusplus)
void
__putshort(register u_int16_t s, register u_char *msgp)	/* must match proto */
#else
void
__putshort(s, msgp)
	register u_int16_t s;
	register u_char *msgp;
#endif
{
	PUTSHORT(s, msgp);
}

#ifdef __STDC__
void
__putlong( register u_int32_t l, register u_char *msgp )
#else
void
__putlong(l, msgp)
	register u_int32_t l;
	register u_char *msgp;
#endif
{
	PUTLONG(l, msgp);
}

