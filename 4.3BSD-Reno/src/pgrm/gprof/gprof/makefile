#
# Copyright (c) 1983 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)makefile	5.1 (Berkeley) 10/14/86
#
DESTDIR= 
MACHINE=`machine`

.c.o:
	make -f Makefile ${MFLAGS} MACHINE=${MACHINE} $*.o

all:	FRC
	make -f Makefile ${MFLAGS} MACHINE=${MACHINE}

FRC:

install:
	make -f Makefile ${MFLAGS} MACHINE=${MACHINE} install

clean:
	make -f Makefile ${MFLAGS} MACHINE=${MACHINE} clean
