#! /bin/bash
# gdbx.sh - Start up gdb with all libsocket sources available
# Written by Richard Dawe <richdawe@bigfoot.com>

LSCKDIRS=`find ../src -type d`
RUNGDB="gdb $1"

echo "Running gdb with access to sources in:"
echo "$LSCKDIRS"
echo

for i in $LSCKDIRS; do
    RUNGDB="$RUNGDB --dir=$i";
done

$RUNGDB