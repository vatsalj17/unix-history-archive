#
DESTDIR=
CFLAGS=	-O

# Programs that live in subdirectories, and have makefiles of their own.
#
SUBDIR=	cal kermit print ramdisk

# C programs that live in the current directory 
#
CUR=	grow.rsrc macscrawl.rsrc insane.rsrc alert.rsrc \
	mfinder.rsrc testprint.rsrc boot.rsrc

all:	${SUBDIR} ${CUR}

cur:	${CUR}

${SUBDIR}: /tmp
	cd $@; make ${MFLAGS}

.SUFFIXES: .rsrc .b

.c.b:
	cc68 -c -I../h $<

.s.b:
	cc68 -c $<

.b.rsrc:
	cc68 -z -m $<
	rmaker $*.rc

grow.rsrc: grow.b grow.rc
macscrawl.rsrc: macscrawl.b macscrawl.rc
insane.rsrc: insane.b insane.rc
alert.rsrc: alert.b alert.rc
mfinder.rsrc: mfinder.b mfinder.rc
testprint.rsrc: testprint.b testprint.rc
boot.rsrc: boot.b boot.rc

clean:
	rm -f a.out core *.s *.o *.b z
	for i in ${SUBDIR}; do (cd $$i; make -i ${MFLAGS} clean); done
	rm -f ${CUR}
