#
# Makefile for libsocket for DJGPP
#
# libsocket Copyright 1997, 1998 by Indrek Mandre
# libsocket Copyright 1997, 1998 by Richard Dawe
#

include ../../Makefile.cfg

CFLAGS += -I../../include

LOBJS =	i_ntoa.o i_addr.o i_lnaof.o i_mkaddr.o i_net.o			\
	i_netof.o gprtbynm.o gprtbyn2.o gprtent.o 			\
	gnetbynm.o gnetent.o gnetbyad.o gsrvbynm.o gsrvent.o		\
	shstent.o l_res.o r_data.o l_herrno.o ghstnmad.o		\
	gsrvbypt.o i_nsap.o herror.o					\
	r_debug.o r_mkqry.o r_qry.o r_init.o r_comp.o r_send.o		\
	bresvpt.o rcmd.o rexec.o rusrpass.o				\
	i_pton.o i_ntop.o

all:	$(LOBJS)

.PHONY:	all dep clean distclean

dep:
	$(CC) $(CFLAGS) -M *.c > depend.dep

clean:
	rm -f $(LOBJS)

distclean: clean
	rm -f *.\$$\$$\$$
	rm -f *~

$(OBJS) $(LOBJS):
include depend.dep
