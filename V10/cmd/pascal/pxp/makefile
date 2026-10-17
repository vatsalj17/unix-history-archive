SCCSID = "@(#)makefile	1.2 (Berkeley) 3/5/81"
DESTDIR=
ED=	-ed
AS=	-as
VERSION= version
EYACC=	/usr/bin/eyacc
CFLAGS=	-O -DPXP
OBJS=	call.o case.o const.o cset.o \
	error.o fdec.o func.o hash.o lab.o lval.o \
	main.o nl.o pmon.o pp.o proc.o rec.o rval.o \
	stat.o string.o subr.o tree.o type.o var.o y.tab.o \
	yycomm.o yycosts.o yyerror.o yyget.o yyid.o yylex.o yymain.o \
	yypanic.o yyparse.o yyprint.o yyput.o yyrecover.o \
	yyseman.o yytree.o
OBJS2=	doprnt.o printf.o treen.o yycopy.o TRdata.o Version.o

all: a.out

a.out:	${OBJS} ${OBJS2}
	${CC} ${CFLAGS} ${OBJS} ${OBJS2}

doprnt.o: doprnt.c
	${CC} ${CFLAGS} -E doprnt.c > doprnt.s
	${AS} -o doprnt.o doprnt.s
	rm -f doprnt.s

#y.tab.c: pas.y
#	${EYACC} pas.y
#	${ED} - y.tab.c < gram

clean:
	rm -f pi1:2strings a.out core
	rm -f *.o x*
	# don't remove y.tab.c

install: a.out
	strip a.out
	rm -f ${DESTDIR}/usr/bin/pxp
	cp a.out ${DESTDIR}/usr/bin/pxp
	chown bin ${DESTDIR}/usr/bin/pxp
	chgrp bin ${DESTDIR}/usr/bin/pxp
	chmod o-w,g+w ${DESTDIR}/usr/bin/pxp
#	cp a.outNOID /usr/bin/pxp34

print:
	@pr makefile READ_ME
	@ls -ls | pr
	@size a.out *.o | pr
#	@cxref *.c | pr
	@pr 0.h main.c pas.y
	@pr yy.h yy*.c
	@pr [a-ln-x]*.c
#	@pr putn.s treen.s yycopy.s
