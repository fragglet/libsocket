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
OBJS = 

all:	$(OBJS)

clean:
	rm -f $(OBJS)

dep:
	$(CC) $(CFLAGS) -MM *.c > depend.dep

$(OBJS):
include depend.dep
