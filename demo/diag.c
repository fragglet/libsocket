/*
    diag.c
    Copyright 1998, 1999 by Richard Dawe

    This file illustrates some of the internal functions of libsocket. These
    should not be used in normal programs. They may give some insight into
    whether libsocket is detecting all the DNS servers configured and where
    it is looking for its configuration files. It should be useful in finding
    out whether libsocket is configured properly.

    This program is particularly lax at free'ing the memory allocated by the
    internal functions. In general, all the memory used in NULL-terminated
    lists must be freed (i.e. all the char * strings, and then the char ** of
    the list).
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <lsck/lsck.h>
#include <lsck/copyrite.h>
#include <lsck/if.h>
#include <lsck/domname.h>
#include <lsck/hostname.h>

/* Constant to name mappings for tests */
typedef struct _PROTO_MAP {
    int   num;
    char *name;
} PROTO_MAP;

PROTO_MAP proto_map[] = {
	{ 0,            "default (0)" },
	{ IPPROTO_ICMP, "ICMP"        },
	{ IPPROTO_TCP,  "TCP"         },
	{ IPPROTO_UDP,  "UDP"         },
	{ IPPROTO_RAW,  "raw"         },
	{ 0,            NULL          }
};

typedef PROTO_MAP TYPE_MAP;

TYPE_MAP type_map[] = {
	{ SOCK_STREAM,    "stream"     },
	{ SOCK_DGRAM,     "datagram"   },
	{ SOCK_RAW,       "raw"        },
	{ SOCK_SEQPACKET, "sequential" },
	{ 0,              NULL         }
};

typedef PROTO_MAP AF_MAP;

AF_MAP af_map[] = {
	{ AF_INET, "Internet domain (AF_INET)" },
	{ AF_UNIX, "Unix domain (AF_UNIX)" },
	{ 0,       NULL }
};

LSCK_IF *__lsck_interface[LSCK_MAX_IF + 1];

int main (void)
{
	LSCK_IF *interface = NULL;
	char hostbuf[MAXGETHOSTNAME], dombuf[MAXGETDOMAINNAME];
	char **q = NULL;
	int i, j, k, sock;

	/* Banner */
	printf("libsocket Diagnostic Program\n\n");

	/* --- Configuration --- */
	__lsck_init();

	printf("%s\n%s\n\n", __lsck_get_copyright(), __lsck_get_version());
	
	__lsck_gethostname(hostbuf, sizeof(hostbuf));
	__lsck_getdomainname(dombuf, sizeof(dombuf));
	q = __lsck_getdnsaddrs();

	printf("Host name: '%s'\n", hostbuf);
	printf("Domain name: '%s'\n", dombuf);
	printf("Configuration directory: '%s'\n\n", __lsck_config_getdir());

	printf("DNS servers:");
	if ((q == NULL) || (*q == NULL)) {
		printf(" None found\n\n");
	} else {
		for (i = 0; q[i] != NULL; i++) { printf(" %s", q[i]); }
		printf("\n\n");
		for (i = 0; q[i] != NULL; i++) { free(q[i]); }
		free(q);
	}

	/* --- Available interfaces --- */
	for (i = 0; (interface = __lsck_interface[i]) != NULL; i++) {
		printf("Interface %s\n", interface->name);

		/* Only do this for Internet family interfaces */
		if (interface->addr == NULL) {
			printf("\n");
			continue;
		}
		
		if (interface->addr->family != AF_INET) {
			printf("\n");
			continue;
		}

		for (j = 0; interface->addr->table.inet[j] != NULL; j++) {
			/* Address details */
			printf("\tAddress %s ",
			       inet_ntoa(interface->addr->table.inet[j]->addr));
			printf("netmask %s ",
			       inet_ntoa(interface->addr->table.inet[j]->netmask));

			if (interface->addr->table.inet[j]->gw_present)
				printf("gateway %s ",
				       inet_ntoa(interface->addr->table.inet[j]->gw));
			printf("\n");

			/* DNS servers */
			for (k = 0;
			     (k < LSCK_IF_ADDR_INET_DNS_MAX)
			     && (interface->addr->table.inet[j]->dns[k].s_addr != 0);
			     k++) {
				printf("\tDNS %i %s\n", k + 1,
				       inet_ntoa(interface->addr->table.inet[j]->dns[k]));
			}

			/* Host name, domain name */
			if (strlen(interface->addr->table.inet[j]->hostname)
			    != 0)
				printf("\tHost name '%s'\n",
				       interface->addr->table.inet[j]->hostname);

			if (strlen(interface->addr->table.inet[j]->domainname)
			    != 0)
				printf("\tDomain name '%s'\n",
				       interface->addr->table.inet[j]->domainname);

			printf("\t---\n");
		}

		printf("\n");
	}

	/* --- Sockets --- */

	/* Check support for socket types protocols */	
	for (i = 0; af_map[i].name != NULL; i++) {
		printf("Testing %s sockets...\n", af_map[i].name);
		
		for (j = 0; type_map[j].name != NULL; j++) {
			for (k = 0; proto_map[k].name != NULL; k++) {
				sock = socket(af_map[i].num,
					      type_map[j].num,
					      proto_map[k].num);
			    
				if (sock > -1) {
					printf("+ SUPPORTED: %s type socket "
					       "with %s protocol\n",
					       type_map[j].name,
					       proto_map[k].name);
					close(sock);
				} else if (errno == EPROTONOSUPPORT) {
					printf(". Unsupported: %s type socket "
					       "with %s protocol\n",
					       type_map[j].name,
					       proto_map[k].name);
				} else {
					printf("- Error creating %s type "
					       "socket with %s protocol\n",
					       type_map[j].name,
					       proto_map[k].name);
				}
			}
		}

		printf("\n");
	}	

	/* Test error messages. */
	errno = EAFNOSUPPORT;

	printf("These should be meaningful error messages:\n");
	printf("strerror(): %s\n", strerror(errno));
	perror("perror()");

	/* OK */
	return(0);
}
