# assemble man pages
#	@(#)Makefile	1.1	2/25/81

MANDIR = /usr/man/man3
CATDIR = /usr/man/cat3

MANUAL = \
	access.3f \
	chdir.3f \
	etime.3f \
	fdate.3f \
	flush.3f \
	fork.3f \
	fseek.3f \
	getarg.3f \
	getc.3f \
	getenv.3f \
	getlog.3f \
	getpid.3f \
	getuid.3f \
	idate.3f \
	kill.3f \
	link.3f \
	loc.3f \
	perror.3f \
	putc.3f \
	qsort.3f \
	signal.3f \
	sleep.3f \
	stat.3f \
	system.3f \
	time.3f \
	unlink.3f \
	wait.3f

man.3.f77:	$(MANUAL)
	@rm -f man.3.f77
	@nroff -man $(MANUAL) > man.3.f77
	@echo "" > /dev/tty

install:
	@cp $(MANUAL) $(MANDIR)
	@for i in $(MANUAL); do nroff -man $$i > $(CATDIR)/$$i; done
	@echo "" > /dev/tty
