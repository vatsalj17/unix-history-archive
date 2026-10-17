
HDRS=		../h/rt.h ../h/config.h ../h/cpuconf.h ../h/memsize.h

FOBJS= 		fconv.o fmisc.o fstr.o fstranl.o fstruct.o fsys.o
IOBJS=		imain.o idata.o interp.o invoke.o
LOBJS=		lmisc.o lrec.o lscan.o
OOBJS=		oarith.o oasgn.o ocat.o ocomp.o omisc.o oref.o oset.o\
		 ovalue.o
ROBJS=		rcomp.o rconv.o rdefault.o rdoasgn.o rlocal.o rmemmgt.o\
		 rmemmon.o rmisc.o rover.o rstruct.o rswitch.o rsys.o
OBJS=		$(IOBJS) $(FOBJS) $(LOBJS) $(OOBJS) $(ROBJS)


iconx:		$(OBJS)
		$(CC) $(LDFLAGS) -o iconx  $(OBJS) -lm

$(OBJS):	$(HDRS)

fmisc.o:	$(HDRS) gc.h
idata.o:	$(HDRS) ../h/fdef.h
imain.o:	$(HDRS) gc.h ../h/header.h ../h/version.h
interp.o: 	$(HDRS) gc.h ../h/opdef.h
lmisc.o:	$(HDRS) ../h/keyword.h ../h/version.h
rconv.o:	$(HDRS) ../h/fdef.h
rmemmgt.o:	$(HDRS) gc.h
rmemmon.o:	$(HDRS) gc.h
rover.o:	$(HDRS) $(ROVER)
		$(CC) $(CFLAGS) $(OFLAGS) -c $(ROVER)
rswitch.o:	$(HDRS) $(RSWITCH)
		$(CC) $(CFLAGS) $(OFLAGS) -c $(RSWITCH)
