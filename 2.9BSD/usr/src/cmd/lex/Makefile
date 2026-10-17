CFLAGS=		-O
LDFLAGS=	-i
SRC=		header.c lmain.c sub1.c sub2.c y.tab.c
OBJS=		header.o lmain.o sub1.o sub2.o y.tab.o

all:		lex 

cmp:		all
		cmp lex ${DESTDIR}/usr/bin/lex

cp:		all
		cp lex ${DESTDIR}/usr/bin/lex
		-strip ${DESTDIR}/usr/bin/lex

lex:		${OBJS}
		cc ${LDFLAGS} ${OBJS} -o lex

smallex:
		cc -DSMALL ${CFLAGS} ${LDFLAGS} ${SRC} -o smallex

y.tab.c:	parser.y
		yacc parser.y

lmain.o:	lmain.c
lmain.o:	ldefs.c
lmain.o:	once.c
sub1.o:		sub1.c
sub1.o:		ldefs.c
sub2.o:		sub2.c
sub2.o:		ldefs.c
header.o:	header.c
header.o:	ldefs.c

clean:
		rm -f *.o lex y.tab.c
