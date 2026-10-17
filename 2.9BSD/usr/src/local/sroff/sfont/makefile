OBJS=	H10.o H8.o M12.o M10.o M12I.o

all:	$(OBJS)

cp:	all
	strip *.o
	for i in *.o; do cp $$i /usr/lib/sfont/`basename $$i .o`; done
	rm *.o

cmp:	all
	strip *.o
	for i in *.o; do cmp $$i /usr/lib/sfont/`basename $$i .o`; done
	rm *.o

$(OBJS):	../sdef.h Special
