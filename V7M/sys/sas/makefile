CFLAGS = -O

all:	/lib/libsa.a srt0.o restor scat icheck mkfs htboot mtboot maketape boot

cp cmp:
	@echo "standalone stuff-- examine this makefile before running"

xcp:	all
	cp restor scat mkfs icheck /sas
	rm restor scat mkfs icheck
	cp mtboot /sas
	cp htboot /sas
	rm htboot mtboot
	cp maketape /sas
	rm maketape
	cp tapedir800 /sas
	cp tapedir1600 /sas
	cp contents /sas
	cp boot /sas
	cp boot /
	rm boot
	chmod 600 /sas/*
	chmod 700 /sas/maketape

/lib/libsa.a:	SYS.o conf.o hp.o ht.o prf.o rk.o rp.o tm.o hk.o ts.o rl.o hm.o
	ar rv /lib/libsa.a $?

restor:	srt0.o
	cc+ld-stand restor

scat:	srt0.o
	cc -c scat.c
	ld -s -o scat srt0.o scat.o -lsa -lc
	rm scat.o

mkfs:	srt0.o
	cc+ld-stand mkfs

icheck:	srt0.o
	cc+ld-stand icheck

htboot:	htboot.s
	as -o htboot htboot.s
	strip htboot

mtboot:	mtboot.s
	as -o mtboot mtboot.s
	strip mtboot

maketape:	maketape.c
	cc -o maketape maketape.c

boot:	boot.o M.o
	ld -s -o boot M.o boot.o -lsa -lc
