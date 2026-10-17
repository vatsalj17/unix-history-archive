#	%M%	%I%	%E%
#
DESTDIR=
ALL=	AW anovaws aw base editws fftws getws gradesys \
	graph2ws graphws iows news ngradesys plotws shapews

all:	;

cp:	all
	for i in ${ALL}; do \
		cp $$i ${DESTDIR}/usr/lib/new/apl/$$i ; \
		chmod 644 $$i ; \
		;; \
	done

clean:	;
