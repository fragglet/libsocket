#
# Makefile for libsocket 0.8.0's source distribution
#
# Copyright (C) 1997, 1998 by Indrek Mandre
# Copyright (C) 1997-2000 by Richard Dawe
#

include Makefile.cfg

default:
	@$(CAT) Targets.src

# Inform make of phony targets
.PHONY:	tools library demos netsetup clean blankdep dep distclean	\
	install uninstall setup	lib library check installvxd all

all: library demos netsetup check
	@echo "Have you run setup (hint: make setup)?"
	@echo "Have you installed SOCK.VXD (hint: make installvxd)?"

lib:	library

library:	tools src
	$(MAKE) -C src all

tools:
	$(MAKE) -C tools all

demos: demo
	$(MAKE) -C demo all	

check:
	$(MAKE) -C test all

# Setup program
setup:	netsetup
	setup/netsetup/netsetup

netsetup:
	$(MAKE) -C setup/netsetup all	

# Build a tags file for Emacs.
tags:
	find include src -iname '*.[ch]' -print | etags - -o $@

# Build a table for GNU id-utils. Do not index the paths in $(ID_PRUNE_PATHS).
# This is mainly of benefit to the maintainer. ;)
ID_PRUNE_PATHS = contrib dist doc html manifest redist old share

id:	ID

ID:
	mkid --statistics -o $@ $(patsubst %,-p %,$(ID_PRUNE_PATHS))

clean:
	$(MAKE) -C src			clean
	$(MAKE) -C demo			clean
	$(MAKE) -C test			clean
	$(MAKE) -C setup/netsetup	clean
	$(MAKE) -C tools		clean

blankdep:
# Create blank depend.dep files to avoid errors	
	@echo > src/depend.dep
	@echo > src/oldlibc/depend.dep
	@echo > src/inet/depend.dep
	@echo > src/netgroup/depend.dep
	@echo > src/posix/depend.dep
	@echo > src/csock/depend.dep
	@echo > src/wsock/depend.dep
	@echo > src/win9x/depend.dep
	@echo > src/win9x/regdos/depend.dep
	@echo > src/win3x/depend.dep
	@echo > src/unix/depend.dep
	@echo > test/depend.dep
	@echo Created blank dependency files

dep:	blankdep
# Now carry on as usual
	$(MAKE) -C src			dep

# Blank all the dependencies too
distclean:	blankdep
	$(MAKE) -C src			distclean
	$(MAKE) -C demo			distclean
	$(MAKE) -C setup/netsetup	distclean
	$(MAKE) -C test			distclean
	$(MAKE) -C tools		distclean
	rm -f config.cache config.status config.log
	rm -f *.\$$\$$\$$
	rm -f *~
	rm -f include/*.\$$\$$\$$
	rm -f include/*~
	rm -f include/*/*.\$$\$$\$$
	rm -f include/*/*~
	rm -f lib/*.a
	rm -f info/*.inf
	rm -f doc/*.htm
	rm -f tags ID

# Install/uninstall options
install:	library netsetup
	@echo Backing up header file "netinet/in.h"
	-cp $(includedir)/netinet/in.h $(includedir)/netinet/in-old.h
# Let this fail if the prefix isn't $(DJDIR)
	-chmod a-w $(includedir)/netinet/in-old.h
	@echo
	@echo Now installing...
# Netsetup binaries
	$(INSTALL) -d $(bindir)
	$(INSTALL) setup/netsetup/netsetup.exe $(bindir)
# Libraries
	$(INSTALL) -d $(libdir)
	$(INSTALL) lib/libsocket.a $(libdir)
# For a simple life, create a SFN version too.
	$(INSTALL) lib/libsocket.a $(libdir)/libsocke.a
# Include files
	$(INSTALL) -d $(includedir)/arpa
	$(INSTALL) -d $(includedir)/lsck
	$(INSTALL) -d $(includedir)/net
	$(INSTALL) -d $(includedir)/netinet
	$(INSTALL) -d $(includedir)/sys
	$(INSTALL_DATA) include/*.h         $(includedir)
	$(INSTALL_DATA) include/arpa/*.h    $(includedir)/arpa
	$(INSTALL_DATA) include/lsck/*.h    $(includedir)/lsck
	$(INSTALL_DATA) include/net/*.h     $(includedir)/net
	$(INSTALL_DATA) include/netinet/*.h $(includedir)/netinet
	$(INSTALL_DATA) include/sys/*.h     $(includedir)/sys
# Info documentation
	$(INSTALL) -d $(infodir)
	$(INSTALL_DATA) info/*.inf* $(infodir)
	$(INSTALL_INFO)	--info-file=$(infodir)/lsck.inf		\
			--info-dir=$(infodir)
	$(INSTALL_INFO)	--info-file=$(infodir)/netsetup.inf	\
			--info-dir=$(infodir)
# Distribution docs
	$(INSTALL) -d $(contribdir)
	$(INSTALL_DATA) announce.txt $(contribdir)
	$(INSTALL_DATA) bugs.txt     $(contribdir)
	$(INSTALL_DATA) credits.txt  $(contribdir)
	$(INSTALL_DATA) changes.txt  $(contribdir)
	$(INSTALL_DATA) install.txt  $(contribdir)
	$(INSTALL_DATA) license.txt  $(contribdir)
	$(INSTALL_DATA) readme.txt   $(contribdir)
	$(INSTALL) -d $(contribdir)/setup/netsetup
	$(INSTALL_DATA) setup/netsetup/license.txt $(contribdir)/setup/netsetup
# Miscellaneous documentation
	$(INSTALL) -d $(contribdir)/doc
	$(INSTALL) -d $(contribdir)/doc/beejng
	$(INSTALL) -d $(contribdir)/doc/windows
	$(INSTALL_DATA) doc/*.*         $(contribdir)/doc
	$(INSTALL_DATA) doc/beejng/*.*  $(contribdir)/doc/beejng
	$(INSTALL_DATA) doc/windows/*.* $(contribdir)/doc/windows
# Demos
	$(INSTALL) -d $(contribdir)/demo
	$(INSTALL) -d $(contribdir)/demo/examples
	$(INSTALL_DATA) demo/Makefile        $(contribdir)/demo
	$(INSTALL_DATA) demo/readme          $(contribdir)/demo
	$(INSTALL_DATA) demo/*.c             $(contribdir)/demo
	$(INSTALL_DATA) demo/examples/readme $(contribdir)/demo/examples
	$(INSTALL_DATA) demo/examples/*.c    $(contribdir)/demo/examples
# Redistributable components
	$(INSTALL) -d $(contribdir)/redist
	$(INSTALL_DATA) redist/*.* $(contribdir)/redist

uninstall:
	@echo Now uninstalling...
# Netsetup binaries
	-rm $(bindir)/netsetup.exe
# Libraries
	-rm $(libdir)/libsocket.a
	-rm $(libdir)/libsocke.a
# Include files
	-rm $(includedir)/ioctls.h
	-rm $(includedir)/netdb.h
	-rm $(includedir)/resolv.h
	-rm $(includedir)/arpa/inet.h
	-rm $(includedir)/arpa/nameser.h
	-rm $(includedir)/arpa/ftp.h
	-rm $(includedir)/arpa/telnet.h
	-rm $(includedir)/arpa/tftp.h
	-rm $(includedir)/lsck/*.h
	-rm $(includedir)/net/if.h
	-rm $(includedir)/netinet/in.h
	-rm $(includedir)/sys/socket.h
	-rm $(includedir)/sys/uio.h
	-rm $(includedir)/sys/un.h
	-rmdir $(includedir)/arpa
	-rmdir $(includedir)/lsck
	-rmdir $(includedir)/net
	-rmdir $(includedir)/netinet
	-rmdir $(includedir)/sys
# Info documentation
	$(INSTALL_INFO)	--delete --info-file=$(infodir)/lsck.inf	\
			--info-dir=$(infodir)
	$(INSTALL_INFO)	--delete --info-file=$(infodir)/netsetup.inf	\
			--info-dir=$(infodir)
	-rm $(infodir)/lsck.inf $(infodir)/netsetup.inf
# Miscellaneous distribution stuff
	-rm -Rf $(contribdir)
# Restore old headers
	@echo
	@echo Restoring header file "netinet/in.h"
	-cp $(includedir)/netinet/in-old.h $(includedir)/netinet/in.h
	-rm $(includedir)/netinet/in-old.h

# Install SOCK.VXD
installvxd:
	start /w redist/sockvxd.exe
