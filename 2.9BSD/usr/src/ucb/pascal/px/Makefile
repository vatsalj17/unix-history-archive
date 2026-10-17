#  PDP is set to 70 for separate I/D, 34 for nonseparate I/D with hardware
#  floating point, or 40 for nonseparate, no floating point.
PDP =	70

BINDIR= /usr/bin
AS=	as
CFLAGS=	-O -DHZ=60.0
FPTERP = ../utilities/fpnofetchi/fp.o
STARTUP= ../utilities/assubs/pcrt0.o

ASMS=	00int.s 02rel.s 02relset.s 03bool.s 04as.s 05lv.s  \
	06add.s 07sub.s 10mul.s 12div.s 13mod.s 14neg.s  \
	16dvd.s 17ind.s 17rv.s 20con.s 21rang.s 24case.s 24pxp.s 25set.s  \
	26for.s 27conv.s 30atof.s 30getname.s 30io.s 30iosubs.s 30read.s  \
	30write.s 34fun.s E.s opcode.s wait.s
OBJS=	exit.o int.o palloc.o pcttot.o pdattim.o perror.o pwrite.o \
	getc.o putc.o

all: px.${PDP}

cp: px.${PDP}
	cp px.${PDP} $DESTDIR}${BINDIR}/px
	rm px.${PDP} *.o

cmp: px.${PDP}
	cmp px.${PDP} $DESTDIR}${BINDIR}/px
	rm *.o

px.70:	as.o ${OBJS} ${STARTUP}
	ld -X -i -o px.70 ${STARTUP} as.o ${OBJS} -lm -lc

px.34: as.o ${OBJS} ${STARTUP}
	ld -n -X -o px.34 ${STARTUP} as.o ${OBJS} -lm -lc

# This used to be loaded -n; to gain almost 8K of data space, omit -n.
px.40: nofp_as.o ${OBJS} ${STARTUP} ${FPTERP}
	ld -X -o px.40 ${STARTUP} nofp_as.o ${OBJS}
	${FPTERP} -lm -lc

getc.o: getc.s
	as -o getc.o /usr/include/sys.s getc.s
putc.o: putc.s
	as -o putc.o /usr/include/sys.s putc.s
as.o:	${ASMS}
	rm -f 00head.s
	echo 'FP = 1' > 00head.s
	rm -f as.o
	${AS} -o as.o /usr/include/sys.s 00head.s ${ASMS}
nofp_as.o: ${ASMS}
	rm -f 00head.s
	echo 'FP = 0' > 00head.s
	rm -f as.o
	${AS} -o as.o /usr/include/sys.s 00head.s ${ASMS}
int.c palloc.c pcttot.c perror.c: E.h
E.s E.h: Edata
	ed - Edata < Emake
../utilities/assubs/pcrt0.o: ../utilities/assubs/pcrt0.s
	cd ../utilities/assubs; make pcrt0.o
../utilities/fpterp/fp.o: ../utilities/fpterp/fp*.s
	cd ../utilities/fpterp; make fp.o
../utilities/fpnofetchi/fp.o: ../utilities/fpnofetchi/fp*.s
	cd ../utilities/fpnofetchi; make fp.o
clean:
	rm a.out core E.h E.s 00head.s
	rm *.o
print:
	@pr makefile README
	@ls -ls | pr
	@pr ../opcodes/picture
	@pr 0x.h int.c exit.c
	@pr [0-9][0-9]*.s
	@pr p*.c
install:
	cp a.out /usr/bin/px
	chmod 711 /usr/bin/px
installNOID:
	cp a.outNOID /usr/bin/px
	chmod 711 /usr/bin/px
