#
# Makefile for Win9x functions for libsocket
# Written by Richard Dawe, made consistent by Indrek Mandre and modified a bit
# more by Richard Dawe
#
# libsocket Copyright 1997, 1998 by Indrek Mandre
# libsocket Copyright 1997-1999 by Richard Dawe
#

include ../../Makefile.cfg

CFLAGS += -I../../include

# Objects to build
OBJS =	w9x_tcp.o w9x_ras.o w9x_dhcp.o w9x_addr.o

all:	$(OBJS) regdos

.PHONY:	all clean distclean dep regdos

clean:
	$(MAKE) -C regdos clean
	rm -f $(OBJS)	

distclean:	clean
	$(MAKE) -C regdos distclean
	rm -f *.\$$\$$\$$
	rm -f *~

dep:
	$(CC) $(CFLAGS) -M *.c > depend.dep
	$(MAKE) -C regdos dep

regdos:
	$(MAKE) -C regdos all

$(OBJS):
include depend.dep
