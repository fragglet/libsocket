#
# Makefile for libsocket for DJGPP
#
# libsocket Copyright 1997, 1998 by Indrek Mandre
# libsocket Copyright 1997, 1998 by Richard Dawe
#
# This was written by Indrek Mandre and modified heavily by Richard Dawe.

include ../Makefile.cfg

CFLAGS += -I../include -I./include

LOBJS =	init_net.o fsext.o socket.o bind.o listen.o accept.o recv.o	\
	send.o connect.o gsocknm.o gpeernm.o close.o sockopt.o 		\
	shutdown.o ioctl.o select.o fcntl.o newerror.o

all: library

# comp_* are the different components of the library
library: 	$(LOBJS) comp_resolve comp_registry comp_config		\
		comp_wsock comp_pktdrv
	rm -rf ../lib/libsocket.a
	$(AR) rcs ../lib/libsocket.a	$(LOBJS) resolve/*.o 		\
					registry/*.o config/*.o		\
					wsock/*.o

comp_resolve:	resolve
	@cd resolve;make all;cd ..

comp_registry:	registry
	@cd registry;make all;cd ..

comp_config:	config
	@cd config;make all;cd ..

comp_wsock:	wsock
	@cd wsock;make all;cd ..

comp_pktdrv:	pktdrv
	@cd pktdrv;make all;cd ..

dep:
	$(CC) $(CFLAGS) -MM *.c > depend.dep
	@cd resolve;make dep;cd ..
	@cd registry;make dep;cd ..
	@cd config;make dep;cd ..
	@cd wsock;make dep;cd ..
	@cd pktdrv;make dep;cd ..

clean:
	rm -f $(LOBJS)
	@cd resolve;make clean;cd ..
	@cd registry;make clean;cd ..
	@cd config;make clean;cd ..
	@cd wsock;make clean;cd ..
	@cd pktdrv;make clean;cd ..

distclean: clean	
	rm -f ../lib/libsocket.a

$(OBJS) $(LOBJS):
include depend.dep
