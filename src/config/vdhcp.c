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
    vdhcp.c

    Written using information from Alfons Hoogervorst - see
    misc/docs/ipdata.txt for more information.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <dpmi.h>
#include <sys/segments.h>
#include <sys/movedata.h>

#include <lsck/vxd.h>

/*#define VDHCP_BUFFERSIZE 4096*/           /* 4K seems good */
#define VDHCP_BUFFERSIZE 65536              /* 4K actually leads to errors! */

int vdhcp_inited = 0;                   /* Whether vdhcp_init completed OK */
int vdhcp_entry[] = {0, 0};             /* Entry point of VDHCP.VXD        */
int vdhcp_buffer;                       /* DPMI buffer for results         */
__dpmi_meminfo vdhcp_buffer_meminfo;    /* DPMI buffer info                */

/* --------------
   - vdhcp_init -
   -------------- */

/* This loads and obtains the entry point of the DHCP VxD. It returns 1 on
   successful initialisation, and 0 on failure. */

int vdhcp_init (void)
{
    if (!vdhcp_inited) {
        /* Load the DHCP VxD & get its entry */
        VxdLdrLoadDevice("VDHCP.VXD");
        VxdGetEntry(vdhcp_entry, 0x49A);

        /* Valid entry point? Fail if not */
        if ( (vdhcp_entry[0] == 0) && (vdhcp_entry[1] == 0) ) return(0);

        /* Allocate DPMI memory */
        vdhcp_buffer_meminfo.handle = 0;
        vdhcp_buffer_meminfo.size = VDHCP_BUFFERSIZE;
        vdhcp_buffer_meminfo.address = 0;

        if (__dpmi_allocate_memory(&vdhcp_buffer_meminfo) == -1) return(0);

        vdhcp_buffer = __dpmi_allocate_ldt_descriptors(1);
        if (vdhcp_buffer == -1) return(0);

        __dpmi_set_segment_base_address(vdhcp_buffer,
                                        vdhcp_buffer_meminfo.address);
        __dpmi_set_segment_limit (vdhcp_buffer, vdhcp_buffer_meminfo.size);

        /* Done */
        vdhcp_inited = 1;
    }

    /* Okey dokey */
    return(vdhcp_inited);
}

/* -----------------
   - vdhcp_callvxd -
   ----------------- */

/* This calls the DHCP VxD. If the entry point has not been obtained, then a
   protection fault will result. The return value is the contents of the EAX
   register. */

int vdhcp_callvxd (void)
{
    int result = -1;    /* Fail by default */

    /* If no entry point, fail */
    if ( (vdhcp_entry[0] == 0) && (vdhcp_entry[1] == 0) ) return(result);

    __asm__ __volatile__ ( "pushl   %%es                \n\
                            pushl   %%eax               \n\
                            popl    %%es                \n\
                            xorl    %%ebx, %%ebx        \n\
                            movl    %%eax, 1            \n\
                            lcall   _vdhcp_entry        \n\
                            andl    %%eax, 0x0000ffff   \n\
                            popl    %%es"
                          : "=a" (result)
                          : "a" (vdhcp_buffer), "c" (vdhcp_buffer_meminfo.size)
                          : "eax", "ebx"
                         );

    return(result);
}

/* -----------------
   - vdhcp_getdata -
   ----------------- */

/* This gets a block of data from the DHCP VxD that contains the DNS IP data,
   but needs parsing (see vdhcp_getdnsaddrs()). */

char *vdhcp_getdata (void)
{
    char *buffer = NULL;    /* Buffer allocated after successful call */
    int ret;                /* Return from VxD call                   */

    /* Init first */
    if (!vdhcp_init()) return(NULL);

    /* Find out large the buffer has to be first */
    ret = vdhcp_callvxd();

    /* 0 = success, 111 = need more data => anything else = error? */
    if (ret != 0) {
#ifdef DEBUG
        fprintf(stderr, "vdhcp_getdata: Unable to get data "
                        "(code %i = 0x%x)\n", ret, ret);
#endif

        return(NULL);
    }

    /* Done, so allocate returned buffer and fill it */
    buffer = (char *) malloc (vdhcp_buffer_meminfo.size);
    if (buffer == NULL) return(NULL);

    movedata(vdhcp_buffer, 0, _my_ds(), (int) buffer,
             vdhcp_buffer_meminfo.size);

    return(buffer);
}

/* ---------------------
   - vdhcp_getdnsaddrs -
   --------------------- */

/* This parses the data returned by vdhcp_getdata() and converts it into a list
   of strings. If the function fails, NULL is returned. */

char **vdhcp_getdnsaddrs (void)
{
    char *buffer = NULL;
    long ip_addr = 0;           /* Temp. DNS IP address          */
    struct in_addr ip_in_addr;  /* Temp. DNS IP address struct   */
    int ip_count = 0;           /* No. DNS IP addresses          */
    int ip_offset = 0;          /* Offset in buffer to IP addr's */
    int i;                      /* Loop variable                 */

    char **p;

    /* Init first */
    if (!vdhcp_init()) return(NULL);

    /* Get the DHCP data */
    buffer = vdhcp_getdata();
    if (buffer == NULL) return(NULL);

    /* Get the number of DNS IP addresses & their offset in buffer */
    ip_count = (* ((short *) buffer + 0x20)) / 4;
    ip_offset = * ((short *) buffer + 0x24);

    /* Allocate the array of pointers */
    p = malloc(sizeof(char *) * (ip_count + 1));
    if (p == NULL) return(NULL);
    memset(p, 0, sizeof(char *) * (ip_count + 1));

    /* QUERY: Are the IP addresses returned by the DHCP VxD in host order?
       This would seem the most likely, so I have assumed this is the case. */

    /* Convert each of the addresses - First get the raw data, then convert
       it from host to network byte order and then get the dotted-quad
       version, i.e. xxx.xxx.xxx.xxx */
    for (i = 0; i < ip_count; i++) {
        ip_addr = * ((long *) buffer + ip_offset + (i * 4) );
        ip_in_addr.s_addr = htons(ip_addr);
        p[i] = strdup(inet_ntoa(ip_in_addr));
    }

    /* Free the buffer */
    free(buffer);

    /* Done */
    return(p);
}
