#
# Makefile for registry functions for libsocket
# Written by Richard Dawe, made consistent by Indrek Mandre
#
# libsocket Copyright 1997, 1998 by Indrek Mandre
# libsocket Copyright 1997-1999 by Richard Dawe
# Registry functions Copyright (C) 1997-1998 by the RegDos Group
#

include ../../../Makefile.cfg

CFLAGS += -I../../../include

# Objects to build
OBJS = dosxdj.o lstack.o regdosz.o

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
