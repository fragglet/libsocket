/* Copyright (C) 1995 Free Software Foundation, Inc.
 * This file is part of the GNU C Library.
 *
 * The GNU C Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * The GNU C Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the GNU C Library; see the file COPYING.LIB.  If
 * not, write to the Free Software Foundation, Inc., 675 Mass Ave,
 * Cambridge, MA 02139, USA.
 *
 * Author:  Swen Thuemmler <swen@uni-paderborn.de>
 */


/*#include <ansidecl.h>*/ /* Richard Dawe for libsocket: Don't need this! */
#include <stddef.h>
#include <ctype.h>

#if defined(YP)
#include <rpcsvc/yp_prot.h>
#include <rpcsvc/ypclnt.h>

extern void setnetgrent(const char *);
extern void endnetgrent(void);
extern int getnetgrent(char **, char **, char **);
extern int innetgr(const char *, const char *, const char *, const char *);
extern int __yp_check(char **);

struct netgrentry 
{
  char *host;
  char *user;
  char *domain;
};


struct netgrlist 
{
  int maxmembers;
  int members;
  struct netgrentry *list;
};


static void _expand_netgroupentry(const char *, struct netgrlist *);
static void _parse_entry(char *, char *, struct netgrlist *);
static void _netgr_free(struct netgrlist *);
static struct netgrlist list = { 0, 0, NULL };
static int first = 1;
static char *netgroup = NULL ;
static char *nisdomain = NULL ;

void
setnetgrent(const char *netgr)
{
  if (NULL == netgroup || 0 != strcmp(netgroup, netgr))
    {
      endnetgrent();
      netgroup = strdup(netgr);
      _expand_netgroupentry(netgr, &list);
    }
  first = 1;
}

void
endnetgrent(void)
{
  if (NULL != netgroup)
    {
      free(netgroup);
      netgroup = NULL;
    }
  
  if (NULL != list.list)
      _netgr_free(&list);
  first = 1;
}

int
getnetgrent(char **machinep, char **userp, char **domainp)
{
  static int current = 0;
  struct netgrentry *entry;
    
  if (1 == first)
    current = first = 0;
  else
    current++;

  if (current < list.members)
    {
      entry = &list.list[current];
      *machinep = entry->host;
      *userp = entry->user;
      *domainp = entry->domain;
      return 1;
    }
  return 0;
}

int
innetgr(const char *netgr, const char *host, const char *user, const char *domain)
{
  struct netgrlist list = { 0, 0, NULL };
  struct netgrentry *entry;
  int current, status = 0;
  
  _expand_netgroupentry(netgr, &list);
  for (current = 0; current < list.members; current++)
    {
      entry = &list.list[current];
          /* This may look weired, but it is correct */
      status =
        ((NULL == host) ? 1 : ((NULL == entry->host) ? 1 : 0 == strcmp(host, entry->host)))
        && ((NULL == user) ? 1 : ((NULL == entry->user) ? 1 : 0 == strcmp(user, entry->user)))
        && ((NULL == domain) ? 1 : ((NULL == entry->domain) ? 1 : 0 == strcmp(domain, entry->domain)));
      if (1 == status)
        break;
    }
  if (NULL != list.list)
    _netgr_free(&list);
  return status;
}

static void
_netgr_free(struct netgrlist *list)
{
  int i;
  for (i = 0; i < list->members; i++)
  {
    free(list->list[i].host);
    free(list->list[i].user);
    free(list->list[i].domain);
  }
  free(list->list);
  list->maxmembers = 0;
  list->members = 0;
  list->list = NULL;
}

static void
_expand_netgroupentry(const char *netgr, struct netgrlist *list)
{
  char *outval;
  int outvallen, status;
  char *start, *end, *realend;
    
  if ('\0' == *netgr)
    return;
  if (1 == __yp_check(NULL))
    {
      if (NULL == nisdomain)
        yp_get_default_domain(&nisdomain);
      status = yp_match(nisdomain, "netgroup",
                        netgr, strlen(netgr),
                        &outval, &outvallen);
      if (0 != status)
        return ;
          /* outval enthaelt den Eintrag. Zuerst Leerzeichen ueberlesen */
      start = outval;
      realend = start + strlen(outval);
      while (isspace(*start) && start < realend)
        start++;
      while (start < realend)
        {
          if ( '(' == *start ) /* Eintrag gefunden */
            {
              end = strchr(start, ')') ;
              if (NULL == end)
                return ;
              _parse_entry(start + 1, end, list);
            }
          else
            {
              end = start + 1;
              while ('\0' != *end && !isspace(*end))
                end++;
              *end = '\0';
              _expand_netgroupentry(start, list);
            }
          start = end + 1 ;
          while (isspace(*start) && start < realend)
            start++;
        }
      free(outval);
    }
}

static void
_parse_entry(char *start, char *end, struct netgrlist *list)
{
  char *host, *user, *domain;
  struct netgrentry *entry;
      /* First split entry into fields. Return, when finding malformed entry */
  host = start ;
  start = strchr(host, ',') ;
  if (NULL == start || start >= end)
    return ;
  *start = '\0' ;
  user = start + 1 ;
  start = strchr(user, ',') ;
  if (NULL == start || start >= end)
    return ;
  *start = '\0';
  domain = start + 1;
  if (start > end)
    return ;
  *end = '\0' ;
      /* Entry is correctly formed, put it into the list */
  if (0 == list->maxmembers)
    {
      list->list = malloc(10 * sizeof(struct netgrentry));
      if ( NULL != list->list )
        list->maxmembers = 10 ;
    }

  if (list->members == list->maxmembers)
    {
      list->list = realloc(list->list,
                           (list->maxmembers + 10) * sizeof(struct netgrentry));
      if (NULL == list->list)
        {
          list->maxmembers = 0;
          list->members = 0;
          return;
        }
      list->maxmembers += 10;
    }
      /*
       * FIXME: this will not handle entries of the form ( asdf, sdfa , asdf )
       * (note the spaces). This should be handled better!
       */
  entry = &list->list[list->members];
  entry->user = ('\0' == *user) ? NULL : strdup(user);
  entry->host = ('\0' == *host) ? NULL : strdup(host);
  entry->domain = ('\0' == *domain) ? NULL : strdup(domain);
  list->members++ ;
  return;
}

#else

int
getnetgrent(char **machinep, char **userp, char **domainp)
{
  return 0;
}

void
setnetgrent(char *netgroup)
{
  return ;
}


void
endnetgrent(void)
{
  return ;
}

int
innetgr(char *netgroup, char *machine, char *user, char *domain)
{
  return 0;
}

#endif /* YP */
