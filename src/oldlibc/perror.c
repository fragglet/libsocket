/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <string.h>
#include <errno.h>

/* This file just needs recompilation to find libsocket's version of
 * strerror(), which calls DJGPP's libc's strerror() to use old error
 * messages. */

void
perror(const char *s)
{
  fprintf(stderr, "%s: %s\n", s, strerror(errno));
}
