#
# Makefile for Netsetup
# Copyright (C) 1997-2000 by Richard Dawe
#

include ../../Makefile.cfg

CFLAGS += -I../../include
LDFLAGS = -L../../lib
LIBS    = -lsocket

TARGETS = netsetup.exe
DOCS    = netsetup.inf netsetup.htm

.PHONY:	all info html

all: $(TARGETS) $(DOCS)

%.exe:	%.c
	$(CC) -o $@ $(CFLAGS) $< $(LDFLAGS) $(LIBS)
ifeq ($(LSCK_STRIPALL),y)
	strip $@
endif

%.htm:	%.txi
	-mkdir ../../doc
	$(MAKEINFO) --verbose --html $< -o $@
	cp -u $@ ../../doc

%.inf:	%.txi
	-mkdir ../../info
	$(MAKEINFO) --verbose --no-split $< -o $@
	cp -u $@ ../../info

clean:
	rm -f *.o

distclean:	clean
	rm -f *.\$$\$$\$$
	rm -f *~
	rm -f $(DOCS)
	rm -f $(TARGETS)
