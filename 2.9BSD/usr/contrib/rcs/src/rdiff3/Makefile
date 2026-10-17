# Note: merge uses /usr/new/lib/rdiff3

CFLAGS	      = -O

DEST	      = /usr/new/lib

EXTHDRS	      =

HDRS	      =

LDFLAGS	      = -i

LIBS	      =

LINKER	      = cc

MAKEFILE      = Makefile

OBJS	      = diff3.o

PRINT	      = pr

PROGRAM	      = rdiff3

SRCS	      = diff3.c

all:		$(PROGRAM)

$(PROGRAM):     $(OBJS) $(LIBS)
		@echo -n "Loading $(PROGRAM) ... "
		@$(LINKER) $(LDFLAGS) $(OBJS) $(LIBS) -o $(PROGRAM)
		@echo "done"

clean:;		@rm -f $(OBJS)

depend:;	@mkmf -f $(MAKEFILE) PROGRAM=$(PROGRAM) DEST=$(DEST)

index:;		@ctags -wx $(HDRS) $(SRCS)

cp:		$(PROGRAM)
		@echo Installing $(PROGRAM) in $(DEST)
		@cp $(PROGRAM) $(DEST)
		@strip $(DEST)/$(PROGRAM)

print:;		@$(PRINT) $(HDRS) $(SRCS)

program:        $(PROGRAM)

tags:           $(HDRS) $(SRCS); @ctags $(HDRS) $(SRCS)

update:		$(DEST)/$(PROGRAM)

$(DEST)/$(PROGRAM): $(SRCS) $(LIBS) $(HDRS) $(EXTHDRS)
		@make -f $(MAKEFILE) DEST=$(DEST) cp
