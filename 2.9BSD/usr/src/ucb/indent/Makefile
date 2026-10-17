#	@(#)Makefile	4.2	(Berkeley)	10/22/82
#
DESTDIR=
CFLAGS=	-O
LFLAGS=-n ${FP}
OBJS=	indent.o io.o lexi.o parse.o pr_comment.o

indent: indent.o io.o lexi.o parse.o pr_comment.o
	cc ${LFLAGS} -o indent ${OBJS}

clean:
	rm -f ${OBJS} indent

cp:	indent
	cp indent ${DESTDIR}/usr/ucb
	strip ${DESTDIR}/usr/ucb/indent
