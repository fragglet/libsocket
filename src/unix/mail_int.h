/*
 * mail_int.h
 * Copyright 1999, 2000 by Richard Dawe
 * 
 * ---
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* This file was taken from libmslot for use in libsocket. */

#ifndef __mailslot_internal_h__
#define __mailslot_internal_h__

#ifndef CARRY
#define CARRY 1
#endif

#define MAILSLOT_MAX        256	       		/* Upto 256 mailslots      */
#define MAILSLOT_DEV        "/dev/mailslot/"	/* Prefix used in FSEXT    */

/* Do not use names that break the 8+3 convention, otherwise there will be
 * problems on Windows NT. */
#define MAILSLOT_LOCAL      "local/"		/* Prefix used for local   */
#define MAILSLOT_BROADCAST  "bcast/"		/* Prefix used for b'casts */

#endif /* __mailslot_internal_h__ */
