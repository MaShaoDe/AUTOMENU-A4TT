FreeBSD-Port (Gerüst, auf FreeBSD noch nicht getestet)

Schritte auf einem FreeBSD-Rechner:
 1. Im GitHub-Repo den Tag v0.2.1 setzen (git tag v0.2.1 && git push --tags).
 2. Ordner automenu/ nach /usr/ports/misc/automenu kopieren.
 3. MAINTAINER in der Makefile eintragen.
 4. cd /usr/ports/misc/automenu && make makesum     (erzeugt distinfo)
 5. make stage && make check-plist
 6. portlint -AC
 7. Test sauber mit poudriere: poudriere testport -j <jail> -p <tree> misc/automenu
Danach kann das Paket mit "make package" gebaut oder der Port bei FreeBSD eingereicht werden.
