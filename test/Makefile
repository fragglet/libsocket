#
# Makefile for libsocket tests
#
# libsocket Copyright 1997, 1998 by Indrek Mandre
# libsocket Copyright 1997-1999 by Richard Dawe
#

include ../Makefile.cfg

CFLAGS  += -I../include
LDFLAGS += -L../lib
LIBS    += -lsocket

TARGETS     = create
TARGETS_EXE = create.exe

# Phony targets
.PHONY:	all dgram stream lowlevel general

default:	all

all:	$(TARGETS)
	$(MAKE) -C dgram all
	$(MAKE) -C stream all
	$(MAKE) -C lowlevel all
	$(MAKE) -C general all

create:	create.o
	$(CC) $(LDFLAGS) -o $@ $< $(LIBS)

%.o:	%.c
	$(CC) $(CFLAGS) -c $<

dep:
	$(CC) $(CFLAGS) -M *.c > depend.dep

clean:
	$(MAKE) -C dgram clean
	$(MAKE) -C stream clean
	$(MAKE) -C lowlevel clean
	$(MAKE) -C general clean
	rm -f *.o

distclean:	clean
	$(MAKE) -C dgram distclean
	$(MAKE) -C stream distclean
	$(MAKE) -C lowlevel distclean
	$(MAKE) -C general distclean
	rm -f $(TARGETS) $(TARGETS_EXE)
	rm -f *.\$$\$$\$$
	rm -f *~
