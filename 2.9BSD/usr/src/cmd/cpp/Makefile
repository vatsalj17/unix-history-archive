CC=		cc
CFLAGS=		-O -DFLEXNAMES=1 ${INCLUDES}
LDFLAGS=	-n

all:		cpp

cp:		cpp
		-strip cpp
		cp cpp ${DESTDIR}/lib
		rm *.o cpp

cmp:		cpp
		cmp cpp ${DESTDIR}/lib/cpp
		rm *.o

cpp:		cpp.o cpy.o
		${CC} ${LDFLAGS} ${CFLAGS} $@.o cpy.o -o $@

cpp.o:		cpp.c
		${CC} ${CFLAGS} -c $<

cpy.o:		cpy.y yylex.c

clean:
		rm -f cpp.o cpy.o
