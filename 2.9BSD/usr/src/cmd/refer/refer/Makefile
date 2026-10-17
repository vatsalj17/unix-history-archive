CFLAGS = -O -s -DMFLAG

CC= cc
all:	refer

cp:	all
	cp refer ${DESTDIR}/usr/bin
	rm refer *.o

cmp:	all
	cmp refer ${DESTDIR}/bin/refer
	rm *.o

refer0.o: refer.h
refer1.o: refer.h
refer2.o: refer.h
refer3.o: refer.h
refer4.o: refer.h
refer6.o: refer.h
refer7.o: refer.h
refer8.o: refer.h
refer: glue1.o refer1.o refer2.o refer4.o refer5.o refer6.o mkey3.o
refer: refer7.o refer8.o hunt2.o hunt3.o deliv2.o hunt6.o hunt8.o
refer: hunt7.o glue2.o glue4.o glue5.o refer0.o shell.o 
	${CC} -i -o refer refer[0124678].o glue?.o refer5.o \
		hunt[23678].o mkey3.o shell.o deliv2.o
	size refer

clean:
	rm -f *.o
