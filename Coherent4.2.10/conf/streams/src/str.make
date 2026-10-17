# Standard 'conf'-system makefile with productions for standardized targets;
# 'make install', 'make clean', 'make manifest', 'make setup', 'make distrib'
# See the top-level 'conf' system Makefile for more details.

INSTALL_DIR=..
PREFIX=.
MANIFEST=$(PREFIX)/src
MAKEFILE=str.make
USRINC=/usr/include

DRIVER=$(INSTALL_DIR)/Driver.a
DESTLIB=$(DRIVER)

TARGETS=$(INSTALL_DIR)/Space.c $(DRIVER) $(INSTALL_DIR)/Stub.o \
	$(INSTALL_DIR)/mkdev

OBJS=   $(DRIVER)(strhead.o) $(DRIVER)(strinit.o) $(DRIVER)(strmem.o) \
	$(DRIVER)(strmisc.o) $(DRIVER)(strmlib.o) $(DRIVER)(struser.o) \
	$(DRIVER)(str_drv.o)

.c.a:
	echo $< >>sources; echo $% >> members

all:	prelude $(TARGETS) postlude

prelude:
	echo > sources; echo > members

postlude:
	@case "$${members=$$(cat members)}" in \
	"")	;; \
	*)	case "$${sources=$$(cat sources)}" in \
		"")	;; \
		*)	echo Compiling $$sources; \
			$(CC) $(CFLAGS) -c $$sources || exit 1 ;; \
		esac; \
		echo Archiving $$members ; \
		$(AR) $(ARFLAGS) $(DESTLIB) $$members; \
		rm -f $$members ;; \
	esac
	rm sources; rm members

distrib:
	@for i in $(TARGETS); do echo $(PREFIX)/$${i\#$(INSTALL_DIR)/}; done

$(DRIVER): $(OBJS)

clean:
	rm -f $(TARGETS)

depend:
	echo > makedep
	grep "^\#include" [a-z]* \
		| sed 's/\(.*\)\.c:.*"\(.*\)".*/$$(DRIVER)(\1.o): \2/' \
		| sed '/\#include/d' >> makedep
	grep "^\#include" [a-z]* \
		| sed 's/\(.*\)\.c:.*<\(.*\)>.*/$$(DRIVER)(\1.o): $$(USRINC)\/\2/' \
		| sed '/\#include/d' >> makedep
	echo '/^\# DO NOT DELETE THIS LINE/+1,$$d' > eddep
	echo '$$r makedep' >> eddep
	echo 'w' >> eddep
	ed $(MAKEFILE) < eddep
	rm makedep eddep
	echo >> $(MAKEFILE)
	echo "\# DEPENDENCIES MUST END AT END OF FILE:" \
	     "IF YOU PUT STUFF HERE, IT WILL GO AWAY" >> $(MAKEFILE)
	echo "\# See make depend, above" >> $(MAKEFILE)

# DO NOT DELETE THIS LINE - make depend needs it

$(DRIVER)(ether.o): $(USRINC)/sys/kernel.h
$(DRIVER)(ether.o): $(USRINC)/sys/dma.h
$(DRIVER)(ether.o): $(USRINC)/sys/stream.h
$(DRIVER)(ether.o): $(USRINC)/sys/strmlib.h
$(DRIVER)(ether.o): $(USRINC)/sys/strproto.h
$(DRIVER)(ether.o): $(USRINC)/sys/dp8390.h
$(DRIVER)(ether.o): $(USRINC)/sys/ether.h
$(DRIVER)(ether.o): $(USRINC)/sys/ints.h
$(DRIVER)(ether.o): $(USRINC)/sys/netstuff.h
$(DRIVER)(ether.o): $(USRINC)/string.h
$(DRIVER)(str_drv.o): $(USRINC)/common/ccompat.h
$(DRIVER)(str_drv.o): $(USRINC)/common/xdebug.h
$(DRIVER)(str_drv.o): $(USRINC)/kernel/ddi_base.h
$(DRIVER)(str_drv.o): $(USRINC)/kernel/strmlib.h
$(DRIVER)(str_drv.o): $(USRINC)/sys/debug.h
$(DRIVER)(str_drv.o): $(USRINC)/sys/types.h
$(DRIVER)(str_drv.o): $(USRINC)/sys/cmn_err.h
$(DRIVER)(str_drv.o): $(USRINC)/sys/file.h
$(DRIVER)(str_drv.o): $(USRINC)/sys/open.h
$(DRIVER)(str_drv.o): $(USRINC)/sys/uio.h
$(DRIVER)(str_drv.o): $(USRINC)/sys/errno.h
$(DRIVER)(str_drv.o): $(USRINC)/sys/poll.h
$(DRIVER)(str_drv.o): $(USRINC)/sys/cred.h
$(DRIVER)(str_drv.o): $(USRINC)/stddef.h
$(DRIVER)(str_drv.o): $(USRINC)/sys/confinfo.h
$(DRIVER)(str_drv.o): $(USRINC)/sys/inode.h
$(DRIVER)(strhead.o): $(USRINC)/common/ccompat.h
$(DRIVER)(strhead.o): $(USRINC)/kernel/strmlib.h
$(DRIVER)(strhead.o): $(USRINC)/sys/debug.h
$(DRIVER)(strhead.o): $(USRINC)/sys/types.h
$(DRIVER)(strhead.o): $(USRINC)/sys/stream.h
$(DRIVER)(strhead.o): $(USRINC)/sys/stropts.h
$(DRIVER)(strhead.o): $(USRINC)/sys/poll.h
$(DRIVER)(strhead.o): $(USRINC)/sys/errno.h
$(DRIVER)(strhead.o): $(USRINC)/sys/signal.h
$(DRIVER)(strhead.o): $(USRINC)/stddef.h
$(DRIVER)(strinit.o): $(USRINC)/common/ccompat.h
$(DRIVER)(strinit.o): $(USRINC)/kernel/strmlib.h
$(DRIVER)(strinit.o): $(USRINC)/sys/cmn_err.h
$(DRIVER)(strinit.o): $(USRINC)/sys/types.h
$(DRIVER)(strinit.o): $(USRINC)/sys/errno.h
$(DRIVER)(strinit.o): $(USRINC)/sys/stat.h
$(DRIVER)(strinit.o): $(USRINC)/stropts.h
$(DRIVER)(strinit.o): $(USRINC)/stddef.h
$(DRIVER)(strinit.o): $(USRINC)/sys/inode.h
$(DRIVER)(strinit.o): $(USRINC)/sys/fd.h
$(DRIVER)(strinit.o): $(USRINC)/sys/proc.h
$(DRIVER)(strmem.o): $(USRINC)/common/ccompat.h
$(DRIVER)(strmem.o): $(USRINC)/kernel/ddi_lock.h
$(DRIVER)(strmem.o): $(USRINC)/sys/types.h
$(DRIVER)(strmem.o): $(USRINC)/sys/debug.h
$(DRIVER)(strmem.o): $(USRINC)/sys/ksynch.h
$(DRIVER)(strmem.o): $(USRINC)/sys/cmn_err.h
$(DRIVER)(strmem.o): $(USRINC)/sys/kmem.h
$(DRIVER)(strmem.o): $(USRINC)/kernel/strmlib.h
$(DRIVER)(strmem.o): $(USRINC)/string.h
$(DRIVER)(strmisc.o): $(USRINC)/common/ccompat.h
$(DRIVER)(strmisc.o): $(USRINC)/kernel/strmlib.h
$(DRIVER)(strmisc.o): $(USRINC)/sys/debug.h
$(DRIVER)(strmisc.o): $(USRINC)/sys/types.h
$(DRIVER)(strmisc.o): $(USRINC)/sys/kmem.h
$(DRIVER)(strmisc.o): $(USRINC)/sys/stream.h
$(DRIVER)(strmlib.o): $(USRINC)/common/ccompat.h
$(DRIVER)(strmlib.o): $(USRINC)/kernel/defer.h
$(DRIVER)(strmlib.o): $(USRINC)/sys/debug.h
$(DRIVER)(strmlib.o): $(USRINC)/sys/types.h
$(DRIVER)(strmlib.o): $(USRINC)/sys/kmem.h
$(DRIVER)(strmlib.o): $(USRINC)/sys/cmn_err.h
$(DRIVER)(strmlib.o): $(USRINC)/sys/strlog.h
$(DRIVER)(strmlib.o): $(USRINC)/sys/errno.h
$(DRIVER)(strmlib.o): $(USRINC)/stdarg.h
$(DRIVER)(strmlib.o): $(USRINC)/string.h
$(DRIVER)(strmlib.o): $(USRINC)/sys/stream.h
$(DRIVER)(strmlib.o): $(USRINC)/kernel/strmlib.h
$(DRIVER)(struser.o): $(USRINC)/common/ccompat.h
$(DRIVER)(struser.o): $(USRINC)/kernel/ddi_lock.h
$(DRIVER)(struser.o): $(USRINC)/kernel/strmlib.h
$(DRIVER)(struser.o): $(USRINC)/sys/confinfo.h
$(DRIVER)(struser.o): $(USRINC)/sys/types.h
$(DRIVER)(struser.o): $(USRINC)/sys/kmem.h
$(DRIVER)(struser.o): $(USRINC)/sys/poll.h
$(DRIVER)(struser.o): $(USRINC)/sys/ksynch.h
$(DRIVER)(struser.o): $(USRINC)/sys/file.h
$(DRIVER)(struser.o): $(USRINC)/sys/uio.h
$(DRIVER)(struser.o): $(USRINC)/sys/cmn_err.h
$(DRIVER)(struser.o): $(USRINC)/sys/errno.h
$(DRIVER)(struser.o): $(USRINC)/sys/signal.h
$(DRIVER)(struser.o): $(USRINC)/sys/fd.h
$(DRIVER)(struser.o): $(USRINC)/sys/cred.h
$(DRIVER)(struser.o): $(USRINC)/stropts.h
$(DRIVER)(struser.o): $(USRINC)/string.h
$(DRIVER)(struser.o): $(USRINC)/poll.h

# DEPENDENCIES MUST END AT END OF FILE: IF YOU PUT STUFF HERE, IT WILL GO AWAY
# See make depend, above
