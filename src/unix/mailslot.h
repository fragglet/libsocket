/*
 * mailslot.h
 * Copyright 1998-2000 by Richard Dawe
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

#ifndef __mailslot_h__
#define __mailslot_h__

/* --- Typedefs --- */

typedef struct _MAILSLOT {
    char *name;			/* Mailslot name          */
    int handle;			/* Mailslot handle        */
    int sel;			/* DOS buffer selector    */
    int seg;			/* DOS buffer segment     */
    int off;			/* DOS buffer offset      */
    int len;			/* DOS buffer length      */
    int maxlen;			/* Maximum message length */
} MAILSLOT;

typedef struct _MAILSLOT_INFO {
    int maxlen;			/* Maximum message length */
    int slotlen;		/* Mailslot size          */
    int nextlen;		/* Next message length    */
    int nextpriority;		/* Next message priority  */
    int queued;			/* No. messages waiting   */
} MAILSLOT_INFO;

/* --- Constants --- */

#define MAILSLOT_MINSIZE        0x1000	/* Minimum mailslot buffer size */
#define MAILSLOT_MAXSIZE	0xFFF6	/* Maximum mailslot buffer size */
#define MAILSLOT_SENDSIZE	0x0FF0	/* Don't send more than 4093
					 * (Win95 bug)                  */
#define MAILSLOT_WAIT_FOREVER	-1

/* Macros */
#ifndef _IO
	
#define IOCPARM_MASK    0x7f            /* parameters must be < 128 bytes */
#define IOC_VOID        0x20000000      /* no parameters */
#define IOC_OUT         0x40000000      /* copy out parameters */
#define IOC_IN          0x80000000      /* copy in parameters */
#define IOC_INOUT       (IOC_IN|IOC_OUT)
                                        /* 0x20000000 distinguishes new &
                                           old ioctl's */
	
#define _IO(x,y)        (IOC_VOID|((x)<<8)|(y))
#define _IOR(x,y,t)     (IOC_OUT|((sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|(y))
#define _IOW(x,y,t)     (IOC_IN|((sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|(y))

#endif	/* _IO */

/* The usual I/O ioctls. */
#ifndef FIONREAD
#define FIONREAD    _IOR('f', 127, int)    /* get # bytes to read */
#endif

#ifndef FIONBIO	
#define FIONBIO     _IOW('f', 126, int)    /* set/clear non-blocking i/o */
#endif

#ifndef FIOASYNC
#define FIOASYNC    _IOW('f', 125, int)    /* set/clear async i/o */
#endif

/* Define some mailslot-specific ioctls. Is this dodgy? */
#define MIOPEEK _IOW('m', 1, int)

/* --- Functions --- */
int __mailslot_create (char * /* name */, int /* len */);

int __mailslot_destroy (int /* hMailslot */);

int __mailslot_read (int    /* hMailslot */,
		     char * /* msg */,
		     int    /* len */,
		     int    /* timeout */);

int __mailslot_peek (int /* hMailslot */, char * /* msg */, int /* len */);

int __mailslot_write (char * /* name */,
		      char * /* msg */,
		      int    /* len */,
		      int    /* timeout */);

int __mailslot_info (int /* hMailslot */, MAILSLOT_INFO * /* mi */);

int __mailslot_install_handler (void);

#endif /* __mailslot_h__ */
