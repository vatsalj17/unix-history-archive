FMT=	-i		# -n for nonseparate I/D
ED=	-ed
AS=	as

BINDIR= /usr/bin
LIBDIR= /usr/lib

VERSION= version
EYACC=	/usr/ucb/eyacc -v
CFLAGS=	-O -DPXP
OBJS=	call.o case.o const.o cset.o \
	error.o fdec.o func.o hash.o lab.o lval.o \
	main.o nl.o pmon.o pp.o proc.o rec.o rval.o \
	stat.o string.o subr.o tree.o type.o var.o y.tab.o \
	yycomm.o yycosts.o yyerror.o yyget.o yyid.o yylex.o yymain.o \
	yypanic.o yyparse.o yyprint.o yyput.o yyrecover.o \
	yyseman.o yytree.o

#  On a separate I/D machine without floating point,
#  you will have to use ../utililities/fpterp/fp.o instead of
#  ../utilities/fpnofetchi/fp.o; see the comments in ../utilities/fpterp.

OBJS2=	printf.o treen.o yycopy.o ../utilities/fpnofetchi/fp.o \
	TRdata.o ../px/getc.o ../px/putc.o

all: pxp

cp: pxp
	cp pxp ${DESTDIR}${BINDIR}/pxp
	cp how_pxp ${DESTDIR}${BINDIR}

cmp: pxp
	cmp pxp ${DESTDIR}${BINDIR}/pxp

pxp:	${OBJS} ${OBJS2}
	${CC} -c Version.c
	${CC} ${FMT} -o pxp ${OBJS} ${OBJS2} Version.o

y.tab.c: pas.y
	${EYACC} pas.y
	${ED} - y.tab.c < gram

../utilities/fpterp/fp.o:
	cd ../utilities/fpterp; make fp.o

../utilities/fpnofetchi/fp.o:
	cd ../utilities/fpnofetchi; make fp.o

../px/getc.o: ../px/getc.s
	cd ../px; make getc.o

../px/putc.o: ../px/putc.s
	cd ../px; make putc.o

clean:
	-rm -f pi1.2strings a.out core y.tab.c y.output *.o x*

print:
	@pr makefile READ_ME
	@ls -ls | pr
	@size a.out *.o | pr
	-@cxref *.c | pr
	@pr 0.h main.c pas.y
	@pr yy.h yy*.c
	@pr [a-ln-x]*.c
	@pr putn.s treen.s yycopy.s
