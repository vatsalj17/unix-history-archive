#       Makefile for the Fortran 77 compiler
#       running on a separate I/D PDP11,
#       generating code for the PDP11,
#       using the Ritchie pass 2

CFL=		-DTARGET=PDP11 -DFAMILY=DMR -DHERE=PDP11 -DOUTPUT=BINARY \
		-DPOLISH=POSTFIX
FP=		# -f for machines without floating point
CFLAGS=		-O ${FP} ${CFL} 
LDFLAGS=	-i -s
PASS1OBJECTS=	data.o equiv.o error.o exec.o expr.o gram.o init.o \
		intr.o io.o lex.o main.o misc.o pdp11.o pdp11x.o proc.o \
		put.o putdmr.o xs.o

all:    	f77 f77pass1

cp:     	all
		cp f77 ${DESTDIR}/bin
		cp f77pass1 ${DESTDIR}/usr/lib
		-strip ${DESTDIR}/bin/f77 ${DESTDIR}/lib/f77pass1

cmp:    	all
		cmp f77 ${DESTDIR}/bin/f77
		cmp f77pass1 ${DESTDIR}/usr/lib/f77pass1

clean:
		-rm -f f77 f77pass1 *.o x.c xs.c f77_strings strings

f77:		driver.c pdp11x.c
		${CC} ${CFLAGS} -n -s driver.c pdp11x.c -o f77
		@size f77

f77pass1:	${PASS1OBJECTS}
		${CC} ${LDFLAGS} ${PASS1OBJECTS} -o f77pass1
		@size f77pass1

gram.c:		gram.head gram.dcl gram.expr gram.exec gram.io tokdefs
		( sed <tokdefs "s/#define/%token/" ;\
		cat gram.head gram.dcl gram.expr gram.exec gram.io ) >gram.in
		@echo expect 2 shift/reduce conflicts
		${YACC} ${YFLAGS} gram.in
		mv y.tab.c gram.c
		rm gram.in

.c.o:
		${CC} ${CFL} -E $*.c | xstr -c -
		${CC} ${CFLAGS} -c x.c
		mv x.o $*.o

intr.o:		intr.c
		${CC} ${CFLAGS} -c intr.c

xs.o:		strings
		xstr
		${CC} ${CFLAGS} -c xs.c

data.o:		defs
data.o:		string_defs
driver.o:	/usr/include/stdio.h
driver.o:	/usr/include/ctype.h
driver.o:	defines
driver.o:	locdefs
driver.o:	drivedefs
driver.o:	ftypes
driver.o:	/usr/include/signal.h
driver.o:	/usr/include/errno.h
driver.o:	ftypes
equiv.o:	defs
equiv.o:	string_defs
error.o:	defs
error.o:	string_defs
exec.o:		defs
exec.o:		string_defs
expr.o:		defs
expr.o:		string_defs
gram.o:		defs
gram.o:		string_defs
init.o:		defs
init.o:		string_defs
intr.o:		defs
intr.o:		string_defs
io.o:		defs
io.o:		string_defs
lex.o:		defs
lex.o:		string_defs
lex.o:		tokdefs
main.o:		defs
main.o:		string_defs
misc.o:		defs
misc.o:		string_defs
pdp11.o:	defs
pdp11.o:	string_defs
pdp11x.o:	/usr/include/stdio.h
pdp11x.o:	defines
pdp11x.o:	locdefs
proc.o:		defs
proc.o:		string_defs
put.o:		defs
put.o:		string_defs
putdmr.o:	defs
putdmr.o:	string_defs
putdmr.o:	dmrdefs

defs:		/usr/include/stdio.h /usr/include/ctype.h ftypes defines locdefs
		touch defs
	
tokdefs:	tokens
		grep -n . <tokens | sed "s/\([^:]*\):\(.*\)/#define \2 \1/" >tokdefs

locdefs:	pdp11defs
		cp pdp11defs locdefs
