#
# Makefile for libsocket general tests
#
# libsocket Copyright 1997, 1998 by Indrek Mandre
# libsocket Copyright 1997-1999 by Richard Dawe
#

include ../../Makefile.cfg

CFLAGS  += -I../../include
LDFLAGS += -L../../lib
LIBS    += -lsocket

TARGETS     = iovec
TARGETS_EXE = iovec.exe

all:	$(TARGETS)

iovec:	iovec.o
	$(CC) $(LDFLAGS) -o $@ $< $(LIBS)

%.o:	%.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o

distclean:
	rm -f $(TARGETS) $(TARGETS_EXE)
	rm -f *.\$$\$$\$$
	rm -f *~