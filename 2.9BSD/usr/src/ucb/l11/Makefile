CFILES=		link.c pass2.c in.c sup.c 
OBJS=		link.o pass2.o in.o sup.o 
CFLAGS=		-O
LFLAGS=		-n -s

all: 		l11

cp:		l11
		cp l11 $(DESTDIR)/usr/ucb/l11

cmp:		l11
		cmp l11 $(DESTDIR)/usr/ucb/l11

l11:		$(OBJS)
		cc -o l11 $(LFLAGS) link.o pass2.o in.o sup.o

lint:	
		lint link.c pass2.c in.c sup.c > FLUFF

obint:		obint.o in.o sup.o
		cc -o obint obint.o in.o sup.o

tags:		$(CFILES)
		ctags $(CFILES)

clean:
		rm -f *.o obint l11

list:
		pr link.h link.c pass2.c in.c sup.c obint.c | lpr
