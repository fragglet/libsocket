/*
    internal.c
    Copyright 1998 by Richard Dawe

    This file illustrates some of the internal functions of libsocket. These
    should not be used in normal programs. They may give some insight into
    whether libsocket is detecting all the DNS servers configured and where
    it is looking for its configuration files. It should be useful in finding
    out whether libsocket is configured properly.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <lsck/lsck.h>
#include <lsck/config.h>
#include <lsck/dnsaddr.h>
#include <lsck/mstcp.h>
#include <lsck/ras.h>
#include <lsck/vdhcp.h>
#include <lsck/ini.h>

/* Constant to name mappings for tests */
int   proto_num[]  = {IPPROTO_ICMP, IPPROTO_TCP, IPPROTO_UDP, IPPROTO_RAW, 0};
char *proto_name[] = {"ICMP",       "TCP",       "UDP",       "raw", NULL   };

int type_num[]    = { SOCK_STREAM, SOCK_DGRAM, SOCK_RAW, SOCK_SEQPACKET, 0  };
char *type_name[] = { "stream",    "datagram", "raw",    "sequential", NULL };

extern int vdhcp_init (void);
extern char *vdhcp_getdata (void);

int main (void)
{
    char **p = NULL, *q = NULL;
    int i, j, sock;

    /* Banner */
    printf("libsocket Diagnostic Program\n\n");

    /* Configuration information */
    printf("Configuration directory: %s\n\n", lsck_config_getdir());

    printf("---All DNS Servers obtained via auto-configuration---\n");
    p = lsck_getdnsaddrs();
    if (p != NULL) while (*p != NULL) { printf("DNS address: %s\n", *p), p++; }
    printf("---End DNS Servers---\n\n");

    printf("MSTCP (Microsoft TCP/IP stack) DNS enabled: %s\n\n",
           mstcp_dnsenabled() ? "Yes":"No");

    printf("---MSTCP DNS Servers---\n");
    p = mstcp_getdnsaddrs();
    if (p != NULL) while (*p != NULL) { printf("MSTCP DNS address: %s\n", *p), p++; }
    printf("---End MSTCP DNS servers---\n\n");

    printf("RAS (dial-up) active : %s\n", ras_active() ? "Yes":"No");
    printf("Default RAS phonebook: %s\n\n", ras_getdefaultphonebook());

    printf("---RAS DNS Servers---\n");
    p = ras_getdnsaddrs();
    if (p != NULL) while (*p != NULL) { printf("RAS DNS address: %s\n", *p), p++; }
    printf("---End RAS DNS servers---\n\n");

    printf("VDHCP.VXD initialised OK: %s\n", vdhcp_init() ? "Yes":"No");
    q = vdhcp_getdata();
    printf("VDHCP.VXD returns data  : %s\n\n", (q != NULL) ? "Yes":"No");
    free(q);

    printf("---DHCP DNS Servers---\n");
    p = vdhcp_getdnsaddrs();
    if (p != NULL) while (*p != NULL) { printf("VDHCP DNS address: %s\n", *p), p++; }
    printf("---End DHCP DNS servers---\n\n");

    printf("---System.ini DNS Servers---\n");
    p = systemini_getdnsaddrs();
    if (p != NULL) while (*p != NULL) { printf("System.ini DNS address: %s\n", *p), p++; }
    printf("---End System.ini DNS servers---\n\n");

    /* Check support for socket types protocols */
    printf("Testing sockets with INET adapter family (AF_INET)...\n");

    for (i = 0; type_name[i] != NULL; i++) {
        for (j = 0; proto_name[j] != NULL; j++) {
            sock = socket(AF_INET, type_num[i], proto_num[j]);
            if (sock > -1) {
                printf("+ Created %s type socket with %s protocol\n",
                       type_name[i], proto_name[j]);
                close(sock);
            } else
                printf("- Failed to create %s type socket with %s protocol\n",
                       type_name[i], proto_name[j]);
        }
    }

    printf("\n");

    /* Test error messages */
    errno = EAFNOSUPPORT;
    printf("These should be meaningful error messages:\n");
    printf("lsck_sterror(): %s\n", lsck_strerror(errno));
    lsck_perror("lsck_perror() ");

    /* OK */
    return(0);
}
