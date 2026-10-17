CFLAGS	      = -O -DPDP -DCLR_SETUID -DSYMLINKS

DESTDIR	      = .

EXTHDRS	      = /usr/include/stdio.h

HDRS	      = grab.h

LDFLAGS	      =

LIBS	      =

LINKER	      = cc

MAKEFILE      = Makefile

OBJS	      = bread.o \
		find.o \
		grab.o \
		readi.o \
		tmode.o

PRINT	      = pr

PROGRAM	      = grab

SRCS	      = bread.c \
		find.c \
		grab.c \
		readi.c \
		tmode.c

all:		$(PROGRAM)

$(PROGRAM):     $(OBJS) $(LIBS)
		@echo -n "Loading $(PROGRAM) ... "
		@$(LINKER) $(LDFLAGS) $(OBJS) $(LIBS) -o $(PROGRAM)
		@echo "done"

clean:;		@rm -f $(OBJS)

depend:;	@mkmf -f $(MAKEFILE) PROGRAM=$(PROGRAM) DESTDIR=$(DESTDIR)

index:;		@ctags -wx $(HDRS) $(SRCS)

cp:		$(PROGRAM)
		@echo Installing $(PROGRAM) in $(DESTDIR)
		@cp $(PROGRAM) $(DESTDIR)
		-strip $(DESTDIR)/$(PROGRAM)

print:;		@$(PRINT) $(HDRS) $(SRCS)

program:        $(PROGRAM)

tags:           $(HDRS) $(SRCS); @ctags $(HDRS) $(SRCS)

update:		$(DESTDIR)/$(PROGRAM)

$(DESTDIR)/$(PROGRAM): $(SRCS) $(LIBS) $(HDRS) $(EXTHDRS)
		@make -f $(MAKEFILE) DESTDIR=$(DESTDIR) install
###
bread.o: grab.h /usr/include/stdio.h
find.o: grab.h /usr/include/stdio.h
grab.o: grab.h /usr/include/stdio.h
readi.o: grab.h /usr/include/stdio.h
tmode.o: grab.h /usr/include/stdio.h
