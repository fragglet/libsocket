#
# Makefile for WSOCK.VXD functions for libsocket
# Written by Richard Dawe
# more by Richard Dawe
#
# libsocket Copyright 1997, 1998 by Indrek Mandre
# libsocket Copyright 1997, 1998 by Richard Dawe
#

include ../../Makefile.cfg

CFLAGS += -I../../include -I../include -I.

# Objects to build
OBJS =	vxd.o vxdldr.o callvxd.o w_socket.o w_bind.o w_accept.o		\
	w_listen.o w_send.o w_recv.o w_connct.o w_gprnm.o w_gscknm.o	\
	w_close.o w_sckopt.o w_shtdwn.o w_ioctl.o w_select.o w_errors.o	\
	w_initnt.o

all:	$(OBJS)

clean:
	rm -f $(OBJS)

dep:
	$(CC) $(CFLAGS) -MM *.c > depend.dep

$(OBJS):
include depend.dep
