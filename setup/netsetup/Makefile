# Makefile for Netsetup
# Copyright 1997 by Richard Dawe

include ../../Makefile.cfg

LDFLAGS = -L../../lib
LIBS =
NETLIBS = $(LIBS) -lsocket

CFLAGS += -I../../include

TARGETS = netsetup

NETSETUP = netsetup.o

all: $(TARGETS)

netsetup:	$(NETSETUP)
	$(CC) -o netsetup $(CFLAGS) $(NETSETUP) $(LDFLAGS) $(LIBS)
	$(STRIP) netsetup
	coff2exe netsetup

dep:
	$(CC) $(CFLAGS) -MM *.c > depend.dep

clean:
	rm -f $(NETSETUP)

distclean: clean
	rm -f $(TARGETS) *.exe resolv.conf hosts.eg host.conf networks.eg

$(OBJS) $(LOBJS):
include depend.dep
