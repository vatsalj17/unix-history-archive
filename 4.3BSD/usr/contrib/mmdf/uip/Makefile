#	UIP Subtree Makefile
#
#
# Programs that live in subdirectories, and have makefiles of their own.
#
SUBDIR=	msg send other # ucbmail

all:
	for dir in ${SUBDIR}; do \
		(cd $${dir}; gen ${MFLAGS} -${MAKEFLAGS}); done

install:
	for dir in ${SUBDIR}; do \
		(cd $${dir}; gen ${MFLAGS} -${MAKEFLAGS} install); done

dist:
	-touch `ls */x*|grep -v '\.'`
	echo echo UIP Distribution made `date` > install.sh
	for dir in ${SUBDIR}; do \
		echo cd $${dir} >> install.sh; \
		( cd $${dir}; \
		  gen -n ${MFLAGS} -${MAKEFLAGS} install) >> install.sh; \
		echo cd .. >> install.sh; \
	done
	tar cvf uipdist.tar install.sh `ls */x*|grep -v '\.'` ucbmail/misc/Mail.help*

depend:
	for dir in ${SUBDIR}; do \
		(cd $${dir}; gen ${MFLAGS} -${MAKEFLAGS} depend); done

lint:
	for dir in ${SUBDIR}; do \
		(cd $${dir}; gen ${MFLAGS} -${MAKEFLAGS} lint); done

clean:
	-rm -f make.out
	for dir in ${SUBDIR}; do \
		(cd $${dir}; make -f Makefile.real clean); done
