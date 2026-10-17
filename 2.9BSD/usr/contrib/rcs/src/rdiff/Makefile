# Note: RCS uses /usr/new/lib/rdiff and /usr/lib/diffh

CFLAGS	      = -O -DDIFF='"${DIFF}"' -DDIFFH='"${DIFFH}"' -DPR='"${PR}"'

DEST	      = /usr/new/lib

DIFF	      = /bin/diff

DIFFH	      = /usr/lib/diffh

EXTHDRS	      =

HDRS	      = diff.h

LDFLAGS	      = -i

LIBS	      =

LINKER	      = cc

MAKEFILE      = Makefile

OBJS	      = diff.o \
		diffdir.o \
		diffreg.o

PRINT	      = pr

PROGRAM	      = rdiff

SRCS	      = diff.c \
		diffdir.c \
		diffreg.c

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
