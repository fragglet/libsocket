/*
 *  libsocket - BSD socket like library for DJGPP
 *  Copyright 1997, 1998 by Indrek Mandre
 *  Copyright 1997, 1998 by Richard Dawe
 *
 *  This library is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU Library General Public License as published
 *  by the Free Software Foundation; either version 2 of the License, or (at
 *  your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 * 
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * IM:
 * In future this "basket" file must be separated into different parts
 * as in UNIXes, maybe djgpp's next version will help to do it...
 */

#ifndef __socket_h
#define __socket_h

#ifdef __cplusplus	/* IM: now in C++ too */
extern "C"
{
#endif

#include <errno.h>

/* IM: These are the errnos which are missing in DJGPP, many apps use them
   and for fun I took them from Linux errno.h file. Fun that is, that can't
   stay so for future... */

#define EWOULDBLOCK EAGAIN
#define ELOOP		40
#define EREMOTE         66      /* Object is remote */
#define EPROTOTYPE	71
#define EUSERS          87      /* Too many users */
#define ENOTSOCK	88
#define EDESTADDRREQ    89
#define EMSGSIZE        90
#define ENOPROTOOPT	92
#define EPROTONOSUPPORT	93
#define ESOCKTNOSUPPORT	94
#define EOPNOTSUPP      95
#define EPFNOSUPPORT	96
#define EAFNOSUPPORT	97
#define EADDRINUSE      98      /* Address already in use */
#define EADDRNOTAVAIL   99      /* Cannot assign requested address */
#define ENETDOWN        100     /* Network is down */
#define ENETUNREACH     101     /* Network is unreachable */
#define ENETRESET       102     /* Network dropped connection because of reset */
#define ECONNABORTED    103     /* Software caused connection abort */
#define ECONNRESET      104     /* Connection reset by peer */
#define ENOBUFS         105     /* No buffer space available */
#define EISCONN         106     /* Transport endpoint is already connected */
#define ENOTCONN        107     /* Transport endpoint is not connected */
#define ESHUTDOWN       108     /* Cannot send after transport endpoint shutdown */
#define ETOOMANYREFS    109     /* Too many references: cannot splice */
#define ETIMEDOUT       110     /* Connection timed out */
#define ECONNREFUSED    111     /* Connection refused */
#define EHOSTDOWN       112     /* Host is down */
#define EHOSTUNREACH    113     /* No route to host */
#define EALREADY        114
#define EINPROGRESS     115
#define ESTALE          116     /* Stale NFS file handle */
#define EDQUOT          122     /* Quota exceeded */

/* socket.h, mostly taken from Linux .h files */

/*
 * Type values for resources and queries
 */
#define T_A             1               /* host address */
#define T_NS            2               /* authoritative server */
#define T_MD            3               /* mail destination */
#define T_MF            4               /* mail forwarder */
#define T_CNAME         5               /* canonical name */
#define T_SOA           6               /* start of authority zone */
#define T_MB            7               /* mailbox domain name */
#define T_MG            8               /* mail group member */
#define T_MR            9               /* mail rename name */
#define T_NULL          10              /* null resource record */
#define T_WKS           11              /* well known service */
#define T_PTR           12              /* domain name pointer */
#define T_HINFO         13              /* host information */
#define T_MINFO         14              /* mailbox information */
#define T_MX            15              /* mail routing information */
#define T_TXT           16              /* text strings */
#define T_RP            17              /* responsible person */
#define T_AFSDB         18              /* AFS cell database */
#define T_X25           19              /* X_25 calling address */
#define T_ISDN          20              /* ISDN calling address */
#define T_RT            21              /* router */
#define T_NSAP          22              /* NSAP address */
#define T_NSAP_PTR      23              /* reverse NSAP lookup (deprecated) */
#define T_SIG           24              /* security signature */
#define T_KEY           25              /* security key */
#define T_PX            26              /* X.400 mail mapping */
#define T_GPOS          27              /* geographical position (withdrawn) */
#define T_AAAA          28              /* IP6 Address */
#define T_LOC           29              /* Location Information */
        /* non standard */
#define T_UINFO         100             /* user (finger) information */
#define T_UID           101             /* user ID */
#define T_GID           102             /* group ID */
#define T_UNSPEC        103             /* Unspecified format (binary data) */
        /* Query type values which do not appear in resource records */
#define T_AXFR          252             /* transfer zone of authority */
#define T_MAILB         253             /* transfer mailbox records */
#define T_MAILA         254             /* transfer mail agent records */
#define T_ANY           255             /* wildcard match */

/*
 * Values for class field
 */

#define C_IN            1               /* the arpa internet */
#define C_CHAOS         3               /* for chaos net (MIT) */
#define C_HS            4               /* for Hesiod name server (MIT) (XXX) */
        /* Query class values which do not appear in resource records */
#define C_ANY           255             /* wildcard match */

#define NETDB_INTERNAL -1 /* see errno */
#define NETDB_SUCCESS   0 /* no problem */
#define HOST_NOT_FOUND  1 /* Authoritative Answer Host not found */
#define TRY_AGAIN       2 /* Non-Authoritive Host not found, or SERVERFAIL */
#define NO_RECOVERY     3 /* Non recoverable errors, FORMERR, REFUSED, NOTIMP */
#define NO_DATA         4 /* Valid name, no data record of requested type */
#define NO_ADDRESS      NO_DATA         /* no address, look for MX record */


#define SIOCGIFNAME 0
#define SIOCGIFNETMASK 0
#define SIOCGIFADDR 0
#define SIOCGIFCONF 0

struct sockaddr
{
        unsigned short  sa_family;      /* address family, AF_xxx       */
        char            sa_data[14];    /* 14 bytes of protocol address */
};

struct linger {
        int             l_onoff;        /* Linger active                */
        int             l_linger;       /* How long to linger for       */
};

/*
 * Structures returned by network data base library.  All addresses are
 * supplied in host order, and returned in network order (suitable for
 * use in system calls).
 */
struct  hostent {
        char    *h_name;        /* official name of host */
        char    **h_aliases;    /* alias list */
        int     h_addrtype;     /* host address type */
        int     h_length;       /* length of address */
        char    **h_addr_list;  /* list of addresses from name server */
#define h_addr  h_addr_list[0]  /* address, for backward compatiblity */
};

/*
 * Assumption here is that a network number
 * fits in an unsigned long -- probably a poor one.
 */
struct  netent {
        char            *n_name;        /* official name of net */
        char            **n_aliases;    /* alias list */
        int             n_addrtype;     /* net address type */
        unsigned long   n_net;          /* network # */
};


struct  servent {
        char    *s_name;        /* official service name */
        char    **s_aliases;    /* alias list */
        int     s_port;         /* port # */
        char    *s_proto;       /* protocol to use */
};

struct  protoent {
        char    *p_name;        /* official protocol name */
        char    **p_aliases;    /* alias list */
        int     p_proto;        /* protocol # */
};

struct rpcent {
        char    *r_name;        /* name of server for this rpc program */
        char    **r_aliases;    /* alias list */
        int     r_number;       /* rpc program number */
};

#define SOCK_STREAM     1               /* stream (connection) socket   */
#define SOCK_DGRAM      2               /* datagram (conn.less) socket  */
#define SOCK_RAW        3               /* raw socket                   */
#define SOCK_RDM        4               /* reliably-delivered message   */
#define SOCK_SEQPACKET  5

/* Flags we can use with send/ and recv. */
#define MSG_OOB         1
#define MSG_PEEK        2
#define MSG_DONTROUTE   4
/*#define MSG_CTRUNC    8       - We need to support this for BSD oddments */
#define MSG_PROXY       16      /* Supply or ask second address. */

/* Setsockoptions(2) level. Thanks to BSD these must match IPPROTO_xxx */
#define SOL_IP          0
#define SOL_IPX         256
#define SOL_AX25        257
#define SOL_ATALK       258
#define SOL_NETROM      259
#define SOL_TCP         6
#define SOL_UDP         17

enum {
  IPPROTO_IP = 0,               /* Dummy protocol for TCP               */
  IPPROTO_ICMP = 1,             /* Internet Control Message Protocol    */
  IPPROTO_IGMP = 2,             /* Internet Gateway Management Protocol */
  IPPROTO_IPIP = 4,             /* IPIP tunnels (older KA9Q tunnels use 94) */
  IPPROTO_TCP = 6,              /* Transmission Control Protocol        */
  IPPROTO_EGP = 8,              /* Exterior Gateway Protocol            */
  IPPROTO_PUP = 12,             /* PUP protocol                         */
  IPPROTO_UDP = 17,             /* User Datagram Protocol               */
  IPPROTO_IDP = 22,             /* XNS IDP protocol                     */

  IPPROTO_RAW = 255,            /* Raw IP packets                       */
  IPPROTO_MAX
};

/* Standard well-known ports.  */
enum
  {
    IPPORT_ECHO = 7,            /* Echo service.  */
    IPPORT_DISCARD = 9,         /* Discard transmissions service.  */
    IPPORT_SYSTAT = 11,         /* System status service.  */
    IPPORT_DAYTIME = 13,        /* Time of day service.  */
    IPPORT_NETSTAT = 15,        /* Network status service.  */
    IPPORT_FTP = 21,            /* File Transfer Protocol.  */
    IPPORT_TELNET = 23,         /* Telnet protocol.  */
    IPPORT_SMTP = 25,           /* Simple Mail Transfer Protocol.  */
    IPPORT_TIMESERVER = 37,     /* Timeserver service.  */
    IPPORT_NAMESERVER = 42,     /* Domain Name Service.  */
    IPPORT_WHOIS = 43,          /* Internet Whois service.  */
    IPPORT_MTP = 57,

    IPPORT_TFTP = 69,           /* Trivial File Transfer Protocol.  */
    IPPORT_RJE = 77,
    IPPORT_FINGER = 79,         /* Finger service.  */
    IPPORT_TTYLINK = 87,
    IPPORT_SUPDUP = 95,         /* SUPDUP protocol.  */


    IPPORT_EXECSERVER = 512,    /* execd service.  */
    IPPORT_LOGINSERVER = 513,   /* rlogind service.  */
    IPPORT_CMDSERVER = 514,
    IPPORT_EFSSERVER = 520,

    /* UDP ports.  */
    IPPORT_BIFFUDP = 512,
    IPPORT_WHOSERVER = 513,
    IPPORT_ROUTESERVER = 520,

    /* Ports less than this value are reserved for privileged processes.  */
    IPPORT_RESERVED = 1024,

    /* Ports greater this value are reserved for (non-privileged) servers.  */
    IPPORT_USERRESERVED = 5000
  };

/* Link numbers.  */
#define IMPLINK_IP              155
#define IMPLINK_LOWEXPER        156
#define IMPLINK_HIGHEXPER       158

/*
 * Address families.
 */
#define AF_UNSPEC       0               /* unspecified */
#define AF_UNIX         1               /* local to host (pipes, portals) */
#define AF_INET         2               /* internetwork: UDP, TCP, etc. */
#define AF_IMPLINK      3               /* arpanet imp addresses */
#define AF_PUP          4               /* pup protocols: e.g. BSP */
#define AF_CHAOS        5               /* mit CHAOS protocols */
#define AF_IPX          6               /* IPX and SPX */
#define AF_NS           6               /* XEROX NS protocols */
#define AF_ISO          7               /* ISO protocols */
#define AF_OSI          AF_ISO          /* OSI is ISO */
#define AF_ECMA         8               /* european computer manufacturers */
#define AF_DATAKIT      9               /* datakit protocols */
#define AF_CCITT        10              /* CCITT protocols, X.25 etc */
#define AF_SNA          11              /* IBM SNA */
#define AF_DECnet       12              /* DECnet */
#define AF_DLI          13              /* Direct data link interface */
#define AF_LAT          14              /* LAT */
#define AF_HYLINK       15              /* NSC Hyperchannel */
#define AF_APPLETALK    16              /* AppleTalk */
#define AF_NETBIOS      17              /* NetBios-style addresses */
#define AF_VOICEVIEW    18              /* VoiceView */
#define AF_FIREFOX      19              /* FireFox */
#define AF_UNKNOWN1     20              /* Somebody is using this! */
#define AF_BAN          21              /* Banyan */

/* RD: From Winsock 2 headers, we have: */

/*#define AF_MAX          22*/

#define AF_ATM          22              /* Native ATM Services */
#define AF_INET6        23              /* Internetwork Ver. 6 (RD: IPv6?) */

#define AF_MAX          24

/* End RD */

/*
 * Protocol families, same as address families for now.
 */
#define PF_UNSPEC       AF_UNSPEC
#define PF_UNIX         AF_UNIX
#define PF_INET         AF_INET
#define PF_IMPLINK      AF_IMPLINK
#define PF_PUP          AF_PUP
#define PF_CHAOS        AF_CHAOS
#define PF_NS           AF_NS
#define PF_IPX          AF_IPX
#define PF_ISO          AF_ISO
#define PF_OSI          AF_OSI
#define PF_ECMA         AF_ECMA
#define PF_DATAKIT      AF_DATAKIT
#define PF_CCITT        AF_CCITT
#define PF_SNA          AF_SNA
#define PF_DECnet       AF_DECnet
#define PF_DLI          AF_DLI
#define PF_LAT          AF_LAT
#define PF_HYLINK       AF_HYLINK
#define PF_APPLETALK    AF_APPLETALK
#define PF_VOICEVIEW    AF_VOICEVIEW
#define PF_FIREFOX      AF_FIREFOX
#define PF_UNKNOWN1     AF_UNKNOWN1
#define PF_BAN          AF_BAN

/* RD: Winsock 2 new ones (see AF_* above) */

#define PF_ATM          AF_ATM
#define PF_INET6        AF_INET6

/* End RD */

#define PF_MAX          AF_MAX

/*
 * Option flags per-socket.
 */
#define SO_DEBUG        0x0001          /* turn on debugging info recording */
#define SO_ACCEPTCONN   0x0002          /* socket has had listen() */
#define SO_REUSEADDR    0x0004          /* allow local address reuse */
#define SO_KEEPALIVE    0x0008          /* keep connections alive */
#define SO_DONTROUTE    0x0010          /* just use interface addresses */
#define SO_BROADCAST    0x0020          /* permit sending of broadcast msgs */
#define SO_USELOOPBACK  0x0040          /* bypass hardware when possible */
#define SO_LINGER       0x0080          /* linger on close if data present */
#define SO_OOBINLINE    0x0100          /* leave received OOB data in line */

#define SO_DONTLINGER   (u_int)(~SO_LINGER)

/*
 * Additional options.
 */
#define SO_SNDBUF       0x1001          /* send buffer size */
#define SO_RCVBUF       0x1002          /* receive buffer size */
#define SO_SNDLOWAT     0x1003          /* send low-water mark */
#define SO_RCVLOWAT     0x1004          /* receive low-water mark */
#define SO_SNDTIMEO     0x1005          /* send timeout */
#define SO_RCVTIMEO     0x1006          /* receive timeout */
#define SO_ERROR        0x1007          /* get error status and clear */
#define SO_TYPE         0x1008          /* get socket type */

/*
 * Options for connect and disconnect data and options.  Used only by
 * non-TCP/IP transports such as DECNet, OSI TP4, etc.
 */
#define SO_CONNDATA     0x7000
#define SO_CONNOPT      0x7001
#define SO_DISCDATA     0x7002
#define SO_DISCOPT      0x7003
#define SO_CONNDATALEN  0x7004
#define SO_CONNOPTLEN   0x7005
#define SO_DISCDATALEN  0x7006
#define SO_DISCOPTLEN   0x7007

/*
 * Option for opening sockets for synchronous access.
 */
#define SO_OPENTYPE     0x7008

#define SO_SYNCHRONOUS_ALERT    0x10
#define SO_SYNCHRONOUS_NONALERT 0x20

/*
 * Other NT-specific options.
 */
#define SO_MAXDG        0x7009
#define SO_MAXPATHDG    0x700A
#define SO_UPDATE_ACCEPT_CONTEXT 0x700B
#define SO_CONNECT_TIME 0x700C

/*
 * TCP options.
 */
#define TCP_NODELAY     0x0001
#define TCP_BSDURGENT   0x7000

#define SOL_SOCKET      0xffff          /* options for socket level */

/* Internet address. */
struct in_addr {
        unsigned long s_addr;
};

/* Structure describing an Internet (IP) socket address. */
#define __SOCK_SIZE__   16              /* sizeof(struct sockaddr)      */
struct sockaddr_in {
  short int             sin_family;     /* Address family               */
  unsigned short int    sin_port;       /* Port number                  */
  struct in_addr        sin_addr;       /* Internet address             */

  /* Pad to size of `struct sockaddr'. */
  unsigned char         __pad[__SOCK_SIZE__ - sizeof(short int) -
                        sizeof(unsigned short int) - sizeof(struct in_addr)];
};
#define sin_zero        __pad           /* for BSD UNIX comp. -FvK      */

/*
 * Definitions of the bits in an Internet address integer.
 * On subnets, host and network parts are found according
 * to the subnet mask, not these masks.
 */
#define IN_CLASSA(a)            ((((long int) (a)) & 0x80000000) == 0)
#define IN_CLASSA_NET           0xff000000
#define IN_CLASSA_NSHIFT        24
#define IN_CLASSA_HOST          (0xffffffff & ~IN_CLASSA_NET)
#define IN_CLASSA_MAX           128

#define IN_CLASSB(a)            ((((long int) (a)) & 0xc0000000) == 0x80000000)
#define IN_CLASSB_NET           0xffff0000
#define IN_CLASSB_NSHIFT        16
#define IN_CLASSB_HOST          (0xffffffff & ~IN_CLASSB_NET)
#define IN_CLASSB_MAX           65536

#define IN_CLASSC(a)            ((((long int) (a)) & 0xe0000000) == 0xc0000000)
#define IN_CLASSC_NET           0xffffff00
#define IN_CLASSC_NSHIFT        8
#define IN_CLASSC_HOST          (0xffffffff & ~IN_CLASSC_NET)

#define IN_CLASSD(a)            ((((long int) (a)) & 0xf0000000) == 0xe0000000)
#define IN_MULTICAST(a)         IN_CLASSD(a)
#define IN_MULTICAST_NET        0xF0000000

#define IN_EXPERIMENTAL(a)      ((((long int) (a)) & 0xe0000000) == 0xe0000000)
#define IN_BADCLASS(a)          ((((long int) (a)) & 0xf0000000) == 0xf0000000)

/* Address to accept any incoming messages. */
#define INADDR_ANY              ((unsigned long int) 0x00000000)

/* Address to send to all hosts. */
#define INADDR_BROADCAST        ((unsigned long int) 0xffffffff)

/* Address indicating an error return. */
#define INADDR_NONE             0xffffffff

/* Network number for local host loopback. */
#define IN_LOOPBACKNET          127

/* Address to loopback in software to local host.  */
#define INADDR_LOOPBACK         0x7f000001      /* 127.0.0.1   */
#define IN_LOOPBACK(a)          ((((long int) (a)) & 0xff000000) == 0x7f000000)

/* Defines for Multicast INADDR */
#define INADDR_UNSPEC_GROUP     0xe0000000      /* 224.0.0.0   */
#define INADDR_ALLHOSTS_GROUP   0xe0000001      /* 224.0.0.1   */
#define INADDR_MAX_LOCAL_GROUP  0xe00000ff      /* 224.0.0.255 */

/*
 * Define constants based on rfc883
 */
#define PACKETSZ        512             /* maximum packet size */
#define MAXDNAME        256             /* maximum domain name */
#define MAXCDNAME       255             /* maximum compressed domain name */
#define MAXLABEL        63              /* maximum length of domain label */
#define HFIXEDSZ        12              /* #/bytes of fixed data in header */
#define QFIXEDSZ        4               /* #/bytes of fixed data in query */
#define RRFIXEDSZ       10              /* #/bytes of fixed data in r record */
#define INT32SZ         4               /* for systems without 32-bit ints */
#define INT16SZ         2               /* for systems without 16-bit ints */
#define INADDRSZ        4               /* for sizeof(struct inaddr) != 4 */

/*
 * Internet nameserver port number
 */
#define NAMESERVER_PORT 53

/*
 * Currently defined opcodes
 */
#define QUERY           0x0             /* standard query */
#define IQUERY          0x1             /* inverse query */
#define STATUS          0x2             /* nameserver status query */
/*#define xxx           0x3             *//* 0x3 reserved */
#define NS_NOTIFY_OP    0x4             /* notify secondary of SOA change */
#ifdef ALLOW_UPDATES
        /* non standard - supports ALLOW_UPDATES stuff from Mike Schwartz */
# define UPDATEA        0x9             /* add resource record */
# define UPDATED        0xa             /* delete a specific resource record */
# define UPDATEDA       0xb             /* delete all named resource record */
# define UPDATEM        0xc             /* modify a specific resource record */
# define UPDATEMA       0xd             /* modify all named resource record */
# define ZONEINIT       0xe             /* initial zone transfer */
# define ZONEREF        0xf             /* incremental zone referesh */
#endif

/*
 * Currently defined response codes
 */
#define NOERROR         0               /* no error */
#define FORMERR         1               /* format error */
#define SERVFAIL        2               /* server failure */
#define NXDOMAIN        3               /* non existent domain */
#define NOTIMP          4               /* not implemented */
#define REFUSED         5               /* query refused */
#ifdef ALLOW_UPDATES
        /* non standard */
# define NOCHANGE       0xf             /* update failed to change db */
#endif

/*
 * Inline versions of get/put short/long.  Pointer is advanced.
 *
 * These macros demonstrate the property of C whereby it can be
 * portable or it can be elegant but rarely both.
 */
#define GETSHORT(s, cp) { \
        register u_char *t_cp = (u_char *)(cp); \
        (s) = ((u_int16_t)t_cp[0] << 8) \
            | ((u_int16_t)t_cp[1]) \
            ; \
        (cp) += INT16SZ; \
}

#define GETLONG(l, cp) { \
        register u_char *t_cp = (u_char *)(cp); \
        (l) = ((u_int32_t)t_cp[0] << 24) \
            | ((u_int32_t)t_cp[1] << 16) \
            | ((u_int32_t)t_cp[2] << 8) \
            | ((u_int32_t)t_cp[3]) \
            ; \
        (cp) += INT32SZ; \
}

#define PUTSHORT(s, cp) { \
        register u_int16_t t_s = (u_int16_t)(s); \
        register u_char *t_cp = (u_char *)(cp); \
        *t_cp++ = t_s >> 8; \
        *t_cp   = t_s; \
        (cp) += INT16SZ; \
}

#define PUTLONG(l, cp) { \
        register u_int32_t t_l = (u_int32_t)(l); \
        register u_char *t_cp = (u_char *)(cp); \
        *t_cp++ = t_l >> 24; \
        *t_cp++ = t_l >> 16; \
        *t_cp++ = t_l >> 8; \
        *t_cp   = t_l; \
        (cp) += INT32SZ; \
}

/*
 * Defines for handling compressed domain names
 */
#define INDIR_MASK      0xc0

/*
 * Resolver options (keep these in synch with res_debug.c, please)
 */

#define RES_INIT        0x00000001      /* address initialized */
#define RES_DEBUG       0x00000002      /* print debug messages */
#define RES_AAONLY      0x00000004      /* authoritative answers only (!IMPL)*/
#define RES_USEVC       0x00000008      /* use virtual circuit */
#define RES_PRIMARY     0x00000010      /* query primary server only (!IMPL) */
#define RES_IGNTC       0x00000020      /* ignore trucation errors */
#define RES_RECURSE     0x00000040      /* recursion desired */
#define RES_DEFNAMES    0x00000080      /* use default domain name */
#define RES_STAYOPEN    0x00000100      /* Keep TCP socket open */
#define RES_DNSRCH      0x00000200      /* search up local domain tree */
#define RES_INSECURE1   0x00000400      /* type 1 security disabled */
#define RES_INSECURE2   0x00000800      /* type 2 security disabled */
#define RES_NOALIASES   0x00001000      /* shuts off HOSTALIASES feature */

#define RES_DEFAULT     (RES_RECURSE | RES_DEFNAMES | RES_DNSRCH)

/*
 * Resolver "pfcode" values.  Used by dig.
 */
#define RES_PRF_STATS   0x00000001
/*                      0x00000002      */
#define RES_PRF_CLASS   0x00000004
#define RES_PRF_CMD     0x00000008
#define RES_PRF_QUES    0x00000010
#define RES_PRF_ANS     0x00000020
#define RES_PRF_AUTH    0x00000040
#define RES_PRF_ADD     0x00000080
#define RES_PRF_HEAD1   0x00000100
#define RES_PRF_HEAD2   0x00000200
#define RES_PRF_TTLID   0x00000400
#define RES_PRF_HEADX   0x00000800
#define RES_PRF_QUERY   0x00001000
#define RES_PRF_REPLY   0x00002000
#define RES_PRF_INIT    0x00004000
/*                      0x00008000      */

/* hooks are still experimental as of 4.9.2 */
typedef enum { res_goahead, res_nextns, res_modified, res_done, res_error }
        res_sendhookact;


#define MAXNS                   3       /* max # name servers we'll track */
#define MAXDFLSRCH              3       /* # default domain levels to try */
#define MAXDNSRCH               6       /* max # domains in search path */
#define LOCALDOMAINPARTS        2       /* min levels in name that is "local" */
#define RES_TIMEOUT             5       /* min. seconds between retries */
#define MAXRESOLVSORT           10      /* number of net to sort on */
#define RES_MAXNDOTS            15      /* should reflect bit field size */

#define MAXHOSTNAMELEN  64      /* max length of hostname */

struct __res_state {
        int     retrans;                /* retransmition time interval */
        int     retry;                  /* number of times to retransmit */
        unsigned long  options;                /* option flags - see below. */
        int     nscount;                /* number of name servers */
        struct sockaddr_in
                nsaddr_list[MAXNS];     /* address of name server */
#define nsaddr  nsaddr_list[0]          /* for backward compatibility */
        unsigned short id;                     /* current packet id */
        char    *dnsrch[MAXDNSRCH+1];   /* components of domain to search */
        char    defdname[MAXDNAME];     /* default domain */
        unsigned long  pfcode;                 /* RES_PRF_ flags - see below. */
        unsigned ndots:4;               /* threshold for initial abs. query */
        unsigned nsort:4;               /* number of elements in sort_list[] */
        char    unused[3];
        struct {
                struct in_addr  addr;
                unsigned long int       mask;
        } sort_list[MAXRESOLVSORT];
};

/*
 * Structure for query header.  The order of the fields is machine- and
 * compiler-dependent, depending on the byte/bit order and the layout
 * of bit fields.  We use bit fields only in int variables, as this
 * is all ANSI requires.  This requires a somewhat confusing rearrangement.
 */

typedef struct {
        unsigned        id :16;         /* query identification number */
                        /* fields in third byte */
        unsigned        rd: 1;          /* recursion desired */
        unsigned        tc: 1;          /* truncated message */
        unsigned        aa: 1;          /* authoritive answer */
        unsigned        opcode: 4;      /* purpose of message */
        unsigned        qr: 1;          /* response flag */
                        /* fields in fourth byte */
        unsigned        rcode :4;       /* response code */
        unsigned        unused :2;      /* unused bits (MBZ as of 4.9.3a3) */
        unsigned        pr: 1;          /* primary server req'd (!standard) */
        unsigned        ra: 1;          /* recursion available */
                        /* remaining bytes */
        unsigned        qdcount :16;    /* number of question entries */
        unsigned        ancount :16;    /* number of answer entries */
        unsigned        nscount :16;    /* number of authority entries */
        unsigned        arcount :16;    /* number of resource entries */
} HEADER;


extern struct __res_state _res;

int socket ( int, int, int );

int bind ( int sockfd, struct sockaddr *my_addr, int addrlen );

int listen ( int s, int backlog );

int connect(int sockfd, struct sockaddr *serv_addr, int addrlen );

int accept ( int s, struct sockaddr *addr, int *addrlen );

int recv(int s, void *buf, int len, unsigned int flags);
int recvfrom(int s, void *buf, int len, unsigned int flags, struct sockaddr *from, int *fromlen);

int send(int s, void *msg, int len, unsigned int flags);
int sendto(int s, void *msg, int len, unsigned int flags, struct sockaddr *to, int tolen);

int getsockname(int s, struct sockaddr * name, int * namelen );

int getpeername(int s, struct sockaddr * name, int * namelen );

int getsockopt(int s, int level, int optname, void *optval, int *optlen);
int setsockopt(int s, int level, int optname, const void *optval, int optlen);

int shutdown(int s, int how);


unsigned long int inet_addr(const char *cp);
unsigned long int inet_network(const char *cp);
char *inet_ntoa(struct in_addr in);
struct in_addr inet_makeaddr(unsigned long net, unsigned long host);
unsigned long int inet_lnaof(struct in_addr in);
unsigned long int inet_netof(struct in_addr in);


struct netent *getnetent( void );
struct netent *getnetbyname( const char *name );
struct netent *getnetbyaddr( long net, int type );
void setnetent ( int stayopen );
void endnetent ( void );


struct protoent *getprotoent(void);
struct protoent *getprotobyname(const char *name);
struct protoent *getprotobynumber(int proto);
void setprotoent(int stayopen);
void endprotoent(void);


int bindresvport ( int sd, struct sockaddr_in *sin );

/*int rexec( char **ahost, int rport, const char *name, const char *pass,
                const char *cmd, int *fd2p );*/
                
unsigned int inet_nsap_addr( const char *ascii, unsigned char *binary, int maxlen );

extern int h_errno;

struct hostent *gethostbyname ( const char *name );
struct hostent *gethostbyname2 ( const char * name, int af );
struct hostent *gethostbyaddr ( const char *addr, int len, int type );
struct hostent *gethostent ( void );
void sethostent ( int stayopen );
void endhostent ( void );
void herror ( char *string );


struct servent *getservent(void);
struct servent *getservbyname(const char *name, const char *proto);
struct servent *getservbyport(int port, const char *proto);
void setservent(int stayopen);
void endservent(void);

int res_query ( const char *, int, int, unsigned char *, int );
int res_search ( const char *, int, int, unsigned char *, int );
int res_mkquery ( int, const char *, int, int, const unsigned char *, int, const unsigned char *, unsigned char *, int);
int res_send ( const unsigned char *, int, unsigned char *, int );
int res_init ( void );
int dn_comp ( const char *, unsigned char *, int, unsigned char **, unsigned char **);
int dn_expand ( const unsigned char *, const unsigned char *, const unsigned char *, char *, int );

char * inet_nsap_ntoa ( int, register const unsigned char *, register char * );
int inet_aton ( const char *, struct in_addr * );

int getdomainname(char *name, long unsigned int len);
int setdomainname(const char *name, long unsigned int len);

#ifndef O_NDELAY
#define O_NDELAY O_NONBLOCK
#endif

#ifndef FNDELAY
#define FNDELAY O_NONBLOCK
#endif

#include <sys/ioctl.h>

#ifndef FIONBIO
#define FIONBIO     _IOW('f', 126, int)
#endif

#ifndef FIONREAD
#define FIONREAD    _IOR('f', 127, int)
#endif

#ifdef __cplusplus	/* IM: now in C++ too */
}
#endif

#endif
