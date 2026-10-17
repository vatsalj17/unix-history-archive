DP=/usr/stanford/bin
DL=/usr/sun/lib
PRGS=cc68 as68 c268 ccom68 ld68 lorder68 nm68 pr68 rl68 size68
LIBS=a68.hdr libc.a

install: cc68
	cp $(PRGS) $(DP)
	cp $(LIBS) $(DL)

cc68:	cc68.o
	cc -o cc68 cc68.o
