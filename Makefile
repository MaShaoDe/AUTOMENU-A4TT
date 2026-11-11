# Automenu - läuft mit BSD make (FreeBSD, macOS) und GNU make (Linux)
#
#   make                 bauen
#   make test            Tests ausführen
#   make kiosk           Kiosk-Version bauen (Ctrl-C beendet nicht)
#   make install         installieren nach PREFIX (Standard /usr/local)
#   make uninstall
#   make clean
#
# Beispiele:
#   sudo make install                    nach /usr/local
#   make install PREFIX=$HOME/.local     ohne Admin-Rechte

VERSION    = 0.3.5
PREFIX    ?= /usr/local
SYSCONFDIR?= $(PREFIX)/etc
DESTDIR   ?=
CC        ?= cc
CFLAGS    ?= -O2
WARN       = -Wall -Wextra
SRCS       = src/core.c src/loader.c src/terminal.c
HDRS       = src/core.h src/loader.h src/menu.h src/version.h
DEFS       = -DSYSCONFDIR=\"$(SYSCONFDIR)\"

all: automenu

automenu: $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) $(WARN) -Isrc $(DEFS) -o automenu $(SRCS)

kiosk: $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) $(WARN) -Isrc $(DEFS) -DKIOSK -o automenu $(SRCS)

test: tests/test_all.c src/core.c src/loader.c $(HDRS)
	$(CC) $(CFLAGS) $(WARN) -Isrc -o test_all tests/test_all.c src/core.c src/loader.c
	./test_all
	rm -f test_all

install: automenu
	install -d $(DESTDIR)$(PREFIX)/bin
	install -d $(DESTDIR)$(PREFIX)/share/man/man1
	install -d $(DESTDIR)$(SYSCONFDIR)/automenu
	install -d $(DESTDIR)$(PREFIX)/share/examples/automenu
	install -m 755 automenu $(DESTDIR)$(PREFIX)/bin/automenu
	install -m 644 man/automenu.1 $(DESTDIR)$(PREFIX)/share/man/man1/automenu.1
	install -m 644 examples/menu.txt $(DESTDIR)$(SYSCONFDIR)/automenu/menu.txt.sample
	install -m 644 examples/menu.txt examples/ham.txt examples/things.txt $(DESTDIR)$(PREFIX)/share/examples/automenu/

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/automenu
	rm -f $(DESTDIR)$(PREFIX)/share/man/man1/automenu.1
	rm -f $(DESTDIR)$(SYSCONFDIR)/automenu/menu.txt.sample
	rm -rf $(DESTDIR)$(PREFIX)/share/examples/automenu

clean:
	rm -f automenu test_all

.PHONY: all kiosk test install uninstall clean
