libsocket Redistributable Components
====================================

This directory contains components necessary for libsocket functionality that
can and should be distributed with libsocket programs for end-users. It is
recommended that you distribute this file with the ones described below.

Configuration
-------------

The netsetup program can be distributed freely - see ../setup/netsetup/
for the sources/binaries. The document 'u-guide.txt' is intended to be
distributed with programs, to tell users how to configure libsocket. The
document 'config.txt' is the 'Configuration' section from the libsocket
info documentation. These documents do not rely on the user having netsetup,
but it might be easier for them to use netsetup than configure libsocket
manually.

Winsock 2 - csock Interface
---------------------------

The following files are needed for the Winsock 2 interface to function:

sockvxd.exe SOCK.VXD self-extracting installer
license.txt SOCK.VXD licence
sources.txt SOCK.VXD source location (as required by its licence)

To install SOCK.VXD, simply run the installer and select 'dynamic loading'
when prompted. libsocket will load SOCK.VXD as required.

Richard Dawe <richdawe@bigfoot.com> 2000-05-29
