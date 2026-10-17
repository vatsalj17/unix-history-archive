DP=/usr/sun/bin
DL=/usr/sun/lib
PRGS=cc68 as68 c268 ccom68 ld68 lorder68 nm68 pr68 rl68 size68
LIBS=a68.hdr

install: cc68
	cp $(LIBS) $(DL)
#	cp $(PRGS) $(DP)

cc68:	cc68.o
	cc -o cc68 cc68.o

clean:
	rm -f cc68 *.o
