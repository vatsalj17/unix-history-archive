# makefile for sroff
# defines:
#	-DTTY	for printer connected to user's terminal line
#		(does stty differently)
#		if not defined, uses /dev/sanders
#
#	-DPGOFF uses the sanders to set the page offset rather than
#		prepending space.  This speeds output and makes left margin
#		control more transparent but cannot be used
#		with a col-like postprocessor
#	-DADJ	uses sanders for all line adjusting.  This does not
#		work quite right (both sroff and the printer have quirks).
#		PGOFF must also be defined.
#
#	-DVMUNIX	uses larger tables for VAX/UNIX, also
#			longer strings as macro args.
#
#	-DBIG=x		larger temporary file, x > 512
#			allows 65K characters per 512

CFLAGS= -O -DPGOFF -DBIG=768
#FP = -f
LFLAGS= $(FP)

DFLT=H10.o		# default sfont set
SFILES=s1.o s2.o s3.o s4.o s5.o s6.o s7.o s8.o s9.o s10.o si.o sii.o \
	stab.o hytab.o suftab.o tabsand.o sfont/$(DFLT)

sroff:	$(SFILES)
	cc -o sroff $(LFLAGS) $(SFILES)


s1.o:	sdef.h d.h v.h tw.h s.h
s2.o:	sdef.h d.h v.h tw.h s.h
s3.o:	sdef.h d.h v.h tw.h s.h
s4.o:	sdef.h d.h v.h tw.h s.h
s5.o:	sdef.h d.h v.h tw.h s.h
s6.o:	sdef.h d.h v.h tw.h s.h
s7.o:	sdef.h d.h v.h tw.h s.h
s8.o:	sdef.h d.h v.h tw.h s.h
s9.o:	sdef.h d.h v.h tw.h s.h
s10.o:	sdef.h d.h v.h tw.h s.h
si.o:	sdef.h d.h v.h tw.h s.h
sii.o:	sdef.h d.h v.h tw.h s.h
tabsand.o:	sdef.h
sfont/$(DFLT):	sdef.h
	cd sfont; make $(DFLT)

hytab.o:	hytab.c
	cc -S hytab.c
	ed hytab.s <textscript
	as -o hytab.o hytab.s
	rm hytab.s

suftab.o:	suftab.c
	cc -S suftab.c
	ed suftab.s <textscript
	as -o suftab.o suftab.s
	rm suftab.s

.c.o:
	cc -c $(CFLAGS) $*.c
