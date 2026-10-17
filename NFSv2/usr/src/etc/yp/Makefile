#
# @(#)Makefile 1.1 85/05/30 (C) 1984 SMI
#
SRCS = makedbm.c stdhosts.c yppush.c
CFLAGS = -O
TOUCH =	touch -cf
SCRIPT= ypinit
LIBS = -ldbm

all: makedbm yppush stdhosts

makedbm: makedbm.o
	cc $(CFLAGS) makedbm.o -o makedbm $(LIBS)

yppush: yppush.o
	cc $(CFLAGS) yppush.o -o yppush $(LIBS)

stdhosts: stdhosts.o
	cc $(CFLAGS) stdhosts.o -o stdhosts

install: all
	-mkdir ${DESTDIR}/etc && chown bin ${DESTDIR}/etc && \
		chmod 755 ${DESTDIR}/etc
	-mkdir ${DESTDIR}/etc/yp && chown bin ${DESTDIR}/etc/yp && \
		chmod 755 ${DESTDIR}/etc/yp
	install -s makedbm $(DESTDIR)/etc/yp
	install -s yppush $(DESTDIR)/etc/yp
	install -s stdhosts $(DESTDIR)/etc/yp
	rm -f $(DESTDIR)/etc/yp/yppull
	ln -s yppush $(DESTDIR)/etc/yp/yppull
	rm -f $(DESTDIR)/etc/yp/yppoll
	ln -s yppush $(DESTDIR)/etc/yp/yppoll
	for i in ${SCRIPT}; do (install -c $$i.sh ${DESTDIR}/etc/yp/$$i); done
	install -c make.script ${DESTDIR}/etc/yp/Makefile
clean:
	rm -f *.o makedbm yppush stdhosts

lint:
	lint $(SRCS)

print:
	pr $(SRCS) | lpr

depend:
	cp /dev/null x.c
	for i in $(SRCS) ; do \
		(/bin/grep '^#[ 	]*include' x.c $$i | sed \
			-e '/\.\.\/h/d' \
			-e '/\.\.\/ufs/d' \
			-e 's,<\(.*\)>,"/usr/include/\1",' \
			-e 's/:[^"]*"\([^"]*\)".*/: \1/' \
			-e 's/\.c/\.o/' >>makedep); done
	echo '/^# DO NOT DELETE THIS LINE/+2,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	cp Makefile Makefile.bak
	ed - Makefile < eddep
	rm eddep makedep x.c
	echo '# DEPENDENCIES MUST END AT END OF FILE' >> Makefile
	echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >> Makefile
	echo '# see make depend above' >> Makefile

# DO NOT DELETE THIS LINE -- make depend uses it

makedbm.o: /usr/include/dbm.h
makedbm.o: /usr/include/stdio.h
makedbm.o: /usr/include/sys/file.h
makedbm.o: /usr/include/sys/param.h
makedbm.o: /usr/include/sys/stat.h
stdhosts.o: /usr/include/stdio.h
yppush.o: /usr/include/dbm.h
yppush.o: /usr/include/stdio.h
yppush.o: /usr/include/sys/time.h
yppush.o: /usr/include/ctype.h
yppush.o: /usr/include/netdb.h
yppush.o: /usr/include/rpc/rpc.h
yppush.o: /usr/include/sys/socket.h
yppush.o: /usr/include/rpcsvc/ypclnt.h
yppush.o: /usr/include/rpcsvc/yp_prot.h
makedbm.o: /usr/include/dbm.h
makedbm.o: /usr/include/stdio.h
makedbm.o: /usr/include/sys/file.h
makedbm.o: /usr/include/sys/param.h
makedbm.o: /usr/include/sys/stat.h
stdhosts.o: /usr/include/stdio.h
yppush.o: /usr/include/dbm.h
yppush.o: /usr/include/stdio.h
yppush.o: /usr/include/sys/time.h
yppush.o: /usr/include/ctype.h
yppush.o: /usr/include/netdb.h
yppush.o: /usr/include/rpc/rpc.h
yppush.o: /usr/include/sys/socket.h
yppush.o: /usr/include/rpcsvc/ypclnt.h
yppush.o: /usr/include/rpcsvc/yp_prot.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
