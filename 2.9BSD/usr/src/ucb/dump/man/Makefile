DEST	      = $(DESTDIR)/usr/man

MAKEFILE      = Makefile

PRINT	      = vtroff

SRCS	      =	dump.5 dump.8

all:;

clean:;

cp:;		@echo Installing $(SRCS) in $(DEST)
		cp dump.5 $(DEST)/man5
		cp dump.8 $(DEST)/man8

print:		$(SRCS)
		@$(PRINT) -man $?
		@touch print

update:;
