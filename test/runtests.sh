#! /bin/bash
#
# runtest.sh, run libsocket test suite
#
# libsocket Copyright 1997, 1998 by Indrek Mandre
# libsocket Copyright 1997-1999 by Richard Dawe

# If you want to debug a test, run with "runtests.sh 'gdb -d../src ...'"
debug_it=$1

# Interfaces to test
interfaces='wsock csock unix'

# Tests to run - each test should take one argument, the name of the interface
# to test. The current directory is prepended automatically.
tests='create'

for i in $interfaces; do
	LSCK=`pwd`/cfg/$i

	echo "--- Performing $i interface tests ---"
	echo ". Using configuration from $LSCK"
	echo

	for j in $tests; do
		# Do the test
		$debug_it ./$j $i

		# Pass error on to parent
		if [ $? != 0 ]; then
			exit $?;
		fi
	done

	echo
done
