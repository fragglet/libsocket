
#
# Makefile for libsocket for DJGPP
#
# libsocket Copyright 1997, 1998 by Indrek Mandre
# libsocket Copyright 1997, 1998 by Richard Dawe
#

include ../../Makefile.cfg

CFLAGS += -I../../include -I../include -I.

LOBJS =	i_ntoa.o i_addr.o i_lnaof.o i_mkadr.o i_net.o		\
	i_netof.o gproto.o gprtent.o gprtname.o			\
	gnetbynm.o gnetent.o gnetbyad.o gsrvbynm.o gsrvent.o	\
	shostent.o l_res.o r_data.o l_herrno.o ghstnmad.o	\
	gsrvbypt.o i_nsap.o bresvprt.o herror.o writev.o	\
	r_debug.o r_mkqry.o r_qry.o r_init.o r_comp.o		\
	r_send.o

all: library

library: $(LOBJS)

dep:
	$(CC) $(CFLAGS) -MM *.c > depend.dep

clean:
	rm -f $(LOBJS)

distclean: clean

$(OBJS) $(LOBJS):
include depend.dep
