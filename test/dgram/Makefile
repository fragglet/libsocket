#
# Makefile for datagram tests for libsocket
# Written by Richard Dawe <richdawe@bigfoot.com>
#

include ../../Makefile.cfg

CFLAGS  += -I../../include
LDFLAGS += -L../../lib
LIBS    += -lsocket

TARGETS     = d_sockpr
TARGETS_EXE = d_sockpr.exe

all:	$(TARGETS)

d_sockpr:	d_sockpr.o
	$(CC) $(LDFLAGS) -o $@ $< $(LIBS)

%.o:	%.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o

distclean:
	rm -f $(TARGETS) $(TARGETS_EXE)
	rm -f *.\$$\$$\$$
	rm -f *~