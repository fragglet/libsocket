/*
 * l_res.c
 * Copyright unknown - presumably this is part of the Linux networking code.
 *
 * This file is part of libsocket.
 * libsocket Copyright 1997, 1998 by Indrek Mandre
 * libsocket Copyright 1997, 1998 by Richard Dawe
 */

#include <sys/param.h>

#include <stdio.h>
#include <ctype.h>

#include <lsck/ws.h>

#pragma weak _res = __res

#undef _res;
extern struct __res_state _res;
struct __res_state __res = {0};

#pragma weak __res_status_location = __normal__res_status_location

/* The one in libpthread will override __res_status_location () */
struct __res_state *__normal__res_status_location ( void );

struct __res_state *
__normal__res_status_location ( void )
{
  return &_res;
}
