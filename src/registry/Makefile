#
# Makefile for registry functions for libsocket
# Written by Richard Dawe, made consistent by Indrek Mandre
#
# libsocket Copyright 1997, 1998 by Indrek Mandre
# libsocket Copyright 1997, 1998 by Richard Dawe
# Registry functions Copyright (C) 1997-1998 by the RegDos Group
#

include ../../Makefile.cfg

CFLAGS += -I../../include -I../include -I.

# Objects to build
OBJS = dosxdj.o lstack.o regdosz.o

all:	$(OBJS)

clean:
	rm -f $(OBJS)

dep:
# RD: The -DPF_MSDOS_32 #defines PF_MSDOS_32 so that dependency generation
# works with _getvxdp.c and _regcall.c.
	$(CC) $(CFLAGS) -DPF_MSDOS_32 -MM *.c > depend.dep

$(OBJS):
include depend.dep
