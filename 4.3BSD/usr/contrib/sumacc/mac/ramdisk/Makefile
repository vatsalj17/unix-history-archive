.SUFFIXES: .rsrc .b

.c.b:
	cc68 -O -c $<

#	turns .s into .b and strips about 700 useless abs symbols
.s.b:
	cc68 -c $<
	ld68 -r -x -T 0 $*.b
	mv b.out $*.b

all:	rd.rsrc

rd.rsrc: rd.b rdlib.b crtdrvr.b rdinit.b rd.rc
	ld68 -X -r -d -e drvr -T 0 crtdrvr.b rd.b rdlib.b -lmac -lc -x 
	mv b.out rd.out
	ld68 -X -r -d -e _start -T 0 rdinit.b -x 
	mv b.out rdinit.out
	rmaker $*.rc

clean:
	rm  -f *.b *.rsrc *.out *.dl
