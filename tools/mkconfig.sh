#! /bin/bash
#
# mkconfig.sh, Copyright 1999 by Richard Dawe
#
# This file creates a new configure file called 'config', but replaces the
# references to 'config.guess' to 'config.gue', so when can run it under
# plain ol' DOS.

autoconf config.in | sed -e 's/config\.guess/config\.gue/' > config
