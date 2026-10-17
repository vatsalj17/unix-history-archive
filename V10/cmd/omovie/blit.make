# for normal layers software, use dmdcc;
# for version 9, use 3cc.
# you may also find it called m32cc.
DMDCC = dmdcc
DMDCC = /usr/add-on/bin/dmd2.0/dmdcc
DMDCC = 3cc
TERMLD = 32ld

# pick one for yourself.  "." is good for personal use.
LIB = .
LIB = /usr/lib/movie

# use V9 only for version 9 (mux) blit version
# use SYS_V instead of V9 for System V layers/dmd
# default seems good for BSD.
CFLAGS = -g
CFLAGS = -g -DSYS_V
CFLAGS = -g -DV9
CC = cc

HOBJ = host.o hsupport.o protocol.o
TOBJ = term.o tprotocol.o

here:	anim animterm

anim:	$(HOBJ)
	$(CC) $(CFLAGS) $(HOBJ) -o anim

host.o:	anim.h
	$(CC) -c $(CFLAGS) -DBLIT \
		-DANIMTERM=\"$(LIB)/animterm\" -DTERMLD=\"$(TERMLD)\" host.c

animterm: term.c protocol.c
	test -r tprotocol.c || ln protocol.c tprotocol.c
	$(DMDCC) $(CFLAGS) term.c tprotocol.c -o animterm

term.o: anim.h
