/*
   netsetup.c
   Networking parameters set-up program for libsocket, Version 1.02
   Copyright 1997 by Richard Dawe
*/

/*
   RD's Changelog
   ==============

   1997-8-18 - Added support for networks file. Fixed errors in file creation
   messages.

   1997-1-11 - Fixed bug in creation of 'hosts.eg' where the order of the host
   name and IP address was wrong (thanks to Reuben Collver for noticing). Also
   corrected formatting error in message about 'hosts.eg'.

   1997-5-12 - Fixed bug in order output - When no DNS server is present it
   should *not* output "order hosts, bind"; it should output "order hosts".
*/

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRUE -1
#define FALSE 0

int main (void)
{
	int  res;				/* General result						*/
	int  dns_present;		/* Boolean								*/
	char dns_ip[256];		/* 256 chars is probably long enough	*/
	char domain_name[256];	/* Domain name							*/
    char net_ip[256];       /* PC's network IP address              */
    char net_mask[256];     /* IP address network mask              */
    int  net_mask_known;    /* True if user knows net_mask          */
	char pc_name[256];		/* PC's name address					*/
	char pc_ip[256];		/* PC's IP address						*/
	FILE *fOut;				/* Output file handle					*/
    char *p, *q;            /* Temporary pointers                   */
    int i, j;               /* Temporary integers                   */
    char ip_comp[4];        /* IP address component                 */

	/* Banner */
    printf ("Netsetup Version 1.02, libsocket networking set-up\n"
            "Copyright 1997, 1998 by Richard Dawe\n");

    printf ("libsocket Copyright 1997, 1998 by Indrek Madre\n"
            "libsocket Copyright 1997, 1998 by Richard Dawe\n\n");

    /* Info message */
    printf("Netsetup will ask you a series of questions about your TCP/IP "
           "configuration. It\nmay help you to look at the TCP/IP settings "
           "in the Network Control Panel or run\n'winipcfg'. See the "
           "libsocket documentation for further help.\n\n");

	/* --- Ask the questions --- */
	fflush(stdin);

	/* Computer's details */
	printf("Please enter your computer's name: ");
	fflush(stdout);
	scanf("%255s", pc_name);

	printf("Please enter your computer's IP address: ");
	fflush(stdout);
	scanf("%255s", pc_ip);

	/* Domain name */
	printf("Please enter your domain name: ");
	fflush(stdout);
	scanf("%255s", domain_name);

    /* Network address */
    printf("Do you know your network mask [Yn]? ");
	fflush(stdout);
	res = getch();

    if ((res == 'y') || (res == 'y') || (res == '\r')) {
        printf("y\n");
        net_mask_known = TRUE;
    } else {
		printf("n\n");
        net_mask_known = FALSE;
	}

    if (net_mask_known == TRUE) {
        /* Get the net mask */
        printf("Please enter your network mask (e.g. 255.255.255.0): ");
        fflush(stdout);
        scanf("%255s", net_mask);
    } else {
        /* Get the network IP address */
        printf("Do you know your network IP address [Yn]? ");
        fflush(stdout);
        res = getch();

        if ((res == 'y') || (res == 'y') || (res == '\r')) {
            printf("y\n");

            /* Get the network IP address */
            printf("Please enter your network IP address: ");
            fflush(stdout);
            scanf("%255s", net_ip);
        } else {
            printf("n\n");
            net_mask_known = TRUE;
            sprintf(net_mask, "255.255.255.0");
            printf("INFO: Using '%s' as the network mask\n", net_mask);
        }        
    }

    /* Construct net_ip from net_mask, if necessary */
    if (net_mask_known == TRUE) {
        /* Parse dotted-quads */        
        p = strtok(net_mask, ".");
        *net_ip = '\0';

        q = pc_ip;
        for (i = 0;
             (*q != '.') && (*q != '\0');
             ip_comp[i] = *q, q++, i++
            ) {;}
        ip_comp[i] = '\0', q++;

        while ( (p != NULL) && (*ip_comp != '\0') ) {
            /* Get components as integers */
            i = atoi(p);
            j = atoi(ip_comp);

            /* If this part of the network mask is zero, finish */
            if (i == 0) break;

            /* Add to network IP address */
            sprintf(net_ip, "%s%d.", net_ip, i & j);

            /* Next! */
            p = strtok(NULL, ".");

            for (i = 0;
                 (*q != '.') && (*q != '\0');
                 ip_comp[i] = *q, q++, i++
                ) {;}
            ip_comp[i] = '\0', q++;
        }

        /* Nuke trailing '.' */
        p = strrchr(net_ip, '.');
        if (p != NULL) *p = '\0';

        printf("INFO: Using '%s' as the network IP address\n", net_ip);
    }

	/* DNS details */
	printf("Do you have a DNS server [Yn]? ");
	fflush(stdout);
	res = getch();

    if ((res == 'y') || (res == 'y') || (res == '\r')) {
		printf("y\n");
		dns_present = TRUE;
    } else {
		printf("n\n");
		dns_present = FALSE;
	}

    if (dns_present == TRUE) {
		printf ("Please enter the IP address of your DNS server: ");
		fflush(stdout);
		scanf("%255s", dns_ip);
	}

	/* --- Create the files --- */

    printf("\n");

	/* host.conf */
    if ((fOut = fopen("host.conf", "wt")) == NULL) {
		fprintf(stderr,  "Unable to open 'host.conf'!\n");
		exit(0);
	}

	printf("Writing host.conf...");

	if (dns_present == TRUE)
		fprintf(fOut, "order bind, hosts\n");	/* Assume DNS up-to-date */
    else {
        /* RD: Bugfix: No! No DNS => don't output "bind" */
        /*fprintf(fOut, "order hosts, bind\n");*/
        fprintf(fOut, "order hosts");
    }

	printf("done\n");
	fclose(fOut);

	/* resolv.conf */
    if ((fOut = fopen("resolv.conf", "wt")) == NULL) {
        fprintf(stderr,  "Unable to open 'resolv.conf'!\n");
		exit(0);
	}

	printf("Writing resolv.conf...");

	fprintf(fOut, "domain %s\n", domain_name);
	if (dns_present == TRUE) fprintf(fOut, "nameserver %s\n", dns_ip);

	printf("done\n");
	fclose(fOut);

	/* hosts.eg */
    if ((fOut = fopen("hosts.eg", "wt")) == NULL) {
        fprintf(stderr,  "Unable to open 'host.eg'!\n");
		exit(0);
	}

	printf ("Writing hosts.eg (an example hosts file)...");

	fprintf(fOut, "# hosts.eg\n# An example hosts file\n");
	fprintf(fOut, "# Please edit this to suit your site\n\n");
    fprintf(fOut, "127.0.0.1\tlocalhost\n");
    fprintf(fOut, "%s\t%s\n", pc_ip, pc_name);

	printf("done\n");
	fclose(fOut);

    /* networks.eg */
    if ((fOut = fopen("networks.eg", "wt")) == NULL) {
        fprintf(stderr,  "Unable to open 'networks.eg'!\n");
		exit(0);
	}

    printf ("Writing networks.eg (an example networks file)...");

    fprintf(fOut, "# networks.eg\n# An example networks file\n");
	fprintf(fOut, "# Please edit this to suit your site\n\n");
    fprintf(fOut, "127\tloopback\n");
    fprintf(fOut, "%s\t%s\n", net_ip, domain_name);

	printf("done\n");
	fclose(fOut);

	/* Installation message */
	printf("\n");

    printf("Please copy the files 'hosts.eg', 'host.conf', 'resolv.conf' into "
           "your Windows\ndirectory. 'hosts.eg' and 'networks.eg' should be "
           "renamed to 'hosts' and\n'networks', but check that these files "
           "don't already exist, or make backups.\n");

	/* Success */
/*	return(1); IM: 1 is wrong */
	return (0);
}
