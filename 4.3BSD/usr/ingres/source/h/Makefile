#
#  Makefile for INGRES header files
#
#	@(#)Makefile	8.3	12/18/85
#

SRCS=	access.h aux.h batch.h catalog.h func.h errors.h\
	ingres.h lock.h opsys.h pv.h range.h resp.h \
	symbol.h trace.h tree.h useful.h version.h \
	pmon.h btree.h depend.sed

GET=	sccs get
REL=
PR=	print
VGRIND=	csh /usr/ucb/vgrind
HEADER=	Header Files

sources: $(SRCS)

$(SRCS):
	$(GET) $(REL) $@

install: sources

new:

print: sources
	$(PR) Makefile *.[hm] *.[csy]

vgrind: sources
	$(VGRIND) -h "$(HEADER) (Release 8)" -n Makefile
	$(VGRIND) -h "$(HEADER) (Release 8)" *.[hm]

clean:
	rm -f *.o
	rm -f core a.out

tags: sources
	-fgrep ../ tags > othertags
	ctags  $(SRCS) $(HDRS)
	sort -o tags tags othertags
	-rm -f othertags

#
# DANGER	DANGER	DANGER	DANGER	DANGER	DANGER
# The following two entries should NOT be executed by the user.
# These entries are only for the Makefile in conf to construct
# the system tag files.
#
systags: 

#
# We assume that systags has already been done.
#
installtags:
	cp /dev/null tags
	sed "/	h/d" ../tags | sed "s/	/	..\//" > othertags
	sort -o tags tags othertags
	-rm -f othertags systags

