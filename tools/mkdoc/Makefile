#
# Makefile for mkdoc for use with libsocket
# Written by Richard Dawe <richdawe@bigfoot.com>
#

include ../../Makefile.cfg

TARGETS     = mkdoc
TARGETS_EXE = $(patsubst %,%.exe,$(TARGETS))

default:	all

all:	$(TARGETS)

mkdoc:	mkdoc.cc
	$(CXX) $(CXXFLAGS) $< -o $@

clean:

distclean:
	rm -f $(TARGETS) $(TARGETS_EXE)
