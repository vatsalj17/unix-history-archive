DEST	      = $(DESTDIR)/usr/man/mann

MAKEFILE      = Makefile

PRINT	      = vtroff

SRCS	      =	ansi.1

all:;

clean:;

cp:;		@echo Installing $(SRCS) in $(DEST)
		@for i in $(SRCS); do cp $$i $(DEST)/`basename $$i .1`.n; done

print:		$(SRCS)
		@$(PRINT) -man $?
		@touch print

update:;
