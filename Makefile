#
# Makefile for libsocket 0.7.3's source distribution
#
# Copyright 1997, 1998 by Indrek Mandre
# Copyright 1997, 1998 by Richard Dawe
#

include Makefile.cfg

default:
	@echo
	@echo "Welcome to libsocket 0.7.3 source distribution!"
	@echo
	@echo "To make libsocket type:"
	@echo
	@echo "	make sfn		- Set up library for short filename use"
	@echo "	make lfn		- Set up library for long filename use"
	@echo "	make debug		- Set Makefile.cfg for a debug library build"
	@echo "	make nodebug		- Set Makefile.cfg for a release lib. build"
	@echo
	@echo "	make all		- Make library and test programs"
	@echo "	make library		- Make only library"
	@echo "	make demos		- Make test programs"
	@echo "	make setup		- Make setup script and run it"
	@echo "	make netsetup		- Make setup script and run it"
	@echo "	make htmldocs		- Make HTML versions of man pages"
	@echo
	@echo "	make install		- Install library and header files"
	@echo "	make installman		- Install man pages"
	@echo "	make uninstall		- Uninstall library and header files"
	@echo
	@echo "	make clean		- Remove .o files"
	@echo "	make distclean		- Remove ready binaries and .o files"
	@echo "	make dep		- Make dependences"
	@echo

# Inform make of phony targets
.PHONY:	library demos netsetup clean blankdep dep distclean		\
	install installman htmldocs debug nodebug setup			\
	sfn lfn

# Short filename and long filename usage
sfn:
	@mv lib/libsoc~1.a lib/libsocke.a

lfn:
	@mv lib/libsocke.a lib/libsocket.a

all: library demos

library: src
	-mkdir lib
	@cd src
	@make all
	@cd ..

demos: demo
	@cd demo
	@make all
	@echo "Have you run setup?"

# Readying for Win32 setup program
setup:	netsetup

netsetup:
	@cd setup/netsetup
	@make all
	@netsetup

clean:
	@cd src; make clean; cd ..
	@cd setup/netsetup; make clean; cd ../..
	@cd demo; make clean; cd ..

blankdep:
# Create blank depend.dep files to avoid errors
	@echo > demo/depend.dep
	@echo > setup/netsetup/depend.dep
	@echo > src/depend.dep
	@echo > src/resolve/depend.dep
	@echo > src/registry/depend.dep
	@echo > src/config/depend.dep
	@echo > src/wsock/depend.dep
	@echo > src/pktdrv/depend.dep
	@echo Created blank dependency files

dep:	blankdep
# Now carry on as usual
	@cd src;make dep;cd ..
	@cd demo;make dep;cd ..
	@cd setup/netsetup;make dep;cd ../..

# Blank all the dependencies too
distclean:	blankdep
	@cd src;make distclean;cd ..
	@cd setup/netsetup;make distclean;cd ../..
	@cd demo;make distclean

install: library
	cp lib/*.a $(DJDIR)/lib
	ginstall -d $(DJDIR)/include/arpa
	ginstall -d $(DJDIR)/include/lsck
	ginstall -d $(DJDIR)/include/sys
	cp include/*.h $(DJDIR)/include
	cp include/arpa/*.h $(DJDIR)/include/arpa
	cp include/sys/*.h $(DJDIR)/include/sys
	cp include/lsck/*.h $(DJDIR)/include/lsck

installman:
	ginstall -d $(MAN_PLACE)
	cp -r man $(MAN_PLACE)

uninstall:
	-rm $(DJDIR)/lib/libsocket.a
	-rm $(DJDIR)/lib/libsocke.a
	-rm $(DJDIR)/include/netdb.h
	-rm $(DJDIR)/include/winsock.h
	-rm $(DJDIR)/include/ws.h
	-rm $(DJDIR)/include/arpa/inet.h
	-rm $(DJDIR)/include/sys/socket.h
	-rm $(DJDIR)/include/lsck/*.h
	-rmdir $(DJDIR)/include/arpa
	-rmdir $(DJDIR)/include/lsck

htmldocs:
	perl -w misc/man2html/man2html.pl -htm -1 -idx:lsck_man

# Debugging targets - modify Makefile.cfg
nodebug:
	@cp Make-cfg.in Makefile.cfg

debug:	nodebug
	@echo "CFLAGS += -g -DDEBUG" >> Makefile.cfg
