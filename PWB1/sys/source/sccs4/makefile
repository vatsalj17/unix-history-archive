CC = cc
CFLAGS = -O
LDFLAGS = -s -n
LIBES = -lpw -lPW -lS -lwrt


SCCS:		cmd/admin cmd/bdiff cmd/chghist cmd/comb cmd/scv \
	cmd/delta cmd/get cmd/help cmd/prt cmd/rmdel cmd/sccsdiff \
	cmd/what

cmd/admin:	cmd/admin.c com/comobj.a hdr/*.h
	chdir cmd; $(CC) $(CFLAGS) $(LDFLAGS) -o admin admin.c \
		../com/comobj.a $(LIBES)

cmd/bdiff:	cmd/bdiff.c com/comobj.a hdr/*.h
	chdir cmd; $(CC) $(CFLAGS) $(LDFLAGS) -o bdiff bdiff.c \
		../com/comobj.a $(LIBES)

cmd/comb:	cmd/comb.c com/comobj.a hdr/*.h
	chdir cmd; $(CC) $(CFLAGS) $(LDFLAGS) -o comb comb.c \
		../com/comobj.a $(LIBES)

cmd/scv:	cmd/scv.c com/comobj.a hdr/*.h
	chdir cmd; $(CC) $(CFLAGS) $(LDFLAGS) -o scv scv.c \
		../com/comobj.a $(LIBES)

cmd/delta:	cmd/delta.c com/comobj.a hdr/*.h
	chdir cmd; $(CC) $(CFLAGS) $(LDFLAGS) -o delta delta.c \
		../com/comobj.a $(LIBES)

cmd/get:	cmd/get.c com/comobj.a hdr/*.h
	chdir cmd; $(CC) $(CFLAGS) $(LDFLAGS) -o get get.c \
		../com/comobj.a $(LIBES)

cmd/help:	cmd/help.c com/comobj.a hdr/*.h
	chdir cmd; $(CC) $(CFLAGS) $(LDFLAGS) -o help help.c \
		../com/comobj.a $(LIBES)

cmd/prt:	cmd/prt.c com/comobj.a hdr/*.h
	chdir cmd; $(CC) $(CFLAGS) $(LDFLAGS) -o prt prt.c \
		../com/comobj.a $(LIBES)

cmd/chghist:	cmd/rmdel
	-rm -f cmd/chghist
	ln cmd/rmdel cmd/chghist

cmd/rmdel:	cmd/rmdel.c com/comobj.a hdr/*.h
	chdir cmd; $(CC) $(CFLAGS) $(LDFLAGS) -o rmdel rmdel.c \
		../com/comobj.a $(LIBES)

cmd/what:	cmd/what.c com/comobj.a hdr/*.h
	chdir cmd; $(CC) $(CFLAGS) $(LDFLAGS) -o what what.c \
		../com/comobj.a $(LIBES) $(LIBES)

com/comobj.a:	com/*.c hdr/*.h
	chdir com; $(CC) $(CFLAGS) -c *.c
	-chdir com; rm -f tmp.a
	chdir com; ar r\
	tmp.a\
	doie.o\
	dolist.o\
	permiss.o\
	rdmod.o\
	setup.o\
	dodelt.o\
	del_ab.o\
	chksid.o\
	stats_ab.o\
	flushto.o\
	del_ba.o\
	newstats.o\
	sinit.o\
	getline.o\
	putline.o\
	auxf.o\
	sid_ba.o\
	date_ba.o\
	dofile.o\
	pf_ab.o\
	sid_ab.o\
	date_ab.o\
	fmterr.o\
	sidtoser.o\
	chkid.o\
	dohist.o\
	eqsid.o
	chdir com; mv tmp.a comobj.a
	chdir com; rm -f *.o
