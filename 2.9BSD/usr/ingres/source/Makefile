DESTDIR=/usr
DESTBIN=$(DESTDIR)/bin
DESTLIB=$(DESTDIR)/lib
INSTALL=install
SUALL=	creatdb ingres destroydb sysmod purge printr helpr\
	restore
SYSALL=	creatdb ingres destroydb sysmod purge printr helpr \
	restore demodb
SUMODE=	4751
MAKE=	make

install:
	chdir gutil; $(MAKE) $(INSTALL)
	chdir iutil; $(MAKE) $(INSTALL)
	chdir access; $(MAKE) $(INSTALL)
	chdir dbu; $(MAKE) $(INSTALL)
	chdir support; $(MAKE) $(INSTALL)
	chdir monitor; $(MAKE) $(INSTALL)
	chdir scanner; $(MAKE) $(INSTALL)
	chdir yyyacc; $(MAKE) $(INSTALL)
	chdir parser; $(MAKE) $(INSTALL)
	chdir qrymod; $(MAKE) $(INSTALL)
	chdir ovqp; $(MAKE) $(INSTALL)
	chdir decomp; $(MAKE) $(INSTALL)
	chdir equel; $(MAKE) $(INSTALL)
	chdir equel/runtime; $(MAKE) $(INSTALL)

sysinstall: install
	chdir $(BIN); cp $(SYSALL) $(DESTBIN)
	chdir $(DESTBIN); chmod $(SUMODE) $(SUALL)
	chdir $(BIN); cp libq.a $(DESTLIB)
