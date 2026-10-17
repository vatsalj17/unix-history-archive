#	Makefile	4.5	83/07/09
#
ALL=	libplot libt4014 libt300 libt300s libt450 libvt0
SUBDIRS=plot t4014 t300 t300s t450 vt0

all:	${ALL}

libplot: FRC
	cd plot; make

libt4014: FRC
	cd t4014; make

libt300: FRC
	cd t300; make

libt300s: FRC
	cd t300s; make

libt450: FRC
	cd t450; make

libvt0: FRC
	cd vt0; make

FRC:

cp:	all
	for i in ${ALL}; do \
		cp $$i ${DESTDIR}/usr/lib/$$i.a; \
		ranlib ${DESTDIR}/usr/lib/$$i.a; \
	done

clean:
	rm -f ${ALL} errs a.out core
	for i in ${SUBDIRS}; do \
		(cd $$i; make clean); \
	done
