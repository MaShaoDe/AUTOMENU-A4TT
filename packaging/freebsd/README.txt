FreeBSD port (skeleton, not yet tested on FreeBSD)

Steps on a FreeBSD machine:
 1. Set the tag vX.Y.Z (matching DISTVERSION in the port Makefile) in the GitHub repo
    (git tag vX.Y.Z && git push --tags).
 2. Copy the folder automenu/ to /usr/ports/misc/automenu.
 3. Enter MAINTAINER in the Makefile.
 4. cd /usr/ports/misc/automenu && make makesum     (creates distinfo)
 5. make stage && make check-plist
 6. portlint -AC
 7. Clean test with poudriere: poudriere testport -j <jail> -p <tree> misc/automenu
Afterwards the package can be built with "make package" or the port can be submitted to FreeBSD.
