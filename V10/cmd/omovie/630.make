# this is only for 630 with system v layers.
# no guarantees about pathnames or files
DMDCC = dmdcc
DMDCC = /usr/add-on/630/bin/dmdcc
TERMLD = dmdld
TERMLD = /usr/add-on/630/bin/dmdld

# pick one for yourself:
LIB = /usr/lib/movie
LIB = .

# use V9 only for version 9 (mux) blit version
CFLAGS = -g -DSYS_V -DTTY630
CC = cc

HOBJ = host.o hsupport.o protocol.o
TOBJ = term.o tprotocol.o

here:	anim animterm

anim:	$(HOBJ)
	$(CC) $(CFLAGS) $(HOBJ) -o anim

host.o:	anim.h
	$(CC) -c $(CFLAGS) -DBLIT \
		-DANIMTERM=\"$(LIB)/animterm\" -DTERMLD=\"$(TERMLD)\" host.c

animterm: term.c tprotocol.c
	$(DMDCC) $(CFLAGS) term.c tprotocol.c -o animterm

tprotocol.c:
	test -r tprotocol.c || ln protocol.c tprotocol.c

term.o: anim.h
