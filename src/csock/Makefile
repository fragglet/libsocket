#
# Makefile for Coda's SOCK.VXD functions for libsocket
# Written by Richard Dawe <richdawe@bigfoot.com>
#
# libsocket Copyright (C) 1997, 1998 by Indrek Mandre
# libsocket Copyright (C) 1997-2000 by Richard Dawe
#

include ../../Makefile.cfg

CFLAGS += -I../../include

# Objects to build
OBJS =	c_initnt.o c_socket.o c_close.o c_connct.o c_send.o c_errors.o	\
	c_select.o c_recv.o c_listen.o c_bind.o c_accept.o c_gprnm.o	\
	c_gscknm.o c_ioctl.o c_fcntl.o c_nblkio.o c_shtdwn.o c_sckopt.o	\
	c_sockpr.o

all:	$(OBJS)

.PHONY:	all clean distclean dep

clean:
	rm -f $(OBJS)

distclean:	clean
	rm -f *.\$$\$$\$$
	rm -f *~

dep:
	$(CC) $(CFLAGS) -M *.c > depend.dep

$(OBJS):
include depend.dep
