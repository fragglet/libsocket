/*
 * mailslot.c
 * Copyright 1998-2000 by Richard Dawe
 * 
 * LAN Manager Mailslot API routines
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

/*
 * NOTES:
 * 
 * - Mailslots under '95 have to follow 8+3 convention.
 * - All mailslot functions return 0 on success, -1 on failure.
 */

/* This file was taken from libmslot for use in libsocket. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dir.h>

#include <libc/dosio.h>

#include <dpmi.h>
#include <go32.h>
#include <sys/farptr.h>
#include <sys/movedata.h>
#include <sys/segments.h>

#include "mailslot.h"
#include "mail_int.h"

static MAILSLOT *__mailslot[MAILSLOT_MAX];	/* Store mailslot details */
static int __mailslot_inited = 0;		/* Initialised?           */

int __mailslot_init (void);
void __mailslot_uninit (void);

/* Location of mailslot buffer in DOS memory: @ seg:0 (real-mode) or @
 * sel:0 (protected mode). */
static int __mailslot_seg  = 0;
static int __mailslot_sel  = 0;
static int __mailslot_size = 0;

/* -------------------
 * - __mailslot_init -
 * ------------------- */

int __mailslot_init (void)
{
    int paras, i;
    __dpmi_regs r;

    if (__mailslot_inited)
	return (__mailslot_inited);

    /* Check that LAN Manager is present */
    bzero (&r, sizeof (r));
    r.x.ax = 0x5F30;
    __dpmi_int (0x21, &r);
    if (r.x.ax == 0x5F30)
	return (0);		       /* No LAN Manager! */

    for (i = 0; i < MAILSLOT_MAX; i++) {
	__mailslot[i] = NULL;
    }

    /* Allocate DOS memory for mailslot calls. */
    paras = (MAILSLOT_MAXSIZE + 15) >> 4;
    __mailslot_seg = __dpmi_allocate_dos_memory(paras, &__mailslot_sel);
    if (__mailslot_seg == -1) return(0);
    __mailslot_size = paras << 4;

    /* Tidy up on exit. */
    atexit (__mailslot_uninit);

    __mailslot_inited = 1;
    return (1);
}

/* ---------------------
 * - __mailslot_uninit -
 * --------------------- */

void __mailslot_uninit (void)
{
    int i;

    if (!__mailslot_inited) return;

    /* Free DOS memory */
    __dpmi_free_dos_memory(__mailslot_sel);
    __mailslot_seg = __mailslot_sel = __mailslot_size = 0;

    /* Destroy all mailslots & free buffers. */
    for (i = 0; i < MAILSLOT_MAX; i++) {
	if (__mailslot[i] != NULL)
	    __mailslot_destroy (__mailslot[i]->handle);
    }

    __mailslot_inited = 0;
}

/* ---------------------
 * - __mailslot_create -
 * --------------------- */

/* Return handle on success, else -1. */

int __mailslot_create (char *name, int len)
{
    int seg, sel, paras;
    __dpmi_regs r;
    int hMailslot, i;
    char newname[MAXPATH];

    /* Initialise */
    if (!__mailslot_init ())
	return (-1);

    /* Parameter checks */
    if ((name == NULL)
	|| (len < MAILSLOT_MINSIZE) || (len > MAILSLOT_MAXSIZE)) {
	errno = EINVAL;
	return (-1);
    }
    /* Backward-slashify the mailslot name. */
    strcpy (newname, name);
    for (i = 0; i < strlen (newname); i++) {
	if (newname[i] == '/')
	    newname[i] = '\\';
    }

    /* Allocate buffer for mailslot receives. */
    paras = (len + 9 + 15) >> 4;    
    seg = __dpmi_allocate_dos_memory (paras, &sel);
    if (seg == -1)
	return (-1);

    /* Copy the mailslot name into the buffer. */
    movedata (_my_ds(), (int) newname,
	      __mailslot_sel, 0,
	      strlen (newname) + 1);

    /* Make the call */
    r.x.ax = 0x5F4D;
    r.x.bx = len;
    r.x.cx = len + 9;		/* 9 bytes bigger than largest message */
    r.x.ds = __mailslot_seg;	/* Mailslot name   */
    r.x.si = 0;
    r.x.es = seg;		/* Mailslot buffer */
    r.x.di = 0;

    __dpmi_int (0x21, &r);

    hMailslot = ((r.x.flags & CARRY) == CARRY) ? -1 : r.x.ax;

    if (hMailslot != -1) {
	/* Add the mailslot to the info structs */
	for (i = 0; i < MAILSLOT_MAX; i++) {
	    if (__mailslot[i] == NULL) {
		/* Create control structure */
		__mailslot[i] = malloc (sizeof (MAILSLOT));
		if (__mailslot[i] == NULL) {
		    /* TODO: How to destroy mailslot? */
		    hMailslot = -1;
		    break;
		    /*return(-1); */
		}
		/* Store mailslot details */
		__mailslot[i]->name = strdup (newname);
		__mailslot[i]->handle = hMailslot;
		__mailslot[i]->sel = sel;
		__mailslot[i]->seg = seg;
		__mailslot[i]->off = 0;
		__mailslot[i]->len = len + 9;	/* Buffer size */
		__mailslot[i]->maxlen = len;	/* Longest msg */
		break;
	    }
	}
    } else {
	errno = __doserr_to_errno (r.x.ax);
    }

    return (hMailslot);
}

/* ----------------------
 * - __mailslot_destroy -
 * ---------------------- */

int __mailslot_destroy (int hMailslot)
{
    int sel;
    __dpmi_regs r;
    int i;

    /* Initialise */
    if (!__mailslot_init ())
	return (-1);

    /* Parameter checking */
    if (hMailslot == -1) {
	errno = EINVAL;
	return (-1);
    }
    /* Make the call */
    bzero (&r, sizeof (r));
    r.x.ax = 0x5F4E;
    r.x.bx = hMailslot;

    __dpmi_int (0x21, &r);

    /* Success? If so, free the DOS memory alloc'd earlier */
    if ((r.x.flags & CARRY) == 0) {
	/* Find the selector from the info structs */
	for (sel = i = 0; i < MAILSLOT_MAX; i++) {
	    if (__mailslot[i]->handle == hMailslot) {
		sel = __mailslot[i]->sel;
		break;
	    }
	}

	if (sel != 0) {
	    /* Now free buffer memory */
	    __dpmi_free_dos_memory (sel);
	    free (__mailslot[i]);
	    __mailslot[i] = NULL;
	    return (0);
	} else {
	    /* TODO: Error code */
	    return (-1);
	}
    } else {
	/* Failed */
	errno = __doserr_to_errno (r.x.ax);
	return (-1);
    }
}

/* --------------------
 * - __mailslot_write -
 * -------------------- */

int __mailslot_write (char *name, char *msg, int len, int timeout)
{
    int headersize, maxlen, sendlen, total;
    __dpmi_regs r;
    char newname[MAXPATH];
    int i;

    /* Initialise */
    if (!__mailslot_init ())
	return (-1);

    /* Parameter checking */
    if ((len < 0) || (name == NULL) || (msg == NULL)) {
	errno = EINVAL;
	return (-1);
    }
    if (len == 0)
	return (0);

    /*  Backward-slashify the mailslot name. */
    strcpy (newname, name);
    for (i = 0; i < strlen (newname); i++) {
	if (newname[i] == '/')
	    newname[i] = '\\';
    }

    /* TODO: Use a separate buffer to store the message here? */
    
    /* Write loop - transfer the data in small chunks, until the write
     * fails. */
    total = 0;			       		/* Total written    */
    headersize = 4 + 4 + strlen (newname) + 1;	/* See header below */
    maxlen = ((__mailslot_size - headersize) > MAILSLOT_SENDSIZE)
      ? MAILSLOT_SENDSIZE : __mailslot_size - headersize;

    for (i = 0; i < len; i += maxlen) {
	/* Bytes to send */
	sendlen = len - i;
	if (sendlen > maxlen)
	    sendlen = maxlen;

	/* Copy the data into the buffer:
	 * 0     - DWORD - timeout
	 * 4     - DWORD - buffer location
	 * 8     - bytes - mailslot name, length x
	 * 8 + x - bytes - message
	 */

	_farpokel (__mailslot_sel, 0, timeout);			/* Timeout */
	_farpokew (__mailslot_sel, 4, 8 + strlen (newname) + 1);/* Buf off */
	_farpokew (__mailslot_sel, 6, __mailslot_seg);		/* Buf seg */
	movedata (_my_ds(), (int) newname,
		  __mailslot_sel, 8, strlen (newname) + 1);
	movedata (_my_ds(), (int) (msg + i),
		  __mailslot_sel, 8 + strlen (newname) + 1, sendlen);

	/* Set up the regs */
	bzero (&r, sizeof (r));
	r.x.ax = 0x5F52;
	r.x.bx = 2;			/* Second class message? */
	r.x.cx = sendlen;
	r.x.dx = 0;			/* Priority = low? */
	r.x.es = __mailslot_seg;	/* Mailslot struct (2 DWORDs above) */
	r.x.di = 0;
	r.x.ds = __mailslot_seg;	/* Mailslot name location */
	r.x.si = 8;

	/* Make the call */
	__dpmi_int (0x21, &r);

	/* Only convert the error if no bytes have been sent, otherwise the
	 * mailslot's buffer could be full. */
	if (((r.x.flags & 1) == 1) && (total == 0))
	    errno = __doserr_to_errno (r.x.ax);

	if ((r.x.flags & 1) == 0)
	    total += sendlen;
    }

    return ((total == 0) ? -1 : total);
}

/* -------------------
 * - __mailslot_read -
 * ------------------- */

/* TODO: Add range checking on len when writing into msg. */

int __mailslot_read (int hMailslot, char *msg, int len, int timeout)
{
    int total, ret;
    __dpmi_regs r;

    /* Initialise */
    if (!__mailslot_init ())
	return (-1);

    /* Parameter checking */
    if ((len < 0) || (hMailslot == -1) || (msg == NULL)) {
	errno = EINVAL;
	return (-1);
    }
    if (len == 0)
	return (0);

    /* Read loop - transfer the data in small chunks, until the read
     * fails. */
    total = 0;			       /* Total written    */

    for (ret = 1; ret > 0;) {
	/* Set up the regs */
	bzero (&r, sizeof (r));
	r.x.ax = 0x5F50;
	r.x.bx = hMailslot;
	r.x.dx = timeout >> 16;
	r.x.cx = timeout & 0xFFFF;
	r.x.es = __mailslot_seg;
	r.x.di = 0;

	/* Make the call */
	__dpmi_int (0x21, &r);

	ret = ((r.x.flags & 1) == 1) ? -1 : r.x.ax;

	if (ret > 0) {
	    movedata (__mailslot_sel, 0,
		      _my_ds(), (int) (msg + total), r.x.ax);
	    total += r.x.ax;
	}
	if (ret == -1) {
	    errno = __doserr_to_errno (r.x.ax);
	    return (-1);
	}
	/* Don't block if there's nothing more to read */
	if (r.x.cx == 0)
	    break;
    }

    /* 0 bytes read is not an error. */
    return (total);
}

/* -------------------
 * - __mailslot_peek -
 * ------------------- */

/* This works the __mailslot_read(), but does not remove data from the
 * mailslot buffer. Also, one cannot specify a timeout. */

int __mailslot_peek (int hMailslot, char *msg, int len)
{
    int ret;
    __dpmi_regs r;

    /* Initialise */
    if (!__mailslot_init ())
	return (-1);

    /* Parameter checking */
    if ((len < 0) || (hMailslot == -1) || (msg == NULL)) {
	errno = EINVAL;
	return (-1);
    }
    if (len == 0)
	return (0);

    /* Set up the regs */
    bzero (&r, sizeof (r));
    r.x.ax = 0x5F51;
    r.x.bx = hMailslot;
    r.x.es = __mailslot_seg;
    r.x.di = 0;

    /* Make the call */
    __dpmi_int (0x21, &r);

    ret = ((r.x.flags & 1) == 1) ? -1 : r.x.ax;
    if (ret > 0)
	movedata (__mailslot_sel, 0, _my_ds(), (int) msg, r.x.ax);
    if (ret == -1)
	errno = __doserr_to_errno (r.x.ax);

    return (ret);
}

/* -------------------
 * - __mailslot_info -
 * ------------------- */

int __mailslot_info (int hMailslot, MAILSLOT_INFO * mi)
{
    int ret;
    __dpmi_regs r;

    /* Parameter checking */
    if ((hMailslot == -1) || (mi == NULL)) {
	errno = EINVAL;
	return (-1);
    }
    /* Set up the regs */
    bzero (&r, sizeof (r));
    r.x.ax = 0x5F4F;
    r.x.bx = hMailslot;

    /* Make the call */
    __dpmi_int (0x21, &r);

    ret = ((r.x.flags & 1) == 1) ? -1 : 0;

    if (ret == -1) {
	errno = __doserr_to_errno (r.x.ax);
	return (ret);
    }
    /* Store the info */
    mi->maxlen = r.x.ax;
    mi->slotlen = r.x.bx;
    mi->nextlen = r.x.cx;
    mi->nextpriority = r.x.dx;
    mi->queued = r.x.si;

    return (ret);
}
