Dir=

RHDRS=	../h/rt.h ../h/config.h ../h/cpuconf.h ../h/memsize.h
#
#  To add or replace object files, add their names to the OBJS list below.
#  For example, to add nfncs.o and iolib.o, use:
#
#	OBJS=nfncs.o iolib.o         # this is a sample line
#
#  For each object file added to OBJS, add a dependency line to reflect files
#  that are depended on.  In general, new functions depend on $(RHDRS).
#  For example, if nfncs.c contains new functions, use
#  	
#	nfncs.o:	$(RHDRS)
#

OBJS=
LIB=

RTOBJS=../std/rconv.o ../std/idata.o $(OBJS)

Pi:	../picont ../piconx ../pilink ../piconx.hdr

../picont: ../std/icont.c ../h/config.h
	rm -f ../picont picont
	$(CC) $(CFLAGS) -o ../picont -DItran="\"$(Dir)/pitran\""\
 		-DIconx="\"$(Dir)/piconx\"" \
		-DIlink="\"$(Dir)/pilink\""  ../std/icont.c
	strip ../picont
	ln ../picont picont

../pilink: ../std/linklib ../std/builtin.o ../std/ilink.o
	$(CC) $(LDFLAGS) -o ../pilink ../std/builtin.o ../std/ilink.o\
		 ../std/linklib
	strip ../pilink

../std/ixhdr.o:	../h/config.h
	cd ../std;	$(CC) -c $(XCFLAGS) -DIconx="\"$(Dir)/piconx\"" ixhdr.c

../piconx.hdr:	../std/ixhdr.o
	$(CC) $(XLDFLAGS) ../std/ixhdr.o -o ../piconx.hdr
	strip ../piconx.hdr

../piconx: ../std/rtlib $(RTOBJS)
	$(CC) $(LDFLAGS) -o ../piconx $(RTOBJS) ../std/rtlib $(LIB) -lm
	
../std/idata.o:		$(RHDRS) ../h/fdef.h
	cd ../std;	$(CC) -c $(CFLAGS) idata.c

../std/rconv.o:		$(RHDRS) ../h/fdef.h
	cd ../std;	$(CC) -c $(CFLAGS) rconv.c

../std/builtin.o:	../std/ilink.h ../h/config.h ../h/fdef.h
	cd ../std;	$(CC) -c $(CFLAGS) builtin.c

../std/ilink.o:		../std/ilink.h ../h/config.h ../h/header.h ../h/paths.h
	cd ../std;	$(CC) -c $(CFLAGS) -DHeader="\"$(Dir)/piconx.hdr\""  ilink.c

Stripx:	../piconx
	strip ../piconx
