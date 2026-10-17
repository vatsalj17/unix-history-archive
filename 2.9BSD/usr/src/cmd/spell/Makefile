# spell and its hash lists must be reduced in size to run
# on a non-separate I/D machine.  This is done by defining NONSEPERATE
# (in <sys/localopts.h>) instead of using the -i compiler flag.

FMT = -i		# -n for nonseparate I/D

all:	spell hlist hlista hlistb hstop spellin
	:

cp:	all
	-strip spell
	cp spell ${DESTDIR}/usr/lib
	cp hlista ${DESTDIR}/usr/dict
	cp hlistb ${DESTDIR}/usr/dict
	cp hstop ${DESTDIR}/usr/dict
	rm spell hlista hlistb hstop hlist

cmp:	spell
	cmp spell ${DESTDIR}/usr/lib/spell

spell: spell.c spell.h
	cc $(FMT) -O -s spell.c -o spell
spellin: spellin.c spell.h
	cc $(FMT) -O -s spellin.c -o spellin
spellout: spellout.c spell.h
	cc $(FMT) -O -s spellout.c -o spellout

hlist: /usr/dict/words spellin
	spellin </usr/dict/words >hlist
hlista: american local hlist spellin
	(cat american local)|spellin hlist >hlista
hlistb: british local hlist spellin
	(cat british local)|spellin hlist >hlistb
hstop: stop spellin
	spellin <stop >hstop

clean:
	rm -f *.o
