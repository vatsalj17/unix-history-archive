#	Makefile	4.3	83/07/21
#
DESTDIR=
CFLAGS=	-O

# Programs that live in subdirectories, and have makefiles of their own.
#
SUBDIR=	ansitar apl cpm getsyspr jove mh news notes rcs sccstorcs

all:	${SUBDIR}

${SUBDIR}: FRC
	cd $@; make ${MFLAGS}

cp:
	for i in ${SUBDIR}; do \
		(cd $$i; make ${MFLAGS} DESTDIR=${DESTDIR} cp); done

clean:
	for i in ${SUBDIR}; do (echo $$i; cd $$i; make ${MFLAGS} clean); done

FRC:
