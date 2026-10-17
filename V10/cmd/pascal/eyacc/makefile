D = /usr/bin/eyacc
CFLAGS = -O

eyacc: ey0.o ey1.o ey2.o ey3.o ey4.o ey5.o
	cc -o eyacc ey0.o ey1.o ey2.o ey3.o ey4.o ey5.o

clean:
	rm -f *.o eyacc

install: eyacc
	rm -f $D
	strip eyacc
	cp eyacc $D
	chown bin $D
	chgrp bin $D
	chmod o-w,g+w $D

ey1.o:	ey.h
ey2.o:	ey.h
ey3.o:	ey.h
ey4.o:	ey.h
