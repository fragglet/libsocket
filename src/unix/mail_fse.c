/*
 * mail_fse.c
 * Copyright 1999, 2000 by Richard Dawe
 * 
 * Perform some behind-the-scenes trickery with DJGPP's File System Extensions
 * so that file handles can be used for (one-way) file operations.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dir.h>
#include <fcntl.h>

#include <sys/fsext.h>

#include "mailslot.h"
#include "mail_int.h"

typedef struct _MAILSLOT_FSEXT {
	int fd;
	int handle;
	char *name;
	
	/* Flag for blocking I/O (default). */
	unsigned int blocking:1;

	/* Flag to allow peeking of read buffer. This is helpful for Unix
	 * domain sockets. */
	unsigned int peeking:1;
} MAILSLOT_FSEXT;

/* We need this to keep track of which file descriptors are mailslots. */
static MAILSLOT_FSEXT *__mailslot_fsext[MAILSLOT_MAX];

static int __mailslot_fsext_inited = 0;

int __mailslot_install_handler (void);
void __mailslot_uninstall_handler (void);
int __mailslot_handler (__FSEXT_Fnumber n, int *rv, va_list args);

extern int __mailslot_init (void);

/* ------------------------------
 * - __mailslot_install_handler -
 * ------------------------------ */

int __mailslot_install_handler (void)
{
	int i;

	if (__mailslot_fsext_inited) return (__mailslot_fsext_inited);

	/* Initialise mailslots first */
	if (!__mailslot_init()) return (0); /* Failed */

	for (i = 0; i < MAILSLOT_MAX; i++) { __mailslot_fsext[i] = NULL; }
	__FSEXT_add_open_handler (__mailslot_handler);
	atexit (__mailslot_uninstall_handler);
	__mailslot_fsext_inited = 1;

	return (__mailslot_fsext_inited);
}

/* --------------------------------
 * - __mailslot_uninstall_handler -
 * -------------------------------- */

void __mailslot_uninstall_handler (void)
{
	int i;
	
	/* Close all mailslots & free memory. */
	for (i = 0; i < MAILSLOT_MAX; i++) {
		if (__mailslot_fsext[i] != NULL)
			close(__mailslot_fsext[i]->fd);
	}

	/* TODO: How to uninstall FSEXT handler? */
}

/* ------------------------------
 * - __mailslot_fsext_firstfree -
 * ------------------------------ */

int __mailslot_fsext_firstfree (void)
{
	int i, msd;

	for (i = 0, msd = -1; i < MAILSLOT_MAX; i++) {
		if (__mailslot_fsext[i] == NULL) {
			msd = i;
			break;
		}
	}

	return (msd);
}

/* ----------------------
 * - __mailslot_handler -
 * ---------------------- */

int __mailslot_handler (__FSEXT_Fnumber n, int *rv, va_list args)
{
	int emul = 0; /* Emulated the call? */
	char *file = NULL;
	int mode = 0, perm = 0;
	int msd, hMailslot;
	int fd = -1, cmd;
	int fcntl_comm;
	int *ioctl_comm;
	MAILSLOT_FSEXT *me = NULL;
	MAILSLOT_INFO mi;
	char *p = NULL, *q = NULL, *buf = NULL;
	char newname[MAXPATH];
	int len;
	int ret, i;

	switch (n) {
	/* --- __FSEXT_creat, __FSEXT_open --- */
	case __FSEXT_creat:
	case __FSEXT_open:
		/* Get the parameters */
		file = strdup ((char *) va_arg (args, const char *));
		if (file == NULL) break;

		if (n == __FSEXT_creat) {
			perm = va_arg (args, int);

			/* Fake the mode flag based on the permissions. */
			mode = ((perm & S_IWUSR) == S_IWUSR)
				? O_WRONLY : O_RDONLY;
		} else {
			mode = va_arg (args, int);
			perm = va_arg (args, int);
		}

		/* Forward-slashify file */	
		for (i = 0; i < strlen (file); i++) {
			if (file[i] == '\\') file[i] = '/';
		}

		/* Refers to a mailslot? */
		if (strstr (file, MAILSLOT_DEV) != file) {
			free (file);
			break;
		}
		
		emul = 1; /* Emulated */

		/* What kind of a mailslot in needed? */
		/* TODO: Handle other modes too, i.e. text/binary. */

		/* NB: We have to use file descriptors in binary mode. write()
		 * doesn't call the FSEXT unless O_BINARY is set. Then write()
		 * calls _write(), which in turn calls the FSEXT. This problem
		 * is fixed in DJGPP 2.02. Unfortunately, setmode() here has no
		 * effect since the open() function will set the mode after
		 * the FSEXT call has completed. */

		if ((mode & O_RDWR) == O_RDWR) {
			/* Only read-only or write-only allowed -
			 * uni-directional */			
			errno = EPERM; /* TODO: Error code? */
			*rv   = -1;
			break;
		}

		*rv = -1; /* Fail by default */
		
		switch(mode & (O_WRONLY | O_RDONLY)) {
		/* Write-only */
		default:
		case O_WRONLY:
			*rv = -1; /* Fail by default */

			/* Store the mailslot details for write calls */
			msd = __mailslot_fsext_firstfree();
			p = file + strlen (MAILSLOT_DEV);

			if (msd == -1) {
				/* Should be free sometime soon */
				errno = EAGAIN;
				break;
			}
			
			/* Deduce the actual mailslot path */
			bzero (newname, sizeof (newname));

			if (strstr (p, MAILSLOT_LOCAL) == p) {
				/* Local mailslot */
				p += strlen (MAILSLOT_LOCAL);
				sprintf (newname, "/mailslot/%s", p);
			} else if (strstr(p, MAILSLOT_BROADCAST) == p) {
				/* Broadcast mailslot */
				p += strlen (MAILSLOT_BROADCAST);
				sprintf (newname, "//*/mailslot/%s", p);
			} else {
				/* Remote mailslot */
				sprintf (newname, "//%s", p);
				q = strchr (newname + 2, '/');
				if (q != NULL) *q = '\0';

				p = strchr (p, '/');
				if (p != NULL) {
					strcat (newname, "/mailslot");
					strcat (newname, p);
				}
			}

			/* Now set-up the mailslot structure. */
			fd = __FSEXT_alloc_fd( __mailslot_handler);
			if (fd == -1) break;
			
			me = __mailslot_fsext[msd]
				= (MAILSLOT_FSEXT *) malloc(sizeof(*me));
			
			if (me == NULL) {
				__FSEXT_set_function(fd, NULL);
				_close(fd);
				errno = ENOMEM;
				break;
			}
					
			me->fd       = fd;
			me->handle   = -1;
			me->name     = strdup(newname);
			me->blocking = 1;
			me->peeking  = 0;
			__FSEXT_set_data(fd, (void *) me);
			
			*rv = me->fd;
			break;

		/* Read-only */
		case O_RDONLY:			
			/* Create a mailslot */
			p = file + strlen (MAILSLOT_DEV);

			/* Deduce the actual mailslot path */
			bzero (newname, sizeof (newname));

			if (strstr (p, MAILSLOT_LOCAL) == p) {
				/* Local mailslot */
				p += strlen (MAILSLOT_LOCAL);
				sprintf (newname, "/mailslot/%s", p);
			} else {
				/* Baaad filename */
				errno = EINVAL;
				break;
			}

			hMailslot = __mailslot_create(newname,
						      MAILSLOT_MAXSIZE);

			/* Pass through error. */
			if (hMailslot == -1) break;
			
			msd = __mailslot_fsext_firstfree ();

			if (msd == -1) {
				/* Should be free sometime soon */
				errno = EAGAIN;
				break;
			}

			/* Now set up the mailslot structure. */
			fd = __FSEXT_alloc_fd(__mailslot_handler);
			if (fd == -1) break;
			
			me = __mailslot_fsext[msd]
				= (MAILSLOT_FSEXT *) malloc(sizeof(*me));

			if (me == NULL) {
				__FSEXT_set_function(fd, NULL);
				_close(fd);
				errno = ENOMEM;
				break;
			}
						
			me->fd       = fd;
			me->handle   = hMailslot;
			me->name     = NULL;
			me->blocking = 1;
			me->peeking  = 0;
			__FSEXT_set_data(fd, (void *) me);
			
			*rv = fd;
			break;
		}
		
		free (file);
		break;

	/* --- __FSEXT_close --- */
	case __FSEXT_close:		
		*rv  = -1;	/* Fail by default    */
		emul = 1;	/* Emulate by default */

		/* Find the data structure */
		fd = va_arg (args, int);
		
		me = __FSEXT_get_data(fd);
		if (me == NULL) {
			errno = EBADF;
			break;
		}

		__FSEXT_set_data(fd, NULL);
		__FSEXT_set_function(fd, NULL);
		_close (fd);

		if (me->handle != -1) __mailslot_destroy(me->handle);
		if (me->name != NULL) free(me->name);

		/* Find the data structure */
		for (i = 0, msd = -1; i < MAILSLOT_MAX; i++) {
			if (__mailslot_fsext[i]->fd == fd) {
				msd = i;
				break;
			}
		}

		if (msd != -1) {
			free (__mailslot_fsext[msd]);
			__mailslot_fsext[msd] = NULL;
		}
		
		*rv  = 0;		
		break;

	/* --- __FSEXT_write --- */
	case __FSEXT_write:
		*rv  = -1;	/* Fail by default    */
		emul = 1;	/* Emulate by default */

		fd  = va_arg (args, int);
		buf = (char *) va_arg (args, void *);
		len = (int) va_arg (args, size_t);

		me = (MAILSLOT_FSEXT *) __FSEXT_get_data(fd);
		if (me == NULL) {
			errno = EBADF;
			break;
		}

		if (me->name == NULL) {
			errno = EPERM;
			break;
		}
		
		/* Bad params */
		if ((buf == NULL) || (len <= 0)) {
			errno = EINVAL;
			break;
		}
		
		/* TODO: Timeout? */
		if (me->blocking) {
			/* Loop until we have written some data */
			for (ret = -1; ret <= 0;) {
				ret = __mailslot_write(me->name,
						       buf, len, 0);
			}
			*rv = ret;
		} else {
			*rv = __mailslot_write (me->name, buf, len, 0);

			/* TODO: LAN Manager may return this error, via DJGPP's
			 * __doserr_to_errno() routine. If it does not then
			 * some error code conversion code will be necessary.
			 * This will be equally true of the read code below. */
			/*if (*rv == -1) errno = EAGAIN; */
		}

		break;

	/* --- __FSEXT_read --- */
	case __FSEXT_read:
		*rv  = -1;	/* Fail by default    */
		emul = 1;	/* Emulate by default */

		fd  = va_arg (args, int);
		buf = (char *) va_arg (args, void *);
		len = (int) va_arg (args, size_t);

		me = (MAILSLOT_FSEXT *) __FSEXT_get_data(fd);
		if (me == NULL) {
			errno = EBADF;
			break;
		}

		if (me->handle == -1) {
			errno = EPERM;
			break;
		}
		
		/* Bad params */
		if ((buf == NULL) || (len <= 0)) {
			errno = EINVAL;
			break;
		}
		
		/* TODO: Timeout? */
		if (me->blocking) {
			/* Loop until we have some data */
			for (ret = -1; ret <= 0; ) {
				if (me->peeking)
					ret = __mailslot_peek(me->handle,
							      buf, len);
				else
					ret = __mailslot_read(me->handle,
							      buf, len, 0);
			}
			*rv = ret;

			/* TODO: See comment above in writing code. */
		} else {
			if (me->peeking)
				*rv = __mailslot_peek(me->handle, buf, len);
			else
				*rv = __mailslot_read(me->handle, buf, len, 0);
		}

		break;

	/* --- __FSEXT_ready --- */
	case __FSEXT_ready:
		*rv  = 0;	/* Not ready by default */
		emul = 1;	/* Emulate by default   */

		me = (MAILSLOT_FSEXT *) __FSEXT_get_data(fd);
		if (me == NULL) {
			errno = EBADF;
			break;
		}

		/* Write-only mailslots always ready for writing. */
		if (me->name != NULL) *rv |= __FSEXT_ready_write;

		/* Read-only mailslots need checking. */
		if (me->handle != -1) {
			if (__mailslot_info(me->handle, &mi) == 0) {
				if (mi.queued > 0) *rv |= __FSEXT_ready_read;
			}
		}
		
		/* TODO: select() on error conditions? */
		break;

	/* --- __FSEXT_fcntl --- */
	case __FSEXT_fcntl:
		*rv  = -1;	/* Fail by default    */
		emul = 1;	/* Emulate by default */		

		fd         = va_arg (args, int);
		cmd        = va_arg (args, int);
		fcntl_comm = va_arg (args, int);

		me = (MAILSLOT_FSEXT *) __FSEXT_get_data(fd);
		if (me == NULL) {
			errno = EBADF;
			break;
		}

		/* TODO: This should also support other common flags! */
		/* Only support getting/setting flags */
		if ((cmd != F_GETFL) && (cmd != F_SETFL)) {
			errno = EINVAL;
			break;
		}
		
		/* Only support non-blocking flag */		
		if (cmd == F_SETFL) {
			/* If O_NONBLOCK set, clear blocking flag. Else,
			 * block. */
			me->blocking
				= !((fcntl_comm & O_NONBLOCK) == O_NONBLOCK);
		}

		/* TODO: Need to return "reasonable" options as given by open()
		 * call. */
		if (cmd == F_GETFL) {
			*rv = me->blocking ? 0 : O_NONBLOCK;
		}

		break;

	/* --- __FSEXT_ioctl --- */
	case __FSEXT_ioctl:
		*rv  = -1;	/* Fail by default    */
		emul = 1;	/* Emulate by default */

		fd         = va_arg (args, int);
		cmd        = va_arg (args, int);
		ioctl_comm = va_arg (args, int *);

		me = (MAILSLOT_FSEXT *) __FSEXT_get_data(fd);
		if (me == NULL) {
			errno = EBADF;
			break;
		}		

		/* Which ioctl is it? */
		switch(cmd) {
		/* Non-blocking toggle */
		case FIONBIO:
			me->blocking = (*ioctl_comm == 0);
			*rv          = 0;
			break;

		/* Maximum atomic read */
		case FIONREAD:
			*rv = __mailslot_info(me->handle, &mi);
			if (*rv == 0) *ioctl_comm = mi.nextlen;
			break;

		/* For read-only mailslot, toggle peeking/reading. */
		case MIOPEEK:
			if (me->handle == -1) {
				errno = EPERM;
				break;
			}
			
			me->peeking = !(*ioctl_comm == 0);
			*rv         = 0;
			break;
			
		default:
			errno = EINVAL;
			break;
		}
		
		break;

	/* Not emulated => pass it on. */
	default:
		break;
	}

	return (emul);
}
