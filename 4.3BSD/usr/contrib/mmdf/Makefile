#	MMDF Root Makefile
#
#
SUBDIR=	lib src uip

all:
	for dir in ${SUBDIR}; do \
		(cd $${dir}; make ${MFLAGS} -${MAKEFLAGS}); done

depend:
	for dir in ${SUBDIR}; do \
		(cd $${dir}; make ${MFLAGS} -${MAKEFLAGS} depend); done

lint:
	for dir in ${SUBDIR}; do \
		(cd $${dir}; make ${MFLAGS} -${MAKEFLAGS} lint); done

install:
	for dir in ${SUBDIR}; do \
		(cd $${dir}; make ${MFLAGS} -${MAKEFLAGS} install); done

clean:
	-rm -f make.out
	for dir in ${SUBDIR}; do \
		(cd $${dir}; make clean); done
