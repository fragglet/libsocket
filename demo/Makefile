#
# Makefile for libsocket test programs
#
# libsocket Copyright 1997, 1998 by Indrek Mandre
# libsocket Copyright 1997, 1998 by Richard Dawe
#
# Originally written by Indrek Mandre, modified by Richard Dawe
#

include ../Makefile.cfg

LDFLAGS = -L../lib
LIBS = -lsocket

CFLAGS += -I../include

TEST1 = netserv.o
TEST2 = netproto.o
TEST3 = netnet.o
SERVER = server.o
CLIENT = client.o
HTTPGET = httpget.o
RESOLVE = resit.o
SELECT = select.o
DIAG = diag.o

TARGETS = netserv netproto server client httpget resit select diag netnet

all: $(TARGETS)

diag:	$(DIAG)
	$(CC) -o diag $(CFLAGS) $(DIAG) $(LDFLAGS) $(LIBS)

httpget: $(HTTPGET)
	$(CC) -o httpget $(CFLAGS) $(HTTPGET) $(LDFLAGS) $(LIBS)

resit: $(RESOLVE)
	$(CC) -o resit $(CFLAGS) $(RESOLVE) $(LDFLAGS) $(LIBS)

select: $(SELECT)
	$(CC) -o select $(CFLAGS) $(SELECT) $(LDFLAGS) $(LIBS)

server: $(SERVER)
	$(CC) -o server $(CFLAGS) $(SERVER) $(LDFLAGS) $(LIBS)

client: $(CLIENT)
	$(CC) -o client $(CFLAGS) $(CLIENT) $(LDFLAGS) $(LIBS)

netserv: $(TEST1)
	$(CC) -o netserv $(CFLAGS) $(TEST1) $(LDFLAGS) $(LIBS)

netproto: $(TEST2)
	$(CC) -o netproto $(CFLAGS) $(TEST2) $(LDFLAGS) $(LIBS)

netnet:	$(TEST3)
	$(CC) -o netnet $(CFLAGS) $(TEST3) $(LDFLAGS) $(LIBS)

dep:
	$(CC) $(CFLAGS) -MM *.c > depend.dep

clean:
	rm -f $(TEST1) $(TEST2) $(TEST3) $(CLIENT) $(SERVER) $(HTTPGET) $(RESOLVE) $(SELECT) $(DIAG)

distclean: clean
	rm -f $(TARGETS) *.exe

$(OBJS) $(LOBJS):
include depend.dep
