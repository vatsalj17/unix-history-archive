INCLUDE= /usr/include
CFLAGS= -O -I${INCLUDE}
OBJS=	sleep.o killpg.o setpgrp.o signal.o sigset.o wait2.o
NEWLIBC= read.o write.o ioctl.o
LIBC=	${DESTDIR}/lib/libc.a

all: libjobs.a newlibc

libjobs.a: ${OBJS}
# Sorry, OBJS must be listed here in order; tsort can't hack this
	ar cr libjobs.a sleep.o killpg.o setpgrp.o sigset.o \
	signal.o wait2.o

newlibc: ${NEWLIBC}
	@echo "New read, write and ioctl must be installed in C library"
	@echo "or libjobs.a.  See the READ_ME file."

cp: libjobs.a
	cp libjobs.a $(DESTDIR)/usr/lib
	ranlib $(DESTDIR)/usr/lib/libjobs.a

installlibc:
	ar r ${LIBC} ${NEWLIBC}
	ranlib ${LIBC}

clean:
	-rm *.o libjobs.a

libhack.a: sleep.o killpg.o setpgrp.o sigset.o sighack.o wait2.o
	ar cr libjobs.a sleep.o killpg.o setpgrp.o sigset.o \
	sighack.o wait2.o

sighack.o:
	as -o sighack.o ${INCLUDE}/sys.s signal.s read.s write.s ioctl.s

.s.o:
	as -o $*.o ${INCLUDE}/sys.s $*.s
