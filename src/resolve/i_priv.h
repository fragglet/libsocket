#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/param.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <netinet/in.h>

/* RD: Include this instead of ws.h */
#include <sys/socket.h>

typedef unsigned long u_long;
typedef unsigned int u_int;
typedef unsigned long int u_int32_t;
typedef unsigned short u_int16_t;
typedef unsigned short u_short;
typedef unsigned char u_char;

typedef char * caddr_t;

struct iovec
{
        void *iov_base;         /* BSD uses caddr_t (same thing in effect) */
        int iov_len;
};

extern void _sethtent(int f);
extern void _endhtent(void);
extern struct hostent *_gethtent(void);
extern struct hostent *_gethtbyname( const char *name);
extern struct hostent *_gethtbyaddr(const char *addr, int len,
		 int type);
extern int _validuser(FILE *hostf, const char *rhost,
		 const char *luser, const char *ruser, int baselen);
extern int _checkhost(const char *rhost, const char *lhost, int len);
extern void putlong(u_long l, u_char *msgp);
extern void putshort(u_short l, u_char *msgp);
extern u_int32_t _getlong(register const u_char *msgp);
extern u_int16_t _getshort(register const u_char *msgp);
extern void p_query(char *msg);
extern void fp_query(char *msg, FILE *file);
extern char *p_cdname(char *cp, char *msg, FILE *file);
extern char *p_rr(char *cp, char *msg, FILE *file);
extern char *p_type(int type);
extern char * p_class(int class);
extern char *p_time(u_long value);
extern char * hostalias(const char *name);
extern void sethostfile(char *name);
extern void _res_close (void);
extern void ruserpass(const char *host, char **aname, char **apass);
