#
# Makefile for Unix domain socket functions for libsocket
# Written by Richard Dawe
#
# libsocket Copyright 1997, 1998 by Indrek Mandre
# libsocket Copyright 1997-1999 by Richard Dawe
#

include ../../Makefile.cfg

CFLAGS += -I../../include

# Objects to build
OBJS =	mailslot.o mail_fse.o u_initnt.o u_socket.o u_bind.o u_connct.o	\
	u_accept.o u_send.o u_recv.o u_close.o u_gprnm.o u_gscknm.o	\
	u_fcntl.o u_ioctl.o u_shtdwn.o u_listen.o u_sockpr.o u_sckopt.o	\
	u_select.o

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
