#
# Makefile for POSIX functions for libsocket
# Written by Richard Dawe, made consistent by Indrek Mandre and modified a bit
# more by Richard Dawe
#
# libsocket Copyright 1997, 1998 by Indrek Mandre
# libsocket Copyright 1997, 1998 by Richard Dawe
#

include ../../Makefile.cfg

CFLAGS += -I../../include

# Objects to build
OBJS =	genvsec.o

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
