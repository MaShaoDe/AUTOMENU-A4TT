# Automenu - works with BSD make (FreeBSD, macOS) and GNU make (Linux)
#
#   make                 build
#   make test            run the tests
#   make install         install to PREFIX (default /usr/local)
#   make uninstall
#   make clean
#
# Examples:
#   sudo make install                    to /usr/local
#   make install PREFIX=$HOME/.local     without admin rights

VERSION    = 0.9.2
PREFIX    ?= /usr/local
SYSCONFDIR?= $(PREFIX)/etc
DESTDIR   ?=
CC        ?= cc
CFLAGS    ?= -O2
WARN       = -Wall -Wextra
SRCS       = src/core.c src/loader.c src/auth.c src/terminal.c
HDRS       = src/core.h src/loader.h src/auth.h src/menu.h src/version.h
DEFS       = -DSYSCONFDIR=\"$(SYSCONFDIR)\"

all: automenu

automenu: $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) $(WARN) -Isrc $(DEFS) -o automenu $(SRCS)

test: tests/test_all.c src/core.c src/loader.c src/auth.c $(HDRS)
	$(CC) $(CFLAGS) $(WARN) -Isrc -o test_all tests/test_all.c src/core.c src/loader.c src/auth.c
	./test_all
	rm -f test_all

install: automenu
	install -d $(DESTDIR)$(PREFIX)/bin
	install -d $(DESTDIR)$(PREFIX)/share/man/man1
	install -d $(DESTDIR)$(SYSCONFDIR)/automenu
	install -d $(DESTDIR)$(PREFIX)/share/examples/automenu
	install -m 755 automenu $(DESTDIR)$(PREFIX)/bin/automenu
	install -m 644 man/automenu.1 $(DESTDIR)$(PREFIX)/share/man/man1/automenu.1
	install -m 644 examples/menu.mdf $(DESTDIR)$(SYSCONFDIR)/automenu/menu.mdf.sample
	install -m 644 examples/menu.mdf examples/ham.mdf examples/things.mdf $(DESTDIR)$(PREFIX)/share/examples/automenu/

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/automenu
	rm -f $(DESTDIR)$(PREFIX)/share/man/man1/automenu.1
	rm -f $(DESTDIR)$(SYSCONFDIR)/automenu/menu.mdf.sample
	rm -rf $(DESTDIR)$(PREFIX)/share/examples/automenu

clean:
	rm -f automenu test_all

.PHONY: all test install uninstall clean
