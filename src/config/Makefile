#
# Makefile for configuration functions for libsocket
# Written by Richard Dawe, made consistent by Indrek Mandre and modified a bit
# more by Richard Dawe
#
# libsocket Copyright 1997, 1998 by Indrek Mandre
# libsocket Copyright 1997, 1998 by Richard Dawe
#

include ../../Makefile.cfg

CFLAGS += -I../../include -I../include -I.

# Objects to build
OBJS =	dnsaddr.o domname.o hostname.o config.o mstcp.o nettrans.o	\
	ras.o vdhcp.o ini.o

all:	$(OBJS)

clean:
	rm -f $(OBJS)

dep:
	$(CC) $(CFLAGS) -MM *.c > depend.dep

$(OBJS):
include depend.dep
