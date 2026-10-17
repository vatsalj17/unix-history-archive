#	Makefile	4.2	83/08/15
#
# Possible options:
#	make		compile source for terminal tables below
#	make <term>	compile table for a specific <term>
#	make cp		move tables to ${DEST}
#	make links	make links for ease of documentation
#	make clean	remove tab*.o files lying around
ALL= tab37.o tablpr.o tab300.o tab300-12.o tab302.o tab302-12.o tab382.o \
	tab382-12.o tab450.o tab450-12.o tab833.o tab833-12.o tabitoh.o \
	tabitoh12.o tabnec.o tabnec12.o tabnec-t.o tabqume.o tabqume12.o \
	tabxerox.o tabxerox12.o tabx-ecs.o tabx-ecs12.o tab450-12-8.o \
	taba1.o
DESTDIR=
DEST=	${DESTDIR}/usr/lib/term

all:	${ALL}

tab37.o:	tab37.c
tablpr.o:	tablpr.c
tab300.o:	tab300.c code.300
tab300-12.o:	tab300-12.c code.300
tab302.o:	tab302.c code.300
tab302-12.o:	tab302-12.c code.300
tab382.o:	tab382.c code.300
tab382-12.o:	tab382-12.c code.300
tab450.o:	tab450.c code.300
tab450-12.o:	tab450-12.c code.300
tab450-12-8.o:	tab450-12-8.c code.300
tab833.o:	tab833.c code.aj833
tab833-12.o:	tab833-12.c code.aj833
taba1.o:	taba1.c code.300
tabitoh.o:	tabitoh.c code.itoh
tabitoh12.o:	tabitoh12.c code.itoh
tabnec.o:	tabnec.c code.nec
tabnec12.o:	tabnec12.c code.nec
tabnec-t.o:	tabnec-t.c
tabqume.o:	tabqume.c
tabqume12.o:	tabqume12.c
tabxerox.o:	tabxerox.c code.xerox
tabxerox12.o:	tabxerox12.c code.xerox
tabx-ecs.o:	tabx-ecs.c code.x-ecs
tabx-ecs12.o:	tabx-ecs12.c code.x-ecs

.c.o:
	cc -c $<; strip $@

cp:	all
	for i in ${ALL}; do cp $$i ${DEST}/`basename $$i .o`; done
	rm *.o
	-if test ! -f ${DEST}/tabcrt; \
	then \
		make links; \
	fi
	cp README ${DEST}/README; chmod 444 ${DEST}/README

cmp:	all
	for i in ${ALL}; do cmp $$i ${DEST}/`basename $$i .o`; done

links:
	rm -f ${DEST}/tabtn300; ln ${DEST}/tablpr ${DEST}/tabtn300
	rm -f ${DEST}/tabcrt; ln ${DEST}/tablpr ${DEST}/tabcrt
	rm -f ${DEST}/tabdumb; ln ${DEST}/tablpr ${DEST}/tabdumb
	rm -f ${DEST}/tab300s; ln ${DEST}/tab302 ${DEST}/tab300s
	rm -f ${DEST}/tab300s-12; ln ${DEST}/tab302-12 ${DEST}/tab300s-12
	rm -f ${DEST}/tabdtc; ln ${DEST}/tab302 ${DEST}/tabdtc
	rm -f ${DEST}/tabdtc12; ln ${DEST}/tab302-12 ${DEST}/tabdtc12
	rm -f ${DEST}/tabipsi; ln ${DEST}/tab450 ${DEST}/tabipsi
	rm -f ${DEST}/tabipsi12; ln ${DEST}/tab450-12 ${DEST}/tabipsi12

clean:
	rm -f tab*.o errs
