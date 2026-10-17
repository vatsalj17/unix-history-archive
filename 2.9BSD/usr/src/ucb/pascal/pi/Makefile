#	@(#)makefile	2.4	SCCS id keyword

HDRS =	0.h OPnames.h opcode.h tree.h yy.h

SRCS =	ato.c \
	call.c case.c clas.c const.c conv.c cset.c \
	error.c fdec.c func.c gen.c hash.c \
	lab.c lookup.c lval.c \
	main.c nl.c proc.c put.c \
	rec.c rval.c \
	stat.c string.c subr.c \
	tree.c type.c var.c Version.c \
	TRdata.c \
	treen.c putn.c yycopy.c \
	yycosts.c yyerror.c yyget.c yyid.c yylex.c yymain.c yyoptions.c \
	yypanic.c yyparse.c yyprint.c yyput.c yyrecover.c yyseman.c yytree.c

OBJS =	ato.o \
	call.o case.o clas.o const.o conv.o cset.o \
	error.o fdec.o func.o gen.o hash.o \
	lab.o lookup.o lval.o \
	main.o nl.o proc.o put.o \
	rec.o rval.o \
	stat.o string.o subr.o \
	tree.o type.o var.o Version.o \
	TRdata.o \
	treen.o putn.o yycopy.o \
	y.tab.o \
	yycosts.o yyerror.o yyget.o yyid.o yylex.o yymain.o yyoptions.o \
	yypanic.o yyparse.o yyprint.o yyput.o yyrecover.o yyseman.o yytree.o

OTHERS=	pas.y gram

MKSTR=/usr/ucb/mkstr
EYACC=eyacc
RM=-rm
GET=sccs get

CFLAGS =
LDFLAGS = -i
#FP =	-f			# used if host doesn't have floating point

TEMPDIR=tmp
ERRORSTRINGS=pi1.2strings

all: pi pix px_header.out

cp: pi pix px_header.out
	cp ${ERRORSTRINGS} ${DESTDIR}${LIBDIR}/${ERRORSTRINGS}
	cp how_pi ${DESTDIR}${LIBDIR}/how_pi
	cp how_pix ${DESTDIR}${LIBDIR}/how_pix
	dd if=px_header.out of=${DESTDIR}${LIBDIR}/npx_header conv=sync
	cp pi ${DESTDIR}${BINDIR}/pi
	cp pix ${DESTDIR}${BINDIR}/pix
	${RM} *.o ${TEMPDIR}/*.c
	${RM} y.tab.c y.tab.h
	${RM} ${ERRORSTRINGS}
	${RM} core px_header.out *.list obj

cmp: pi pix
	cmp pi ${DESTDIR}${BINDIR}/pi
	cmp pix ${DESTDIR}${BINDIR}/pix
	rm *.o

pix: pix.c
	cc -n -s -O -o pix pix.c

pi: ${OBJS} version
	./version > Version.c
	cc -o pi ${CFLAGS} ${FP} ${LDFLAGS} ${OBJS} Version.c

y.tab.h y.tab.c: pas.y
	${EYACC} pas.y >pas.eyacc.list
	ex - y.tab.c < gram

.c.o:
	rm -f ${TEMPDIR}/$*.c
	${MKSTR} - ${ERRORSTRINGS} ${TEMPDIR}/ $*.c
	cd ${TEMPDIR} ; ${CC} ${CFLAGS} -I.. -c $*.c ; mv $*.o ../$*.o
	rm -f ${TEMPDIR}/$*.c

version: version.c
	cc -o version version.c 

clean:
	${RM} *.o ${TEMPDIR}/*.c
	${RM} y.tab.c y.tab.h
	${RM} ${ERRORSTRINGS}
	${RM} core px_header.out *.list obj

px_header.out : px_header.c
	cc -O -o px_header.out px_header.c
	strip px_header.out

print :
	@pr makefile READ_ME
	@ls -ls | pr
	@pr picture
	@pr 0.h whoami main.c pas.y
	@pr yy.h yy*.c
	@pr [a-ln-x]*.c

BINDIR=/usr/bin
LIBDIR=/usr/lib

depend:
	grep '^#[ 	]*include' *.h \
		| sed '/<.*>/d' \
		| sed 's/\(.*\):[^"]*"\([^"]*\)".*/\1: \2/' >makedep
	grep '^#[ 	]*include' *.c \
		| sed '/<.*>/d' \
		| sed 's/:[^"]*"\([^"]*\)".*/: \1/' \
		| sed 's/\.c/.o/' >>makedep
	echo '/^# DO NOT DELETE THIS LINE/+2,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	cp Makefile Makefile.bak
	ed - Makefile < eddep
	rm eddep makedep
	echo '# DEPENDENCIES MUST END AT END OF FILE' >> Makefile
	echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >> Makefile
	echo '# see make depend above' >> Makefile

# DO NOT DELETE THIS LINE -- make depend uses it

yy.h: y.tab.h
TRdata.o: whoami
TRdata.o: 0.h
ato.o: whoami
ato.o: 0.h
call.o: whoami
call.o: 0.h
call.o: tree.h
call.o: opcode.h
case.o: whoami
case.o: 0.h
case.o: tree.h
case.o: opcode.h
clas.o: whoami
clas.o: 0.h
clas.o: tree.h
clas.o: opcode.h
const.o: whoami
const.o: 0.h
const.o: tree.h
conv.o: whoami
conv.o: 0.h
conv.o: opcode.h
cset.o: whoami
cset.o: 0.h
cset.o: tree.h
cset.o: opcode.h
error.o: whoami
error.o: 0.h
error.o: yy.h
fdec.o: whoami
fdec.o: 0.h
fdec.o: tree.h
fdec.o: opcode.h
func.o: whoami
func.o: 0.h
func.o: tree.h
func.o: opcode.h
gen.o: whoami
gen.o: 0.h
gen.o: tree.h
gen.o: opcode.h
hash.o: whoami
hash.o: 0.h
hash.o: yy.h
lab.o: whoami
lab.o: 0.h
lab.o: tree.h
lab.o: opcode.h
lookup.o: whoami
lookup.o: 0.h
lval.o: whoami
lval.o: 0.h
lval.o: tree.h
lval.o: opcode.h
main.o: whoami
main.o: 0.h
main.o: yy.h
nl.o: whoami
nl.o: 0.h
nl.o: opcode.h
proc.o: whoami
proc.o: 0.h
proc.o: tree.h
proc.o: opcode.h
put.o: whoami
put.o: opcode.h
put.o: 0.h
put.o: OPnames.h
rec.o: whoami
rec.o: 0.h
rec.o: tree.h
rec.o: opcode.h
rval.o: whoami
rval.o: 0.h
rval.o: tree.h
rval.o: opcode.h
stat.o: whoami
stat.o: 0.h
stat.o: tree.h
stat.o: opcode.h
string.o: whoami
string.o: 0.h
string.o: send.h
subr.o: whoami
subr.o: 0.h
tree.o: whoami
tree.o: 0.h
type.o: whoami
type.o: 0.h
type.o: tree.h
var.o: whoami
var.o: 0.h
var.o: tree.h
var.o: opcode.h
yycopy.o: 0.h
yycopy.o: yy.h
yycosts.o: whoami
yycosts.o: 0.h
yycosts.o: yy.h
yyerror.o: whoami
yyerror.o: 0.h
yyerror.o: yy.h
yyget.o: whoami
yyget.o: 0.h
yyget.o: yy.h
yyid.o: whoami
yyid.o: 0.h
yyid.o: yy.h
yylex.o: whoami
yylex.o: 0.h
yylex.o: yy.h
yymain.o: whoami
yymain.o: 0.h
yymain.o: yy.h
yyoptions.o: whoami
yyoptions.o: 0.h
yyoptions.o: yy.h
yypanic.o: whoami
yypanic.o: 0.h
yypanic.o: yy.h
yyparse.o: whoami
yyparse.o: 0.h
yyparse.o: yy.h
yyprint.o: whoami
yyprint.o: 0.h
yyprint.o: yy.h
yyput.o: whoami
yyput.o: 0.h
yyput.o: tree.h
yyput.o: yy.h
yyrecover.o: whoami
yyrecover.o: 0.h
yyrecover.o: yy.h
yyseman.o: whoami
yyseman.o: 0.h
yyseman.o: yy.h
yytree.o: whoami
yytree.o: 0.h
yytree.o: tree.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
