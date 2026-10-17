#
#	makefile	4.1	10/9/80
#
# C Shell with process control; PDP-11 separate I/D Makefile
# Bill Joy UC Berkeley; Jim Kulp IIASA, Austria
#
CFLAGS=	-O
XSTR=	/usr/ucb/xstr
ED=	-ed
AS=	-as
RM=	-rm
CXREF=	/usr/ucb/cref
VGRIND=	csh /usr/ucb/vgrind
CTAGS=	/usr/ucb/ctags
LIBES=	-ljobs

# strings.o must be last in OBJS since it can change when previous files compile
OBJS=	sh.o sh.dol.o sh.err.o sh.exec.o sh.exp.o sh.func.o sh.glob.o \
	sh.hist.o sh.lex.o sh.misc.o sh.parse.o sh.print.o sh.sem.o sh.set.o \
	sh.proc.o sh.dir.o sh.time.o alloc.o sh.init.o printf.o \
	doprnt.o strings.o

all: csh

cp: csh
	-rm -f $DESTDIR/bin/csh $DESTDIR/bin/oldcsh
	cp csh $DESTDIR/bin/csh
	ln $DESTDIR/bin/csh $DESTDIR/bin/oldcsh
	rm csh *.o

cmp: csh
	cmp csh $DESTDIR/bin/csh
	rm *.o

csh: ${OBJS} sh.local.h
	rm -f csh
	cc -i -s ${OBJS} -o csh ${LIBES}

# Special massaging of C files for sharing of strings
.c.o:
	${CC} -E ${CFLAGS} $*.c | ${XSTR} -c -
	${CC} -c ${CFLAGS} x.c 
	mv x.o $*.o

strings.o: strings
	${XSTR}
	${CC} -c xs.c
	mv xs.o strings.o
	${RM} xs.s

csh.prof: ${OBJS} sh.prof.o sh.local.h mcrt0.o
	rm -f csh.prof
	ld -X mcrt0.o ${OBJS} -o csh.prof ${LIBES} -lc

sh.o.prof:
	cp sh.c sh.prof.c
	cc -c ${CFLAGS} -DPROF sh.prof.c

lint:
	lint ${CFLAGS} sh*.c

print:
	@pr READ_ME
	@pr makefile makefile.*
	@(size -l a.out; size *.o) | pr -h SIZES
	@ls -l | pr 
	@pr sh*.h sh*.c alloc.c

vprint:
	@pr -l132 makefile 
	@(size -l a.out; size *.o) | pr -l84 -h SIZES
	@ls -l | pr -l132
	@pr -l132 sh*.h [a-rt-z]*.h sh*.c alloc.c

vgrind:
	@cp /dev/null index
	@for i in *.h; do vgrind -t -h "C Shell" $$i >/crp/bill/csh/$$i.t; done
	@for i in *.c; do vgrind -t -h "C Shell" $$i >/crp/bill/csh/$$i.t; done
	@vgrind -t -x -h Index index >/crp/bill/csh/index.t

install: csh sh.local.h
	install -s csh ${DESTDIR}/bin/csh

clean:
	${RM} -f a.out strings x.c xs.c
	${RM} -f *.o sh.prof.c

tags:	/tmp
	${CTAGS} sh*.c
