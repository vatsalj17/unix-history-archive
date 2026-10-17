CC=		cc
DEFS=		-DNUMDEV=256 -DTERMCAP
CFLAGS=		-O ${INCLUDES}
LDFLAGS=	-n
LIBS=		-ltermcap		# if TERMCAP is defined

all:	ps

cp:	ps
	-strip ps
	cp ps ${DESTDIR}/bin/ps
	rm ps *.o

cmp:	ps
	cmp ps ${DESTDIR}/bin/ps
	rm *.o

ps:		ps.o bmove.11.o
		${CC} ${LDFLAGS} -o $@ $@.o bmove.11.o ${LIBS}

ps.o:		ps.c
		${CC} ${DEFS} ${CFLAGS} -c ps.c

clean:
	rm -f *.o
