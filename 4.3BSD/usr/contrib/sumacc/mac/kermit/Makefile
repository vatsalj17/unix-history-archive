.SUFFIXES:  .b .c

OBJECTS = kermit.b term.b

.c.b:
	cc68  -c $<

kermit.rsrc:  kermit.rc b.out 
	rmaker kermit.rc

b.out: kermit.b term.b
	cc68 -z -m ${OBJECTS}

clean:
	rm -f *.b *.rsrc
