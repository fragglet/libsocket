#
# Makefile for sock.vxd test programs
# Written by Richard Dawe <richdawe@bigfoot.com>
#

include ../../../Makefile.cfg

CFLAGS  += -I../../../include
LDFLAGS += -L../../../lib
LIBS    += -lsocket

TARGETS     = create destroy create2
TARGETS_EXE = $(patsubst %,%.exe,$(TARGETS))

default:	$(TARGETS)

%.o:	%.c
	$(CC) $(CFLAGS) -c $<

create:	create.o
	$(CC) $(LDFLAGS) -o $@ $< $(LIBS)

create2:	create2.o
	$(CC) $(LDFLAGS) -o $@ $< $(LIBS)

destroy:	destroy.o
	$(CC) $(LDFLAGS) -o $@ $< $(LIBS)

clean:
	rm -f *.o

distclean:
	rm -f $(TARGETS) $(TARGETS_EXE)
	rm -f *.\$$\$$\$$
	rm -f *~