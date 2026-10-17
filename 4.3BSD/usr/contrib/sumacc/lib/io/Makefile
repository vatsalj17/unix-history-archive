.SUFFIXES: .b
STUBS= adddrive.b allocate.b appfiles.b control.b create.b disk.b  drvr.b eject.b flushfile.b flushvol.b fsclose.b fsdelete.b fsopen.b fsread.b geteof.b getfinfo.b getfpos.b getvinfo.b getvol.b killio.b mountvol.b offline.b openrf.b pb.b rename.b rstflock.b ser.b setfinfo.b setflock.b setftype.b sound.b status.b

#	turns .s into .b and strips about 700 useless abs symbols
.s.b:
	cc68 -c $<
	ld68 -r -x -T 0 $*.b
	mv b.out $*.b

all:	$(STUBS)

clean:
	rm -f *.a *.ln *.o *.b *.sl *.s.b *.s.list
