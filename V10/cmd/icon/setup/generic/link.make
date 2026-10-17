HDRS=		ilink.h ../h/rt.h ../h/config.h ../h/cpuconf.h ../h/memsize.h

OBJS=		builtin.o ilink.o glob.o lcode.o llex.o lmem.o lsym.o opcode.o

ilink:		$(OBJS)
		$(CC) $(LDFLAGS) -o ilink $(OBJS)

$(OBJS):	$(HDRS)

builtin.o:	$(HDRS) ../h/fdef.h
glob.o:		$(HDRS) opcode.h ../h/version.h
ilink.o:	$(HDRS) ../h/header.h ../h/paths.h
lcode.o:	$(HDRS) opcode.h ../h/header.h ../h/keyword.h ../h/version.h
llex.o:		$(HDRS) opcode.h
opcode.o:	$(HDRS) opcode.h 
