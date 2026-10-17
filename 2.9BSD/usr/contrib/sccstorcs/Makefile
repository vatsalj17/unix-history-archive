SUBDIR	      = src man/man8

all:		${SUBDIR}

${SUBDIR}:	FRC
		cd $@; make ${MFLAGS}

cp:
	for i in ${SUBDIR}; do \
		(cd $$i; make ${MFLAGS} DESTDIR=${DESTDIR} cp); done

clean:
	for i in ${SUBDIR}; do (echo $$i; cd $$i; make ${MFLAGS} clean); done

depend:
	for i in ${SUBDIR}; do (echo $$i; cd $$i; make ${MFLAGS} depend); done

FRC:
