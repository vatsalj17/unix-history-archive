all:		icont iconx.hdr
	
icont.o:	../h/config.h ../h/paths.h
icont:		icont.o
		$(CC) $(CFLAGS) -o icont icont.o
	
ixhdr.o:	../h/config.h ../h/header.h ../h/paths.h
iconx.hdr:	ixhdr.o
		$(CC) $(CFLAGS) $(LDFLAGS) ixhdr.o -o iconx.hdr
		strip iconx.hdr
