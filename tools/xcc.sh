#! /bin/bash
#
# xcc.sh
# Written by Richard Dawe <richdawe@bigfoot.com>
#
# This script runs the configure to build with a cross-compiler. I use this
# on Linux, but it should work with any cross-compiler built using the
# instructions on the DJGPP website ( http://www.delorie.com/djgpp/ ).

CC=/usr/local/bin/i586-pc-msdosdjgpp-gcc			\
CXX=/usr/local/bin/i586-pc-msdosdjgpp-g++			\
AS=/usr/local/bin/i586-pc-msdosdjgpp-as				\
AR=/usr/local/bin/i586-pc-msdosdjgpp-ar				\
STRIP=/usr/local/bin/i586-pc-msdosdjgpp-strip			\
RANLIB=/usr/local/bin/i586-pc-msdosdjgpp-ranlib			\
INSTALL_INFO=/sbin/install-info                                 \
	./config	--target=i586-pc-msdosdjgpp		\
			--enable-debug				\
			--prefix=/usr/local/i586-pc-msdosdjgpp
