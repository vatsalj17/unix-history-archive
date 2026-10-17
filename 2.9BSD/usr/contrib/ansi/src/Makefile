#	@(#)Makefile	1.2	(Berkeley)	8/1/83
#
EXTHDRS	      = /usr/include/ctype.h \
		/usr/include/errno.h \
		/usr/include/sgtty.h \
		/usr/include/signal.h \
		/usr/include/stdio.h \
		/usr/include/sys/ioctl.h \
		/usr/include/sys/mtio.h \
		/usr/include/sys/stat.h \
		/usr/include/sys/types.h

HDRS	      =

CFLAGS	      = -O

LDFLAGS	      = -n

LIBS	      =

LINKER	      = cc

MAKEFILE      = Makefile

OBJS	      = ansi.o

PRINT	      = pr

PROGRAM	      = ansi

SRCS	      = ansi.c

all:		$(PROGRAM)

$(PROGRAM):     $(OBJS) $(LIBS)
		@echo -n "Loading $(PROGRAM) ... "
		@$(LINKER) $(LDFLAGS) $(OBJS) $(LIBS) -o $(PROGRAM)
		@echo "done"

clean:;		@rm -f $(OBJS) $(PROGRAM)

depend:;	@mkmf -f $(MAKEFILE) PROGRAM=$(PROGRAM) DESTDIR=$(DESTDIR)

index:;		@ctags -wx $(HDRS) $(SRCS)

cp:		$(PROGRAM)
		cp $(PROGRAM) $(DESTDIR)
		-strip $(DESTDIR)/$(PROGRAM)

print:;		@$(PRINT) $(HDRS) $(SRCS)

program:        $(PROGRAM)

tags:           $(HDRS) $(SRCS); @ctags $(HDRS) $(SRCS)

update:		$(DESTDIR)/$(PROGRAM)

$(DESTDIR)/$(PROGRAM): $(SRCS) $(LIBS) $(HDRS) $(EXTHDRS)
		@make -f $(MAKEFILE) DESTDIR=$(DESTDIR) cp
###
ansi.o: $(EXTHDRS)
