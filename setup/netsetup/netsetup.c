/*
 *  netsetup.c
 *  Networking parameters set-up program for libsocket, Version 1.2.1
 *  Copyright (C) 1997-2000 by Richard Dawe
 *
 *  ----
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * TODO:
 *
 * . Offer the choice of backing up config files if they exist, in addition
 *   to overwriting them.
 *
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <resolv.h>

#define TRUE  -1
#define FALSE  0

static void banner (void);
static void clean_stdin (void);

static void addforwardslash (char *p);
static void forwardslashify (char *str);
static int recursive_mkdir (const char *pathname, mode_t mode);

static char *strdupnx (const char *str, int n);

static int yesorno (void);
static char *getdnsname (size_t maxlen);
static char *getipaddr (void);
static int check_net_mask (struct in_addr *mask);

static int die (const char *msg);

/* --------
   - main -
   -------- */

int
main (void)
{
  int  dns_present;           /* Boolean                */
  char host_name[256];        /* Host name              */
  char host_name_short[256];  /* Host name without dom. */
  char domain_name[256];      /* Domain name            */
  char *lsck_dir = NULL;      /* libsocket's config dir */
  char *win_dir = NULL;       /* Windows directory      */
  char filename[PATH_MAX];    /* Temp. filename         */

  struct in_addr ip_addr, net_mask, net_addr, dns_addr[MAXNS + 1];
  char net_addr_str[16];      /* Net address in string  */

  char line[256];

  FILE *fOut; /* Output file handle */
  char *p, *q;
  int found;
  int i, j, k;
  int ret;
  char *pret;

  bzero(&ip_addr,  sizeof(ip_addr));
  bzero(&net_mask, sizeof(net_mask));
  bzero(&net_addr, sizeof(net_addr));
  bzero(&dns_addr, sizeof(dns_addr));

  /* Banner */
  banner();

  /* Message about answering questions, aborting, etc. */
  printf("When asked a question, use 'y' or 'Y' for yes, 'n' or 'N' for no,"
	 "'q' or 'Q'\n"
	 "to quit. You can also use ESC to quit (abort) at any point.\n\n");

  /* --- Ask the questions --- */

  /* - libsocket configuration info - */
  printf("* libsocket Configuration Directory *\n\n");
    
  if (getenv("LSCK") == NULL) {        
    if (getenv("LIBSOCKET") != NULL) {
      lsck_dir = strdup(getenv("LIBSOCKET"));
      forwardslashify(lsck_dir);

      printf("WARNING: Please use LSCK environment variable "
	     "instead of LIBSOCKET.\n");
    }
  } else {
    lsck_dir = strdup(getenv("LSCK"));
    forwardslashify(lsck_dir);
  }

  printf("The libsocket environment variable, LSCK, specifies the directory "
	 "where\nlibsocket's configuration files are stored.\n\n");

  if (lsck_dir == NULL) {
    printf("LSCK has not been set. Do you wish to set it up [ynq]? ");
    ret = yesorno();
    if (ret == -1)
      die("User ended netsetup");

    if (ret == 1) {
      printf("Please enter the libsocket configuration directory "
	     "(e.g. c:/lsck):\n");
      fflush(stdout);
      clean_stdin();
      scanf("%255s", line);

      /* Set up the new var */
      p = line + strlen(line) - 1;    /* Trailing slash? */
      if ((*p == '\\') || (*p == '/')) *p = '\0';

      lsck_dir = strdup(line);
      forwardslashify(lsck_dir);
      setenv("LSCK", lsck_dir, 1);

      /* Add to AUTOEXEC.BAT? */
      printf("Add the line 'SET LSCK=%s' to c:/autoexec.bat [ynq]? ",
	     lsck_dir);
      ret = yesorno();
      if (ret == -1)
	die("User ended netsetup");

      if (ret == 1) {
	fOut = fopen("c:/autoexec.bat", "at");
	if (fOut != NULL) {
	  fprintf(fOut, "SET LSCK=%s\n", lsck_dir);
	  fclose(fOut);
	}
      }

      /* Create the directory */
      recursive_mkdir(lsck_dir, S_IWUSR);
    } else {
      printf("netsetup needs the configuration directory to be set!\n");
      exit(EXIT_FAILURE);
    }
  } else {      
    printf("The libsocket configuration directory is '%s'.\n", lsck_dir);
  }

  /* windir - the Windows directory - handle the case where this is not
   * present. */
  printf("\n");
  if (getenv("windir") == NULL) {
    win_dir = strdup(lsck_dir);
    forwardslashify(win_dir);
    printf("Windows directory not found - netsetup will put all "
	   "configuration files in\n'%s'.\n", lsck_dir);
  } else {
    win_dir = strdup(getenv("windir"));
    forwardslashify(win_dir);
    printf("Windows directory is '%s'.\n", win_dir);
  }

  /* - Host & domain name - */
  printf("\n"
	 "* Host and domain name *\n\n"
	 "These are required if you wish to use text names rather than IP "
	 "addresses.\n\n");

  /* Host name */
  printf("Please enter your computer's host name: ");
  fflush(stdout);

  pret = getdnsname(sizeof(host_name));
  if (pret == NULL)
    die("User ended netsetup or an error occurred getting DNS host name");
  strcpy(host_name, pret);

  /* Build a suggested domain name from the host name */
  p = strchr(host_name, '.');
  if (p != NULL) {
    strcpy(domain_name, p + 1);
    printf("Using '%s' as the domain name.\n", domain_name);
  } else {        
    /* Domain name */
    printf("Do you have a domain name [ynq]? ");
    ret = yesorno();
    if (ret == -1)
      die("User ended netsetup");

    if (ret == 1) {
      printf("Please enter your computer's domain name: ");
      pret = getdnsname(sizeof(host_name));
      if (pret == NULL)
	die("User ended netsetup or an error occurred getting "
	    "DNS domain name");

      /* Now add this to the host name */
      strcat(host_name, ".");
      strcat(host_name, pret);
      printf("Now using '%s' as host name.\n", host_name);
    }
  }

  /* - IP address details - */
  printf("\n"
	 "* IP Address Information *\n\n"
	 "This is required for IP networking and text name to IP address "
	 "mappings\n(name resolving).\n\n");

  /* IP address */
  printf("Please enter your computer's IP address (e.g. 192.168.1.123):\n");
  
  ret = 0;
  while (ret == 0) {
    pret = getipaddr();
    if (pret == NULL)
      die("User ended netsetup or an error occurred getting the IP address");

    ret = inet_aton(pret, &ip_addr);
    if (!ret)
      printf("Invalid IP address: %s\n", pret);
  }

  /* Network mask */
  printf("Please enter your computer's IP network mask "
	 "(e.g. 255.255.255.0):\n");

  ret = 0;
  while (ret == 0) {
    pret = getipaddr();
    if (pret == NULL)
      die("User ended netsetup or an error occurred getting "
	  "the IP network mask");

    ret = inet_aton(pret, &net_mask);
    if (!ret || !check_net_mask(&net_mask)) {
      printf("Invalid IP mask: %s\n", pret);
      ret = 0;
    }
  }

  /* Make network address from network mask */
  net_addr.s_addr = ip_addr.s_addr & net_mask.s_addr;
  printf("Using %s as the network address.\n\n", inet_ntoa(net_addr));

  /* DNS details */
  bzero(dns_addr, sizeof(dns_addr));

  printf("Do you have a DNS server [ynq]? ");
  ret = yesorno();
  if (ret == -1)
    die("User ended netsetup");  

  dns_present = (ret == 1) ? TRUE : FALSE;

  if (dns_present == TRUE) {
    for (i = 0; i < MAXNS; i++) {
      printf ("Please enter the IP address for DNS server %i "
	      "(enter for none):\n", i + 1);

      ret = 0;
      while (ret == 0) {
	pret = getipaddr();
	if (pret == NULL)
	  die("User ended netsetup or an error occurred getting"
	      "a DNS server's IP address");

	ret = inet_aton(pret, &dns_addr[i]);
	if (!ret)
	  printf("Invalid DNS server IP address: %s\n", pret);
      }
      
      printf("Is there another DNS server [ynq]? ");
      ret = yesorno();
      if (ret == -1)
	die("User ended netsetup");
      if (ret == 0)
	break;
    }
  }

  /* Print a summary and ask for confirmation. */
  printf("\nSummary of configuration:\n"
	 "\n");

  printf("libsocket directory: %s\n"
	 "Windows directory:   %s\n"
	 "\n",
	 lsck_dir, win_dir);

  printf("Host name:           %s\n", host_name);
  printf("IP address/mask:     %s/", inet_ntoa(ip_addr));
  printf("%s\n", inet_ntoa(net_mask));
  printf("IP network address:  %s\n", inet_ntoa(net_addr));
  printf("\n");

  printf("DNS servers:");
  if (dns_present == TRUE) {
    printf("\n");
    for (i = 0; i < MAXNS; i++) {
      if (dns_addr[i].s_addr == 0) break;
      printf("\t%s\n", inet_ntoa(dns_addr[i]));
    }
  } else {
    printf(" None configured\n");
  }
  printf("\n");

  printf("Is this configuration OK [ynq]? ");
  ret = yesorno();
  if ((ret == 0) || (ret == -1))
    die("User ended netsetup");

  /* --- Create the files --- */
    
  /* Installation message */
  printf("\n"
	 "* Creating Files *\n\n");

  /* - lsck.cfg - */
  sprintf(filename, "%s/lsck.cfg", lsck_dir);
  if (access(filename, F_OK) == 0) {
    printf("%s already exists - overwrite [ynq]? ", filename);
    fflush(stdout);
    clean_stdin();
    scanf("%[yYnN]", line);
    if ((*line == 'N') || (*line == 'n')) exit(EXIT_FAILURE);
  }
  printf("Creating %s...", filename);

  fOut = fopen(filename, "wt");
  if (fOut == NULL) {
    printf("failed\n");
    perror("netsetup");
  } else {
      /* Header */
    fprintf(fOut,
	    "# lsck.cfg\n"
	    "# libsocket configuration file\n"
	    "# Created by netsetup\n"
	    "#\n\n");

    /* Host & domain name */
    fprintf(fOut,
	    "[main]\n"
	    "hostname=%s\n\n",
	    host_name);

    /* As libsocket only runs under Windows, use Windows config files */
    fprintf(fOut,
	    "# Use some Windows configuration files\n"
	    "hosts=%s/hosts\n"
	    "networks=%s/networks\n"
	    "services=%s/services\n"
	    "protocols=%s/protocol\n\n",
	    win_dir, win_dir, win_dir, win_dir);

    /* Resolver config files */
    fprintf(fOut,
	    "# Resolver configuration files\n"
	    "host.conf=%s/host.cfg\n"
	    "resolv.conf=%s/resolv.cfg\n\n",
	    lsck_dir, lsck_dir);

    /* Global remote auto-login files */
    fprintf(fOut,
	    "# Global remote auto-login files\n"
	    "hosts.equiv=%s/hosts.equ\n"
	    ".rhosts=%s/rhosts\n"
	    ".netrc=%s/netrc\n\n",
	    lsck_dir, lsck_dir, lsck_dir);

    /* Done */
    printf("done\n");
    fclose(fOut);
  }

  /* - hosts - */    
  strcpy(host_name_short, host_name);
  p = strchr(host_name_short, '.');
  if (p != NULL) *p = '\0';

  p = inet_ntoa(ip_addr);

  sprintf(filename, "%s/hosts", win_dir);
  printf("Modifying %s...", filename);

  fOut = fopen(filename, "a+t");
  if (fOut == NULL) {
    printf("failed\n");
    perror("netsetup");
  } else {
    rewind(fOut);
    found = 0;

    /* Look for the IP address & host name */
    while(fgets(line, sizeof(line), fOut) != NULL) {
      q = line;
      while ((*q == ' ') || (*q == '\t')) q++;
      if (*q == '#') continue;

      if (   (strstr(q, p)               != NULL)
	  && (strstr(q, host_name_short) != NULL)) {
	found = 1;
	break;
      }
    }

    fseek(fOut, 0, SEEK_END);
    if (!found) fprintf(fOut, "%s    %s\n", p, host_name_short);

    /* Done */
    if (!found)
      printf("done\n");
    else
      printf("done (no changes needed)\n");
    fclose(fOut);
  }

  /* - networks - */

  /* Build the network mask in string form */
  bzero(net_addr_str, sizeof(net_addr_str));
  for (i = 3; i > 0; i--) {
    j = (ntohl(net_addr.s_addr) >> (i * 8)) & 0xFF;
    k = (ntohl(net_mask.s_addr) >> (i * 8)) & 0xFF;
    if (k != 0) {
      sprintf(line, "%d", j);
      if (i < 3) strcat(net_addr_str, ".");
      strcat(net_addr_str, line);
    }
  }

  sprintf(filename, "%s/networks", win_dir);
  printf("Modifying %s...", filename);

  fOut = fopen(filename, "a+t");
  if (fOut == NULL) {
    printf("failed\n");
    perror("netsetup");
  } else {
    rewind(fOut);
    found = 0;

    /* No domain name => this isn't needed, so skip it */
    if (*domain_name == '\0') found = 1;

    /* Look for the IP address & host name */
    while(fgets(line, sizeof(line), fOut) != NULL) {
      q = line;
      while ((*q == ' ') || (*q == '\t')) q++;
      if (*q == '#') continue;

      if (   (strstr(q, net_addr_str) != NULL)
	  && (strstr(q, domain_name)  != NULL)) {
	found = 1;
	break;
      }
    }

    fseek(fOut, 0, SEEK_END);
    if (!found) fprintf(fOut, "%s    %s\n", net_addr_str, domain_name);

    /* Done */
    if (!found) printf("done\n"); else printf("done (none needed)\n");
    fclose(fOut);
  }

  /* - host.cfg - */
  sprintf(filename, "%s/host.cfg", lsck_dir);
  if (access(filename, F_OK) == 0) {
    printf("%s already exists - overwrite [ynq]? ", filename);
    fflush(stdout);
    clean_stdin();
    scanf("%[yYnN]", line);
    if ((*line == 'N') || (*line == 'n')) exit(EXIT_FAILURE);
  }
  printf("Creating %s...", filename);

  fOut = fopen(filename, "wt");
  if (fOut == NULL) {
    printf("failed\n");
    perror("netsetup");
  } else {
    fprintf(fOut,
	    "# host.conf\n"
	    "# Resolver configuration file\n"
	    "# Created by netsetup\n\n");

    if (dns_present == TRUE)
      fprintf(fOut, "order hosts, bind\n");
    else
      fprintf(fOut, "order hosts\n");

    /* Done */
    printf("done\n");
    fclose(fOut);
  }

  /* - resolv.cfg - */
  sprintf(filename, "%s/resolv.cfg", lsck_dir);
  if (access(filename, F_OK) == 0) {
    printf("%s already exists - overwrite [ynq]? ", filename);
    fflush(stdout);
    clean_stdin();
    scanf("%[yYnN]", line);
    if ((*line == 'N') || (*line == 'n')) exit(EXIT_FAILURE);
  }
  printf("Creating %s...", filename);

  fOut = fopen(filename, "wt");
  if (fOut == NULL) {
    printf("failed\n");
    perror("netsetup");
  } else {
    fprintf(fOut,
	    "# resolv.conf\n"
	    "# Resolver configuration file\n"
	    "# Created by netsetup\n\n");

    if (dns_present == TRUE) {
      for (i = 0; i < MAXNS; i++) {
	if (dns_addr[i].s_addr == 0) break;
	fprintf(fOut, "nameserver %s\n", inet_ntoa(dns_addr[i]));
      }
    }

    /* Done */
    printf("done\n");
    fclose(fOut);
  }

  /* Tidy up */
  free(lsck_dir);
  free(win_dir);

  /* Success */
  return(EXIT_SUCCESS);
}

/* ----------
   - banner -
   ---------- */

static void
banner (void)
{
  /* Copyright */
  printf ("Netsetup Version 1.2.1, libsocket networking set-up\n"
	  "Copyright (C) 1997-2000 by Richard Dawe\n");

  printf ("libsocket Copyright 1997, 1998 by Indrek Mandre\n"
	  "libsocket Copyright 1997-2000 by Richard Dawe\n\n");

  /* Info message */
  printf("Netsetup will ask you a series of questions about your TCP/IP "
	 "configuration.\nIt may help you to look at the TCP/IP settings "
	 "in the Network Control Panel\nor run 'winipcfg'. See the "
	 "libsocket documentation for further help.\n\n");

  fflush(stdout);
}

/* ---------------
 * - clean_stdin -
 * --------------- */

/* Remove all whitespace from stdin, so scanf works properly. */

static void
clean_stdin (void)
{
  int c;

  /* Remove any whitespace */
  while ( (c = getc(stdin)) != EOF ) {
    if (!isspace(c)) {
      ungetc(c, stdin);
      break;
    }
  }
}

/* -------------------
 * - addforwardslash -
 * ------------------- */

/* Add a forward slash to the end safely, i.e. if has one, don't do it. */
static void
addforwardslash (char *p)
{
  if (p == NULL) return;
  if (p[strlen(p) - 1] == '/') return;
  strcat(p, "/");
}

/* -------------------
 * - forwardslashify -
 * ------------------- */

static void
forwardslashify (char *str)
{
  char *p;

  if (str == NULL) return;
  for (p = str; *p != '\0'; p++) { if (*p == '\\') *p = '/'; }
}

/* -----------
 * - yesorno -
 * ----------- */

/* This returns 1 on yes, 0 on no, -1 on quit/abort. Loop until we get
 * a valid response. */

static int
yesorno (void)
{
  int key;

  for (;;) {
    key = getch();

    switch(key) {
    case 'y':
    case 'Y':
      printf("%c\n", key);
      return(1);

    case 'n':
    case 'N':
      printf("%c\n", key);
      return(0);

    case 'q':
    case 'Q':
    case '\e':
      printf("%c\n", key);
      return(-1);
    }
  }
}

/* --------------
 * - getdnsname -
 * -------------- */

/*
 * Get a DNS-format name from the user. Valid characters are [A-Za-z0-9.-].
 * The maximum component length is 64 characters. 'max_len' specifies
 * the maximum total length, including nul character.
 *
 * On success, a pointer to a static buffer containing the name is returned;
 * otherwise NULL is returned.
 *
 */

static char *
getdnsname (size_t maxlen)
{
  static char buf[256];
  size_t      len = 0;
  int         ok  = 1;
  int         key;

  if (maxlen > sizeof(buf))
    maxlen = sizeof(buf);
  maxlen--; /* reserve space for the nul */

  while (len < maxlen) {
    key = getch();

    /* - Control keys - */

    /* Escape => abort */
    if (key == '\e') {
      ok = 0;
      break;
    } else if (key == '\b') {
      /* Backspace */
      if (len == 0) {
	putch('\a');
	continue;
      }

      printf("\b \b");
      len--;
    } else if ((key == '\r') || (key == '\n')) {
      /* Input complete. If there's a tailing dot, strip it off. */
      printf("\n");

      if ((len > 0) && (buf[len - 1] == '.')) {
	len--;
	buf[len] = '\0';
      }
      break;
    }

    /* If we're at the maximum buffer size, wait for line completion. */
    if (len == maxlen) {
      putch('\a');
      continue;
    }

    /* - Data keys - */

    if ((isalnum(key) || (key == '-')) && (key != '.')) {
      putch(key);
      buf[len] = (char) (key & 0xff);
      len++;
    } else if (key == '.') {
      if (   (len == 0)
	  || ((len > 0) && (buf[len - 1] == '.'))) {
	/* Not ready for '.' yet */
	putch('\a');
	continue;
      }

      putch(key);
      buf[len] = '.';
      len++;
    } else {
      /* Bad character */
      putch('\a');
    }
  }  

  if (ok) {
    buf[len] = '\0';
    return(buf);
  }

  return(NULL);
}

/* -------------
 * - getipaddr -
 * ------------- */

/*
 * Get a dotted-quad IP address from the user. On success, a pointer to
 * a static buffer containing the name is returnd; otherwise NULL is returned.
 */

#define IP_MAX_COMPS   4

static char *
getipaddr (void)
{
  static char buf[16]; /* dotted quad + nul = (4 * 3) + 3 + 1 */
  size_t      maxlen = sizeof(buf) - 1;
  size_t      len    = 0;
  int         ncomps = 1;
  int         ok     = 1;
  int         key;

  while (len <= maxlen) {
    key = getch();

    /* - Control keys - */

    /* Escape => abort */
    if (key == '\e') {
      ok = 0;
      break;
    } else if (key == '\b') {
      /* Backspace */
      if (len == 0) {
	putch('\a');
	continue;
      } else {
	if (buf[len - 1] == '.')
	  ncomps--;
      }

      printf("\b \b");
      len--;
    } else if ((key == '\r') || (key == '\n')) {
      /* We need four components to complete input. */
      if (   (ncomps < 4)
	  || ((ncomps == 4) && (!isdigit(buf[len - 1])))) {
	putch('\a');
	continue;
      }

      /* Input complete */
      printf("\n");
      break;
    }

    /* If we're at the maximum buffer size, wait for line completion. */
    if (len == maxlen) {
      putch('\a');
      continue;
    }

    /* - Data keys - */

    if (isdigit(key)  && (key != '.')) {
      putch(key);
      buf[len] = (char) (key & 0xff);
      len++;
    } else if (key == '.') {
      if (   (len == 0)
	  || ((len > 0) && (buf[len - 1] == '.'))) {
	/* Not ready for '.' yet */
	putch('\a');
	continue;
      }

      if (ncomps == IP_MAX_COMPS) {
	/* Already have the number of components we need. */
	putch('\a');
	continue;
      }

      putch(key);
      buf[len] = '.';
      len++;
      ncomps++;
    } else {
      /* Bad character */
      putch('\a');
    }
  }

  if (ok) {
    buf[len] = '\0';
    return(buf);
  }

  return(NULL);
}

/* -------
 * - die -
 * ------- */

static int
die (const char *msg)
{
  fprintf(stderr, "%s\n", msg);
  exit(EXIT_FAILURE);
}

/* ------------
 * - strdupnx -
 * ------------ */

/* Copy a string and allocate n extra bytes. */
static char *
strdupnx (const char *str, int n)
{
  char *p = NULL;

  if (str == NULL) return(NULL);
  p = (char *) calloc(strlen(str) + 1 + n, 1);
  if (p == NULL) return(NULL);
  strcpy(p, str);
  return(p);
}

/* -------------------
 * - recursive_mkdir -
 * ------------------- */

/* This works like mkdir, except that all the directory components are created.
 * This is like 'mkdir -p' from GNU fileutils. */

static int
recursive_mkdir (const char *pathname, mode_t mode)
{
  /* Allocate space to add trailing slash, if necessary. */
  char *buf = strdupnx(pathname, 1);
  char *p   = buf;
  int ret;

  addforwardslash(buf);
	
#ifdef __DJGPP__
  /* Skip the drive specifier. */
  if (buf[1] == ':') p += 2;
#endif

  while ((p = strchr(p + 1, '/')) != NULL) {
    /* Quit if it's a trailing slash */
    if (*p == '\0') break;

    /* Chop up temporarily & create a component. */
    *p = '\0';
    ret = mkdir(buf, mode);
    *p = '/';

    if ((ret != 0) && (errno != EEXIST)) {
      /* Pass down error */
      free(buf);
      return(-1);
    }
  }

  free(buf);
  return(0);
}

/* ------------------
 * - check_net_mask -
 * ------------------ */

static int
check_net_mask (struct in_addr *mask)
{
  unsigned long m   = (unsigned long) mask->s_addr;
  int           ret = 1;
  int           found_zero, i, bit;

  for (found_zero = i = 0; i < 31; i++) {
    bit = (m >> i) & 1;

    /* Find first zero bit */    
    if (bit == 0) {
      found_zero = 1;
      continue;
    }

    /* If we've found a zero bit, we should not find a one bit. */
    if (bit && found_zero) {
      ret = 0;
      break;
    }
  }

  return(ret);
}
