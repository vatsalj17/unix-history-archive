# Sub-makefile to batch compilation and archiving into large passes

MAKEFILE=i386.make
DESTLIB=$(K386LIB)/k386.a
USRINC=/usr/include
ASFLAGS=

OBJS = 	$(DESTLIB)(defer.o) $(DESTLIB)(die.o) \
	$(DESTLIB)(dmac.o) $(DESTLIB)(dmalock.o) $(DESTLIB)(fakedma.o) \
	$(DESTLIB)(ff.o) $(DESTLIB)(md.o) $(DESTLIB)(mmu.o) \
	$(DESTLIB)(msig.o) $(DESTLIB)(ndp.o) $(DESTLIB)(ndpas.o) \
	$(DESTLIB)(shm0.o) $(DESTLIB)(sys1632.o) $(DESTLIB)(tioc.o) \
	$(DESTLIB)(trap.o) $(DESTLIB)(user_bt.o) $(DESTLIB)(work.o) \
	$(DESTLIB)(cyrix.o) $(DESTLIB)(cyrix0.o) \
	$(DESTLIB)(mchinit.o) \
	$(K386LIB)/k0.o

.c.a:
	echo $< >>sources; echo $% >> members

all:	prelude $(OBJS) postlude

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
		echo Archiving $$members; \
		$(AR) $(ARFLAGS) $(DESTLIB) $$members || exit 1; \
		rm -f $$members ;; \
	esac
	rm sources; rm members

depend:
	echo > makedep
	grep "^\#include" *.c \
		| sed 's/\(.*\)\.c:.*"\(.*\)".*/\1.o: \2/' \
		| sed '/\#include/d' >> makedep
	grep "^\#include" *.c \
		| sed 's/\(.*\)\.c:.*<\(.*\)>.*/$$(DESTLIB)(\1.o): $$(USRINC)\/\2/' \
		| sed '/\#include/d' >> makedep
	grep ".include" *.s \
		| sed 's/\(.*\)\.s:.*"\(.*\)".*/$$(K386LIB)(\1.o): \2/' \
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

$(DESTLIB)(cyrix.o): $(USRINC)/sys/cyrix.h
$(DESTLIB)(cyrix.o): $(USRINC)/sys/coherent.h
$(DESTLIB)(die.o): $(USRINC)/kernel/reg.h
$(DESTLIB)(die.o): $(USRINC)/sys/mmu.h
$(DESTLIB)(die.o): $(USRINC)/ctype.h
$(DESTLIB)(dmac.o): $(USRINC)/sys/types.h
$(DESTLIB)(dmac.o): $(USRINC)/kernel/reg.h
$(DESTLIB)(dmac.o): $(USRINC)/sys/dmac.h
$(DESTLIB)(dmalock.o): $(USRINC)/kernel/timeout.h
$(DESTLIB)(fakedma.o): $(USRINC)/sys/debug.h
$(DESTLIB)(fakedma.o): $(USRINC)/sys/inline.h
$(DESTLIB)(fakedma.o): $(USRINC)/sys/errno.h
$(DESTLIB)(fakedma.o): $(USRINC)/sys/cmn_err.h
$(DESTLIB)(fakedma.o): $(USRINC)/signal.h
$(DESTLIB)(fakedma.o): $(USRINC)/kernel/reg.h
$(DESTLIB)(fakedma.o): $(USRINC)/sys/mmu.h
$(DESTLIB)(fakedma.o): $(USRINC)/sys/inode.h
$(DESTLIB)(fakedma.o): $(USRINC)/sys/proc.h
$(DESTLIB)(fakedma.o): $(USRINC)/sys/seg.h
$(DESTLIB)(ff.o): $(USRINC)/sys/types.h
$(DESTLIB)(ff.o): $(USRINC)/kernel/fakeff.h
$(DESTLIB)(ff.o): $(USRINC)/kernel/reg.h
$(DESTLIB)(ff.o): $(USRINC)/sys/mmu.h
$(DESTLIB)(ff.o): $(USRINC)/sys/seg.h
$(DESTLIB)(ff.o): $(USRINC)/sys/cmn_err.h
$(DESTLIB)(lex.yy.o): $(USRINC)/stdio.h
$(DESTLIB)(mchinit.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(mchinit.o): $(USRINC)/kernel/reg.h
$(DESTLIB)(mchinit.o): $(USRINC)/kernel/trace.h
$(DESTLIB)(mchinit.o): $(USRINC)/sys/mmu.h
$(DESTLIB)(mchinit.o): $(USRINC)/sys/proc.h
$(DESTLIB)(mchinit.o): $(USRINC)/limits.h
$(DESTLIB)(md.o): $(USRINC)/common/_tricks.h
$(DESTLIB)(md.o): $(USRINC)/sys/types.h
$(DESTLIB)(md.o): $(USRINC)/sys/inline.h
$(DESTLIB)(md.o): $(USRINC)/sys/confinfo.h
$(DESTLIB)(md.o): $(USRINC)/stddef.h
$(DESTLIB)(md.o): $(USRINC)/kernel/trace.h
$(DESTLIB)(md.o): $(USRINC)/kernel/reg.h
$(DESTLIB)(md.o): $(USRINC)/kernel/ddi_cpu.h
$(DESTLIB)(md.o): $(USRINC)/kernel/intr.h
$(DESTLIB)(md.o): $(USRINC)/sys/coherent.h
$(DESTLIB)(mmu.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(mmu.o): $(USRINC)/common/_tricks.h
$(DESTLIB)(mmu.o): $(USRINC)/sys/errno.h
$(DESTLIB)(mmu.o): $(USRINC)/sys/signal.h
$(DESTLIB)(mmu.o): $(USRINC)/coh/misc.h
$(DESTLIB)(mmu.o): $(USRINC)/limits.h
$(DESTLIB)(mmu.o): $(USRINC)/kernel/trace.h
$(DESTLIB)(mmu.o): $(USRINC)/kernel/alloc.h
$(DESTLIB)(mmu.o): $(USRINC)/kernel/param.h
$(DESTLIB)(mmu.o): $(USRINC)/kernel/reg.h
$(DESTLIB)(mmu.o): $(USRINC)/sys/mmu.h
$(DESTLIB)(mmu.o): $(USRINC)/sys/proc.h
$(DESTLIB)(mmu.o): $(USRINC)/sys/clist.h
$(DESTLIB)(mmu.o): $(USRINC)/sys/inode.h
$(DESTLIB)(mmu.o): $(USRINC)/sys/seg.h
$(DESTLIB)(mmu.o): $(USRINC)/sys/buf.h
$(DESTLIB)(mmu.o): $(USRINC)/sys/filsys.h
$(DESTLIB)(mmu.o): $(USRINC)/l.out.h
$(DESTLIB)(mmu.o): $(USRINC)/ieeefp.h
$(DESTLIB)(msig.o): $(USRINC)/common/_gregset.h
$(DESTLIB)(msig.o): $(USRINC)/kernel/sig_lib.h
$(DESTLIB)(msig.o): $(USRINC)/sys/cmn_err.h
$(DESTLIB)(msig.o): $(USRINC)/sys/debug.h
$(DESTLIB)(msig.o): $(USRINC)/stddef.h
$(DESTLIB)(msig.o): $(USRINC)/kernel/param.h
$(DESTLIB)(msig.o): $(USRINC)/kernel/reg.h
$(DESTLIB)(msig.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(msig.o): $(USRINC)/sys/proc.h
$(DESTLIB)(ndp.o): $(USRINC)/common/_gregset.h
$(DESTLIB)(ndp.o): $(USRINC)/sys/errno.h
$(DESTLIB)(ndp.o): $(USRINC)/sys/signal.h
$(DESTLIB)(ndp.o): $(USRINC)/stddef.h
$(DESTLIB)(ndp.o): $(USRINC)/kernel/reg.h
$(DESTLIB)(ndp.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(ndp.o): $(USRINC)/sys/proc.h
$(DESTLIB)(ndp.o): $(USRINC)/sys/mmu.h
$(DESTLIB)(ndp.o): $(USRINC)/sys/ndp.h
$(DESTLIB)(ndp.o): $(USRINC)/sys/seg.h
$(DESTLIB)(shm0.o): $(USRINC)/kernel/reg.h
$(DESTLIB)(shm0.o): $(USRINC)/sys/proc.h
$(DESTLIB)(shm0.o): $(USRINC)/sys/mmu.h
$(DESTLIB)(shm0.o): $(USRINC)/sys/shm.h
$(DESTLIB)(shm0.o): $(USRINC)/sys/seg.h
$(DESTLIB)(sys1632.o): $(USRINC)/common/_limits.h
$(DESTLIB)(sys1632.o): $(USRINC)/common/_tricks.h
$(DESTLIB)(sys1632.o): $(USRINC)/common/_gregset.h
$(DESTLIB)(sys1632.o): $(USRINC)/sys/errno.h
$(DESTLIB)(sys1632.o): $(USRINC)/sys/debug.h
$(DESTLIB)(sys1632.o): $(USRINC)/sys/cmn_err.h
$(DESTLIB)(sys1632.o): $(USRINC)/signal.h
$(DESTLIB)(sys1632.o): $(USRINC)/stddef.h
$(DESTLIB)(sys1632.o): $(USRINC)/kernel/trace.h
$(DESTLIB)(sys1632.o): $(USRINC)/kernel/reg.h
$(DESTLIB)(sys1632.o): $(USRINC)/sys/acct.h
$(DESTLIB)(sys1632.o): $(USRINC)/sys/buf.h
$(DESTLIB)(sys1632.o): $(USRINC)/sys/con.h
$(DESTLIB)(sys1632.o): $(USRINC)/sys/filsys.h
$(DESTLIB)(sys1632.o): $(USRINC)/sys/ino.h
$(DESTLIB)(sys1632.o): $(USRINC)/sys/inode.h
$(DESTLIB)(sys1632.o): $(USRINC)/sys/proc.h
$(DESTLIB)(sys1632.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(sys1632.o): $(USRINC)/sys/sched.h
$(DESTLIB)(sys1632.o): $(USRINC)/sys/seg.h
$(DESTLIB)(sys1632.o): $(USRINC)/sys/timeb.h
$(DESTLIB)(sys1632.o): $(USRINC)/sys/fd.h
$(DESTLIB)(sys1632.o): $(USRINC)/l.out.h
$(DESTLIB)(sys1632.o): $(USRINC)/canon.h
$(DESTLIB)(sys1632.o): $(USRINC)/kernel/systab.h
$(DESTLIB)(tioc.o): $(USRINC)/sys/types.h
$(DESTLIB)(tioc.o): $(USRINC)/sys/errno.h
$(DESTLIB)(tioc.o): $(USRINC)/sgtty.h
$(DESTLIB)(tioc.o): $(USRINC)/sys/mmu.h
$(DESTLIB)(trap.o): $(USRINC)/common/_gregset.h
$(DESTLIB)(trap.o): $(USRINC)/sys/errno.h
$(DESTLIB)(trap.o): $(USRINC)/signal.h
$(DESTLIB)(trap.o): $(USRINC)/stddef.h
$(DESTLIB)(trap.o): $(USRINC)/kernel/trace.h
$(DESTLIB)(trap.o): $(USRINC)/kernel/systab.h
$(DESTLIB)(trap.o): $(USRINC)/kernel/reg.h
$(DESTLIB)(trap.o): $(USRINC)/sys/proc.h
$(DESTLIB)(trap.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(trap.o): $(USRINC)/sys/seg.h
$(DESTLIB)(trap.o): $(USRINC)/sys/mmu.h
$(DESTLIB)(user_bt.o): $(USRINC)/common/_gregset.h
$(DESTLIB)(user_bt.o): $(USRINC)/sys/types.h
$(DESTLIB)(user_bt.o): $(USRINC)/sys/cmn_err.h
$(DESTLIB)(user_bt.o): $(USRINC)/kernel/reg.h
$(DESTLIB)(user_bt.o): $(USRINC)/sys/seg.h
$(DESTLIB)(user_bt.o): $(USRINC)/sys/proc.h
$(DESTLIB)(user_bt.o): $(USRINC)/sys/mmu.h
$(DESTLIB)(work.o): $(USRINC)/kernel/reg.h
$(DESTLIB)(work.o): $(USRINC)/sys/mmu.h
$(K386LIB)(k0.o): as.inc
$(K386LIB)(k0.o): lib/struct.inc
$(K386LIB)(k0.o): lib/intr.inc
$(K386LIB)(k0.o): lib/ddi.inc
$(K386LIB)(tst0.o): as.inc
$(K386LIB)(tst0.o): lib/struct.inc
$(K386LIB)(tst0.o): lib/intr.inc
$(K386LIB)(tst0.o): lib/ddi.inc
$(K386LIB)(tst1.o): as.inc
$(K386LIB)(tst1.o): lib/struct.inc
$(K386LIB)(tst1.o): lib/intr.inc
$(K386LIB)(tst1.o): lib/ddi.inc

# DEPENDENCIES MUST END AT END OF FILE: IF YOU PUT STUFF HERE, IT WILL GO AWAY
# See make depend, above
