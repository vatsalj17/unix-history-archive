all:	as as2

ovall:	ovas ovas2

cmp:	as as2
	cmp as /bin/as
	cmp as2 /lib/as2
	rm as as2 a.out

cp:	as as2
	cp as /bin/as
	cp as2 /lib/as2
	rm as as2 a.out

ovcp:	ovas ovas2
	cp ovas /bin/ovas
	cp ovas2 /lib/ovas2
	rm ovas ovas2 a.out

as:
	/bin/as /usr/include/sys.s nov.s as1?.s
	ld -n -s a.out -o as

as2:
	/bin/as /usr/include/sys.s nov.s as2?.s
	ld -n -s a.out -o as2

ovas:
	/bin/as /usr/include/sys.s ov.s as1?.s
	ld -n -s a.out -o ovas

ovas2:
	/bin/as /usr/include/sys.s ov.s as2?.s
	ld -n -s a.out -o ovas2
