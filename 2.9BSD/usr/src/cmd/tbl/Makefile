CFLAGS=-O -s
FMT= -i		# or nothing for nonseparate I/D

all:	tbl
	:

cp:	tbl
	-strip tbl
	cp tbl ${DESTDIR}/usr/bin/tbl
	rm tbl *.o

cmp:	tbl
	cmp tbl ${DESTDIR}/usr/bin/tbl
	rm *.o

tbl: t0.o t1.o t2.o t3.o t4.o t5.o t6.o t7.o t8.o t9.o tb.o tc.o te.o tf.o tg.o ti.o tm.o ts.o tt.o tu.o tv.o
	cc $(LDFLAGS) -s -O t?.o -o tbl
t0.o t1.o t2.o t3.o t4.o t5.o t6.o t7.o t8.o t9.o tb.o tc.o te.o tf.o tg.o ti.o tm.o ts.o tt.o tu.o tv.o: t..c

clean:
	rm -f *.o
