#
# Makefile for libsocket for DJGPP
#
# libsocket (C) Copyright 1997, 1998 by Indrek Mandre
# libsocket (C) Copyright 1997-2000 by Richard Dawe
#
# This was written by Indrek Mandre and modified heavily by Richard Dawe.
#

include ../Makefile.cfg

CFLAGS += -I../include

MKDOC = ../tools/mkdoc/mkdoc

LOBJS =	initnt.o fsext.o socket.o sockpr.o bind.o listen.o accept.o	\
	recv.o	send.o connct.o gscknm.o gprnm.o close.o sckopt.o 	\
	shtdwn.o ioctl.o select.o fcntl.o newerror.o			\
	dnsaddr.o ini.o hstnm.o config.o addr.o if.o			\
	sockmark.o uname.o windows.o readv.o writev.o

DOCS =	lsck.inf lsck.htm						\
	../bugs.txt ../credits.txt ../changes.txt ../install.txt	\
	../config.txt ../redist/config.txt

default:	library

all: library doc

.PHONY:	all library dep clean distclean					\
	oldlibc inet netgroup posix wsock csock win9x win3x unix	\
	doc

library: 	$(LOBJS) oldlibc inet netgroup posix			\
		wsock csock win9x win3x unix
	-mkdir ../lib
	rm -rf ../lib/libsocket.a
	$(AR) rcs ../lib/libsocket.a	$(LOBJS)			\
					oldlibc/*.o			\
					inet/*.o netgroup/*.o posix/*.o	\
					wsock/*.o csock/*.o		\
					win9x/*.o win9x/regdos/*.o	\
					win3x/*.o unix/*.o
	$(RANLIB) ../lib/libsocket.a

doc:	$(DOCS)

oldlibc inet netgroup posix wsock csock win9x win3x unix:
	$(MAKE) -C $@ all

dep:
	$(CC) $(CFLAGS) -M *.c > depend.dep
# Do dependencies for the other modules
	$(MAKE) -C oldlibc	dep
	$(MAKE) -C inet		dep
	$(MAKE) -C netgroup	dep
	$(MAKE) -C posix	dep
	$(MAKE) -C wsock	dep
	$(MAKE) -C csock	dep
	$(MAKE) -C win9x	dep
	$(MAKE) -C win3x	dep
	$(MAKE) -C unix		dep

clean:
	rm -f $(LOBJS)
	$(MAKE) -C oldlibc	clean
	$(MAKE) -C inet		clean
	$(MAKE) -C netgroup	clean
	$(MAKE) -C posix	clean
	$(MAKE) -C wsock	clean
	$(MAKE) -C csock	clean
	$(MAKE) -C win9x	clean
	$(MAKE) -C win3x	clean
	$(MAKE) -C unix		clean

distclean:	clean
	$(MAKE) -C oldlibc	distclean
	$(MAKE) -C inet		distclean
	$(MAKE) -C netgroup	distclean
	$(MAKE) -C posix	distclean
	$(MAKE) -C wsock	distclean
	$(MAKE) -C csock	distclean
	$(MAKE) -C win9x	distclean
	$(MAKE) -C win3x	distclean
	$(MAKE) -C unix		distclean
	-rm $(DOCS) funcs.txi
	rm -f *.\$$\$$\$$
	rm -f *~

# Make info files
lsck.inf:	lsck.txi *.txh */*.txh
	-mkdir ../info
	$(MKDOC) . ./funcs.txi
	$(MAKEINFO) --verbose --no-split $< -o $@
	cp -u $@ ../info

# Make HTML docs from info files
%.htm:	%.txi
	-mkdir ../doc
	$(MAKEINFO) --verbose --html $< -o $@
	cp -u $@ ../doc

# Make text docs from info file
../bugs.txt:	lsck.inf
	$(INFO) --subnodes -f $< 'Known Bugs' -o $@

../credits.txt:	lsck.inf
	$(INFO) --subnodes -f $< Credits -o $@

../changes.txt:	lsck.inf
	$(INFO) --subnodes -f $< Changelog -o $@

../install.txt:	lsck.inf
	$(INFO) --subnodes -f $< Installation -o $@

../config.txt ../redist/config.txt:	lsck.inf
	$(INFO) --subnodes -f $< Configuration -o $@

$(OBJS) $(LOBJS):
include depend.dep
