# Sub-makefile for batching up compilation and archiving tasks.

USRINC=/usr/include
DESTLIB=$(K386LIB)/k386.a
MAKEFILE=coh.make

OBJS=   $(DESTLIB)(alloc.o) $(DESTLIB)(bio.o) \
	$(DESTLIB)(clock.o) $(DESTLIB)(clocked.o) $(DESTLIB)(exec.o) \
	$(DESTLIB)(fd.o) $(DESTLIB)(fifo.o) $(DESTLIB)(fs1.o) \
	$(DESTLIB)(fs2.o) $(DESTLIB)(fs3.o) $(DESTLIB)(main.o) \
	$(DESTLIB)(misc.o) $(DESTLIB)(null.o) $(DESTLIB)(pipe.o) \
	$(DESTLIB)(printf.o) $(DESTLIB)(proc.o) \
	$(DESTLIB)(rlock.o) $(DESTLIB)(seg.o) $(DESTLIB)(sig.o) \
	$(DESTLIB)(support.o) $(DESTLIB)(sys1.o) $(DESTLIB)(sys2.o) \
	$(DESTLIB)(sys3.o) $(DESTLIB)(sys4.o) $(DESTLIB)(sys5.o) \
	$(DESTLIB)(syscoh.o) $(DESTLIB)(timeout.o) $(DESTLIB)(var.o) \
	$(DESTLIB)(sys6.o) $(DESTLIB)(nlp.o) $(DESTLIB)(v_proc.o)

#	$(DESTLIB)(poll.o) 

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
		echo Archiving $$members ; \
		$(AR) $(ARFLAGS) $(DESTLIB) $$members || exit 1; \
		rm -f $$members ;; \
	esac
	rm sources; rm members

$(DESTLIB)(main.o): ../i386/version main.c
	$(CC) $(CFLAGS) `../i386/version` -c $*.c
	echo $% >> members

depend:
	echo > makedep
	grep "^\#include" * | \
		sed 's/\(.*\)\.c:.*"\(.*\)".*/$$(DESTLIB)(\1.o): \2/' | \
		sed '/\#include/d' >> makedep
	grep "^\#include" * | \
		sed 's/\(.*\)\.c:.*<\(.*\)>.*/$$(DESTLIB)(\1.o): $$(USRINC)\/\2/' | \
		sed '/\#include/d' >> makedep
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

$(DESTLIB)(alloc.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(alloc.o): $(USRINC)/sys/cmn_err.h
$(DESTLIB)(alloc.o): $(USRINC)/sys/debug.h
$(DESTLIB)(alloc.o): $(USRINC)/sys/kmem.h
$(DESTLIB)(alloc.o): $(USRINC)/sys/errno.h
$(DESTLIB)(alloc.o): $(USRINC)/kernel/trace.h
$(DESTLIB)(alloc.o): $(USRINC)/sys/proc.h
$(DESTLIB)(alloc.o): $(USRINC)/kernel/alloc.h
$(DESTLIB)(bio.o): $(USRINC)/common/gregset.h
$(DESTLIB)(bio.o): $(USRINC)/common/_canon.h
$(DESTLIB)(bio.o): $(USRINC)/kernel/proc_lib.h
$(DESTLIB)(bio.o): $(USRINC)/sys/param.h
$(DESTLIB)(bio.o): $(USRINC)/sys/confinfo.h
$(DESTLIB)(bio.o): $(USRINC)/sys/kmem.h
$(DESTLIB)(bio.o): $(USRINC)/sys/cmn_err.h
$(DESTLIB)(bio.o): $(USRINC)/sys/debug.h
$(DESTLIB)(bio.o): $(USRINC)/sys/errno.h
$(DESTLIB)(bio.o): $(USRINC)/sys/stat.h
$(DESTLIB)(bio.o): $(USRINC)/sys/file.h
$(DESTLIB)(bio.o): $(USRINC)/stddef.h
$(DESTLIB)(bio.o): $(USRINC)/limits.h
$(DESTLIB)(bio.o): $(USRINC)/kernel/trace.h
$(DESTLIB)(bio.o): $(USRINC)/kernel/reg.h
$(DESTLIB)(bio.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(bio.o): $(USRINC)/sys/inode.h
$(DESTLIB)(bio.o): $(USRINC)/sys/mmu.h
$(DESTLIB)(bio.o): $(USRINC)/sys/buf.h
$(DESTLIB)(bio.o): $(USRINC)/sys/con.h
$(DESTLIB)(bio.o): $(USRINC)/sys/io.h
$(DESTLIB)(bio.o): $(USRINC)/sys/proc.h
$(DESTLIB)(bio.o): $(USRINC)/sys/sched.h
$(DESTLIB)(bio.o): $(USRINC)/sys/seg.h
$(DESTLIB)(bio.o): $(USRINC)/sys/dmac.h
$(DESTLIB)(bio.o): $(USRINC)/sys/types.h
$(DESTLIB)(bio.o): $(USRINC)/sgtty.h
$(DESTLIB)(bio.o): $(USRINC)/kernel/ddi_cpu.h
$(DESTLIB)(clock.o): $(USRINC)/common/_gregset.h
$(DESTLIB)(clock.o): $(USRINC)/common/_tricks.h
$(DESTLIB)(clock.o): $(USRINC)/kernel/ddi_glob.h
$(DESTLIB)(clock.o): $(USRINC)/kernel/ddi_cpu.h
$(DESTLIB)(clock.o): $(USRINC)/sys/cmn_err.h
$(DESTLIB)(clock.o): $(USRINC)/sys/stat.h
$(DESTLIB)(clock.o): $(USRINC)/stddef.h
$(DESTLIB)(clock.o): $(USRINC)/kernel/_timers.h
$(DESTLIB)(clock.o): $(USRINC)/kernel/sig_lib.h
$(DESTLIB)(clock.o): $(USRINC)/kernel/timeout.h
$(DESTLIB)(clock.o): $(USRINC)/sys/con.h
$(DESTLIB)(clock.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(clock.o): $(USRINC)/sys/proc.h
$(DESTLIB)(clock.o): $(USRINC)/sys/sched.h
$(DESTLIB)(clock.o): $(USRINC)/sys/mmu.h
$(DESTLIB)(clocked.o): $(USRINC)/kernel/param.h
$(DESTLIB)(exec.o): $(USRINC)/common/_gregset.h
$(DESTLIB)(exec.o): $(USRINC)/common/_tricks.h
$(DESTLIB)(exec.o): $(USRINC)/kernel/proc_lib.h
$(DESTLIB)(exec.o): $(USRINC)/kernel/cred_lib.h
$(DESTLIB)(exec.o): $(USRINC)/kernel/sig_lib.h
$(DESTLIB)(exec.o): $(USRINC)/sys/cmn_err.h
$(DESTLIB)(exec.o): $(USRINC)/sys/errno.h
$(DESTLIB)(exec.o): $(USRINC)/sys/file.h
$(DESTLIB)(exec.o): $(USRINC)/sys/stat.h
$(DESTLIB)(exec.o): $(USRINC)/sys/cred.h
$(DESTLIB)(exec.o): $(USRINC)/signal.h
$(DESTLIB)(exec.o): $(USRINC)/fcntl.h
$(DESTLIB)(exec.o): $(USRINC)/stddef.h
$(DESTLIB)(exec.o): $(USRINC)/kernel/reg.h
$(DESTLIB)(exec.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(exec.o): $(USRINC)/sys/mmu.h
$(DESTLIB)(exec.o): $(USRINC)/sys/acct.h
$(DESTLIB)(exec.o): $(USRINC)/sys/buf.h
$(DESTLIB)(exec.o): $(USRINC)/canon.h
$(DESTLIB)(exec.o): $(USRINC)/sys/con.h
$(DESTLIB)(exec.o): $(USRINC)/sys/ino.h
$(DESTLIB)(exec.o): $(USRINC)/sys/inode.h
$(DESTLIB)(exec.o): $(USRINC)/a.out.h
$(DESTLIB)(exec.o): $(USRINC)/l.out.h
$(DESTLIB)(exec.o): $(USRINC)/sys/proc.h
$(DESTLIB)(exec.o): $(USRINC)/sys/sched.h
$(DESTLIB)(exec.o): $(USRINC)/sys/seg.h
$(DESTLIB)(exec.o): $(USRINC)/sys/fd.h
$(DESTLIB)(exec.o): $(USRINC)/sys/types.h
$(DESTLIB)(fd.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(fd.o): $(USRINC)/kernel/proc_lib.h
$(DESTLIB)(fd.o): $(USRINC)/sys/debug.h
$(DESTLIB)(fd.o): $(USRINC)/sys/errno.h
$(DESTLIB)(fd.o): $(USRINC)/sys/kmem.h
$(DESTLIB)(fd.o): $(USRINC)/fcntl.h
$(DESTLIB)(fd.o): $(USRINC)/stddef.h
$(DESTLIB)(fd.o): $(USRINC)/kernel/param.h
$(DESTLIB)(fd.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(fd.o): $(USRINC)/sys/fd.h
$(DESTLIB)(fd.o): $(USRINC)/sys/inode.h
$(DESTLIB)(fifo.o): $(USRINC)/kernel/typed.h
$(DESTLIB)(fs1.o): $(USRINC)/kernel/proc_lib.h
$(DESTLIB)(fs1.o): $(USRINC)/sys/cmn_err.h
$(DESTLIB)(fs1.o): $(USRINC)/sys/debug.h
$(DESTLIB)(fs1.o): $(USRINC)/sys/stat.h
$(DESTLIB)(fs1.o): $(USRINC)/sys/file.h
$(DESTLIB)(fs1.o): $(USRINC)/sys/errno.h
$(DESTLIB)(fs1.o): $(USRINC)/sys/cred.h
$(DESTLIB)(fs1.o): $(USRINC)/dirent.h
$(DESTLIB)(fs1.o): $(USRINC)/stddef.h
$(DESTLIB)(fs1.o): $(USRINC)/kernel/trace.h
$(DESTLIB)(fs1.o): $(USRINC)/acct.h
$(DESTLIB)(fs1.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(fs1.o): $(USRINC)/sys/buf.h
$(DESTLIB)(fs1.o): $(USRINC)/canon.h
$(DESTLIB)(fs1.o): $(USRINC)/sys/filsys.h
$(DESTLIB)(fs1.o): $(USRINC)/sys/ino.h
$(DESTLIB)(fs1.o): $(USRINC)/sys/inode.h
$(DESTLIB)(fs1.o): $(USRINC)/sys/io.h
$(DESTLIB)(fs1.o): $(USRINC)/sys/mount.h
$(DESTLIB)(fs1.o): $(USRINC)/sys/types.h
$(DESTLIB)(fs2.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(fs2.o): $(USRINC)/kernel/proc_lib.h
$(DESTLIB)(fs2.o): $(USRINC)/sys/cmn_err.h
$(DESTLIB)(fs2.o): $(USRINC)/sys/debug.h
$(DESTLIB)(fs2.o): $(USRINC)/sys/errno.h
$(DESTLIB)(fs2.o): $(USRINC)/sys/stat.h
$(DESTLIB)(fs2.o): $(USRINC)/sys/file.h
$(DESTLIB)(fs2.o): $(USRINC)/sys/kmem.h
$(DESTLIB)(fs2.o): $(USRINC)/sys/cred.h
$(DESTLIB)(fs2.o): $(USRINC)/stddef.h
$(DESTLIB)(fs2.o): $(USRINC)/kernel/trace.h
$(DESTLIB)(fs2.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(fs2.o): $(USRINC)/sys/acct.h
$(DESTLIB)(fs2.o): $(USRINC)/sys/buf.h
$(DESTLIB)(fs2.o): $(USRINC)/canon.h
$(DESTLIB)(fs2.o): $(USRINC)/sys/con.h
$(DESTLIB)(fs2.o): $(USRINC)/sys/filsys.h
$(DESTLIB)(fs2.o): $(USRINC)/sys/ino.h
$(DESTLIB)(fs2.o): $(USRINC)/sys/inode.h
$(DESTLIB)(fs2.o): $(USRINC)/sys/io.h
$(DESTLIB)(fs2.o): $(USRINC)/sys/mount.h
$(DESTLIB)(fs2.o): $(USRINC)/sys/proc.h
$(DESTLIB)(fs3.o): $(USRINC)/common/_tricks.h
$(DESTLIB)(fs3.o): $(USRINC)/kernel/proc_lib.h
$(DESTLIB)(fs3.o): $(USRINC)/sys/cmn_err.h
$(DESTLIB)(fs3.o): $(USRINC)/sys/debug.h
$(DESTLIB)(fs3.o): $(USRINC)/sys/errno.h
$(DESTLIB)(fs3.o): $(USRINC)/sys/stat.h
$(DESTLIB)(fs3.o): $(USRINC)/sys/file.h
$(DESTLIB)(fs3.o): $(USRINC)/stddef.h
$(DESTLIB)(fs3.o): $(USRINC)/limits.h
$(DESTLIB)(fs3.o): $(USRINC)/kernel/trace.h
$(DESTLIB)(fs3.o): $(USRINC)/kernel/reg.h
$(DESTLIB)(fs3.o): $(USRINC)/sys/buf.h
$(DESTLIB)(fs3.o): $(USRINC)/canon.h
$(DESTLIB)(fs3.o): $(USRINC)/sys/con.h
$(DESTLIB)(fs3.o): $(USRINC)/sys/filsys.h
$(DESTLIB)(fs3.o): $(USRINC)/sys/mount.h
$(DESTLIB)(fs3.o): $(USRINC)/sys/io.h
$(DESTLIB)(fs3.o): $(USRINC)/sys/ino.h
$(DESTLIB)(fs3.o): $(USRINC)/sys/inode.h
$(DESTLIB)(fs3.o): $(USRINC)/sys/types.h
$(DESTLIB)(main.o): $(USRINC)/kernel/proc_lib.h
$(DESTLIB)(main.o): $(USRINC)/kernel/ddi_cpu.h
$(DESTLIB)(main.o): $(USRINC)/sys/stat.h
$(DESTLIB)(main.o): $(USRINC)/kernel/param.h
$(DESTLIB)(main.o): $(USRINC)/kernel/reg.h
$(DESTLIB)(main.o): $(USRINC)/kernel/typed.h
$(DESTLIB)(main.o): $(USRINC)/kernel/trace.h
$(DESTLIB)(main.o): $(USRINC)/sys/cmn_err.h
$(DESTLIB)(main.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(main.o): $(USRINC)/sys/mmu.h
$(DESTLIB)(main.o): $(USRINC)/sys/devices.h
$(DESTLIB)(main.o): $(USRINC)/sys/fdisk.h
$(DESTLIB)(main.o): $(USRINC)/sys/proc.h
$(DESTLIB)(main.o): $(USRINC)/sys/seg.h
$(DESTLIB)(misc.o): $(USRINC)/kernel/proc_lib.h
$(DESTLIB)(misc.o): $(USRINC)/sys/cmn_err.h
$(DESTLIB)(misc.o): $(USRINC)/sys/types.h
$(DESTLIB)(misc.o): $(USRINC)/sys/errno.h
$(DESTLIB)(misc.o): $(USRINC)/sys/stat.h
$(DESTLIB)(misc.o): $(USRINC)/sys/cred.h
$(DESTLIB)(misc.o): $(USRINC)/stddef.h
$(DESTLIB)(misc.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(misc.o): $(USRINC)/sys/proc.h
$(DESTLIB)(misc.o): $(USRINC)/sys/acct.h
$(DESTLIB)(misc.o): $(USRINC)/sys/ino.h
$(DESTLIB)(nlp.o): $(USRINC)/common/_tricks.h
$(DESTLIB)(nlp.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(nlp.o): $(USRINC)/common/xdebug.h
$(DESTLIB)(null.o): $(USRINC)/kernel/ddi_cpu.h
$(DESTLIB)(null.o): $(USRINC)/kernel/proc_lib.h
$(DESTLIB)(null.o): $(USRINC)/sys/errno.h
$(DESTLIB)(null.o): $(USRINC)/sys/file.h
$(DESTLIB)(null.o): $(USRINC)/sys/stat.h
$(DESTLIB)(null.o): $(USRINC)/sys/cred.h
$(DESTLIB)(null.o): $(USRINC)/sys/types.h
$(DESTLIB)(null.o): $(USRINC)/stddef.h
$(DESTLIB)(null.o): $(USRINC)/kernel/typed.h
$(DESTLIB)(null.o): $(USRINC)/sys/con.h
$(DESTLIB)(null.o): $(USRINC)/sys/inode.h
$(DESTLIB)(null.o): $(USRINC)/sys/seg.h
$(DESTLIB)(null.o): $(USRINC)/sys/coh_ps.h
$(DESTLIB)(null.o): $(USRINC)/sys/io.h
$(DESTLIB)(null.o): $(USRINC)/sys/proc.h
$(DESTLIB)(null.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(null.o): $(USRINC)/sys/mmu.h
$(DESTLIB)(pipe.o): $(USRINC)/kernel/_sleep.h
$(DESTLIB)(pipe.o): $(USRINC)/kernel/proc_lib.h
$(DESTLIB)(pipe.o): $(USRINC)/sys/errno.h
$(DESTLIB)(pipe.o): $(USRINC)/sys/file.h
$(DESTLIB)(pipe.o): $(USRINC)/sys/cmn_err.h
$(DESTLIB)(pipe.o): $(USRINC)/signal.h
$(DESTLIB)(pipe.o): $(USRINC)/stddef.h
$(DESTLIB)(pipe.o): $(USRINC)/limits.h
$(DESTLIB)(pipe.o): $(USRINC)/sys/filsys.h
$(DESTLIB)(pipe.o): $(USRINC)/sys/ino.h
$(DESTLIB)(pipe.o): $(USRINC)/sys/inode.h
$(DESTLIB)(pipe.o): $(USRINC)/sys/io.h
$(DESTLIB)(pipe.o): $(USRINC)/sys/proc.h
$(DESTLIB)(pipe.o): $(USRINC)/sys/sched.h
$(DESTLIB)(printf.o): $(USRINC)/stddef.h
$(DESTLIB)(proc.o): $(USRINC)/common/_wait.h
$(DESTLIB)(proc.o): $(USRINC)/sys/types.h
$(DESTLIB)(proc.o): $(USRINC)/kernel/proc_lib.h
$(DESTLIB)(proc.o): $(USRINC)/kernel/sig_lib.h
$(DESTLIB)(proc.o): $(USRINC)/sys/cmn_err.h
$(DESTLIB)(proc.o): $(USRINC)/sys/ksynch.h
$(DESTLIB)(proc.o): $(USRINC)/sys/wait.h
$(DESTLIB)(proc.o): $(USRINC)/sys/stat.h
$(DESTLIB)(proc.o): $(USRINC)/sys/errno.h
$(DESTLIB)(proc.o): $(USRINC)/stddef.h
$(DESTLIB)(proc.o): $(USRINC)/signal.h
$(DESTLIB)(proc.o): $(USRINC)/limits.h
$(DESTLIB)(proc.o): $(USRINC)/kernel/_timers.h
$(DESTLIB)(proc.o): $(USRINC)/kernel/ddi_cpu.h
$(DESTLIB)(proc.o): $(USRINC)/kernel/trace.h
$(DESTLIB)(proc.o): $(USRINC)/kernel/reg.h
$(DESTLIB)(proc.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(proc.o): $(USRINC)/sys/proc.h
$(DESTLIB)(proc.o): $(USRINC)/sys/mmu.h
$(DESTLIB)(proc.o): $(USRINC)/sys/acct.h
$(DESTLIB)(proc.o): $(USRINC)/sys/inode.h
$(DESTLIB)(proc.o): $(USRINC)/sys/ptrace.h
$(DESTLIB)(proc.o): $(USRINC)/sys/sched.h
$(DESTLIB)(rlock.o): $(USRINC)/kernel/_sleep.h
$(DESTLIB)(rlock.o): $(USRINC)/kernel/proc_lib.h
$(DESTLIB)(rlock.o): $(USRINC)/sys/errno.h
$(DESTLIB)(rlock.o): $(USRINC)/sys/file.h
$(DESTLIB)(rlock.o): $(USRINC)/fcntl.h
$(DESTLIB)(rlock.o): $(USRINC)/stddef.h
$(DESTLIB)(rlock.o): $(USRINC)/unistd.h
$(DESTLIB)(rlock.o): $(USRINC)/kernel/alloc.h
$(DESTLIB)(rlock.o): $(USRINC)/sys/fd.h
$(DESTLIB)(rlock.o): $(USRINC)/sys/proc.h
$(DESTLIB)(rlock.o): $(USRINC)/sys/sched.h
$(DESTLIB)(rlock.o): $(USRINC)/sys/inode.h
$(DESTLIB)(seg.o): $(USRINC)/sys/errno.h
$(DESTLIB)(seg.o): $(USRINC)/stddef.h
$(DESTLIB)(seg.o): $(USRINC)/kernel/alloc.h
$(DESTLIB)(seg.o): $(USRINC)/kernel/trace.h
$(DESTLIB)(seg.o): $(USRINC)/kernel/reg.h
$(DESTLIB)(seg.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(seg.o): $(USRINC)/sys/proc.h
$(DESTLIB)(seg.o): $(USRINC)/sys/mmu.h
$(DESTLIB)(seg.o): $(USRINC)/sys/buf.h
$(DESTLIB)(seg.o): $(USRINC)/sys/ino.h
$(DESTLIB)(seg.o): $(USRINC)/sys/inode.h
$(DESTLIB)(seg.o): $(USRINC)/sys/proc.h
$(DESTLIB)(seg.o): $(USRINC)/sys/sched.h
$(DESTLIB)(seg.o): $(USRINC)/sys/seg.h
$(DESTLIB)(seg.o): $(USRINC)/a.out.h
$(DESTLIB)(sig.o): $(USRINC)/common/__parith.h
$(DESTLIB)(sig.o): $(USRINC)/common/_limits.h
$(DESTLIB)(sig.o): $(USRINC)/common/_wait.h
$(DESTLIB)(sig.o): $(USRINC)/common/_gregset.h
$(DESTLIB)(sig.o): $(USRINC)/kernel/sig_lib.h
$(DESTLIB)(sig.o): $(USRINC)/kernel/proc_lib.h
$(DESTLIB)(sig.o): $(USRINC)/kernel/cred_lib.h
$(DESTLIB)(sig.o): $(USRINC)/kernel/reg.h
$(DESTLIB)(sig.o): $(USRINC)/sys/errno.h
$(DESTLIB)(sig.o): $(USRINC)/sys/file.h
$(DESTLIB)(sig.o): $(USRINC)/sys/cmn_err.h
$(DESTLIB)(sig.o): $(USRINC)/sys/cred.h
$(DESTLIB)(sig.o): $(USRINC)/stddef.h
$(DESTLIB)(sig.o): $(USRINC)/kernel/trace.h
$(DESTLIB)(sig.o): $(USRINC)/kernel/reg.h
$(DESTLIB)(sig.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(sig.o): $(USRINC)/sys/mmu.h
$(DESTLIB)(sig.o): $(USRINC)/sys/ino.h
$(DESTLIB)(sig.o): $(USRINC)/sys/inode.h
$(DESTLIB)(sig.o): $(USRINC)/sys/io.h
$(DESTLIB)(sig.o): $(USRINC)/sys/proc.h
$(DESTLIB)(sig.o): $(USRINC)/sys/ptrace.h
$(DESTLIB)(sig.o): $(USRINC)/sys/sched.h
$(DESTLIB)(sig.o): $(USRINC)/sys/seg.h
$(DESTLIB)(sig.o): $(USRINC)/sys/core.h
$(DESTLIB)(support.o): $(USRINC)/sys/al.h
$(DESTLIB)(sys1.o): $(USRINC)/common/_gregset.h
$(DESTLIB)(sys1.o): $(USRINC)/kernel/sig_lib.h
$(DESTLIB)(sys1.o): $(USRINC)/kernel/proc_lib.h
$(DESTLIB)(sys1.o): $(USRINC)/kernel/cred_lib.h
$(DESTLIB)(sys1.o): $(USRINC)/kernel/reg.h
$(DESTLIB)(sys1.o): $(USRINC)/sys/wait.h
$(DESTLIB)(sys1.o): $(USRINC)/sys/errno.h
$(DESTLIB)(sys1.o): $(USRINC)/sys/times.h
$(DESTLIB)(sys1.o): $(USRINC)/signal.h
$(DESTLIB)(sys1.o): $(USRINC)/sys/stat.h
$(DESTLIB)(sys1.o): $(USRINC)/stddef.h
$(DESTLIB)(sys1.o): $(USRINC)/unistd.h
$(DESTLIB)(sys1.o): $(USRINC)/kernel/_timers.h
$(DESTLIB)(sys1.o): $(USRINC)/kernel/trace.h
$(DESTLIB)(sys1.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(sys1.o): $(USRINC)/sys/acct.h
$(DESTLIB)(sys1.o): $(USRINC)/sys/con.h
$(DESTLIB)(sys1.o): $(USRINC)/sys/proc.h
$(DESTLIB)(sys1.o): $(USRINC)/sys/sched.h
$(DESTLIB)(sys1.o): $(USRINC)/sys/seg.h
$(DESTLIB)(sys1.o): $(USRINC)/sys/cred.h
$(DESTLIB)(sys2.o): $(USRINC)/common/_gregset.h
$(DESTLIB)(sys2.o): $(USRINC)/common/whence.h
$(DESTLIB)(sys2.o): $(USRINC)/kernel/_sleep.h
$(DESTLIB)(sys2.o): $(USRINC)/kernel/proc_lib.h
$(DESTLIB)(sys2.o): $(USRINC)/kernel/cred_lib.h
$(DESTLIB)(sys2.o): $(USRINC)/sys/errno.h
$(DESTLIB)(sys2.o): $(USRINC)/sys/stat.h
$(DESTLIB)(sys2.o): $(USRINC)/sys/file.h
$(DESTLIB)(sys2.o): $(USRINC)/sys/cred.h
$(DESTLIB)(sys2.o): $(USRINC)/stddef.h
$(DESTLIB)(sys2.o): $(USRINC)/fcntl.h
$(DESTLIB)(sys2.o): $(USRINC)/kernel/_timers.h
$(DESTLIB)(sys2.o): $(USRINC)/kernel/trace.h
$(DESTLIB)(sys2.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(sys2.o): $(USRINC)/sys/proc.h
$(DESTLIB)(sys2.o): $(USRINC)/sys/fd.h
$(DESTLIB)(sys2.o): $(USRINC)/sys/ino.h
$(DESTLIB)(sys2.o): $(USRINC)/sys/inode.h
$(DESTLIB)(sys2.o): $(USRINC)/sys/mount.h
$(DESTLIB)(sys2.o): $(USRINC)/sys/sched.h
$(DESTLIB)(sys3.o): $(USRINC)/kernel/proc_lib.h
$(DESTLIB)(sys3.o): $(USRINC)/sys/stat.h
$(DESTLIB)(sys3.o): $(USRINC)/sys/errno.h
$(DESTLIB)(sys3.o): $(USRINC)/fcntl.h
$(DESTLIB)(sys3.o): $(USRINC)/stddef.h
$(DESTLIB)(sys3.o): $(USRINC)/kernel/trace.h
$(DESTLIB)(sys3.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(sys3.o): $(USRINC)/sys/proc.h
$(DESTLIB)(sys3.o): $(USRINC)/sys/buf.h
$(DESTLIB)(sys3.o): $(USRINC)/sys/con.h
$(DESTLIB)(sys3.o): $(USRINC)/sys/fd.h
$(DESTLIB)(sys3.o): $(USRINC)/sys/filsys.h
$(DESTLIB)(sys3.o): $(USRINC)/sys/ino.h
$(DESTLIB)(sys3.o): $(USRINC)/sys/inode.h
$(DESTLIB)(sys3.o): $(USRINC)/sys/io.h
$(DESTLIB)(sys3.o): $(USRINC)/sys/mount.h
$(DESTLIB)(sys3.o): $(USRINC)/sys/file.h
$(DESTLIB)(sys4.o): $(USRINC)/sgtty.h
$(DESTLIB)(sys5.o): $(USRINC)/kernel/proc_lib.h
$(DESTLIB)(sys5.o): $(USRINC)/sys/errno.h
$(DESTLIB)(sys5.o): $(USRINC)/sys/stat.h
$(DESTLIB)(sys5.o): $(USRINC)/sys/file.h
$(DESTLIB)(sys5.o): $(USRINC)/ulimit.h
$(DESTLIB)(sys5.o): $(USRINC)/dirent.h
$(DESTLIB)(sys5.o): $(USRINC)/fcntl.h
$(DESTLIB)(sys5.o): $(USRINC)/stddef.h
$(DESTLIB)(sys5.o): $(USRINC)/limits.h
$(DESTLIB)(sys5.o): $(USRINC)/kernel/alloc.h
$(DESTLIB)(sys5.o): $(USRINC)/kernel/trace.h
$(DESTLIB)(sys5.o): $(USRINC)/kernel/reg.h
$(DESTLIB)(sys5.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(sys5.o): $(USRINC)/sys/proc.h
$(DESTLIB)(sys5.o): $(USRINC)/sys/mmu.h
$(DESTLIB)(sys5.o): $(USRINC)/sys/buf.h
$(DESTLIB)(sys5.o): $(USRINC)/canon.h
$(DESTLIB)(sys5.o): $(USRINC)/sys/con.h
$(DESTLIB)(sys5.o): $(USRINC)/sys/fd.h
$(DESTLIB)(sys5.o): $(USRINC)/sys/filsys.h
$(DESTLIB)(sys5.o): $(USRINC)/sys/ino.h
$(DESTLIB)(sys5.o): $(USRINC)/sys/inode.h
$(DESTLIB)(sys5.o): $(USRINC)/sys/io.h
$(DESTLIB)(sys5.o): $(USRINC)/sys/mount.h
$(DESTLIB)(sys5.o): $(USRINC)/sys/utsname.h
$(DESTLIB)(sys5.o): $(USRINC)/sys/mount.h
$(DESTLIB)(sys5.o): $(USRINC)/ustat.h
$(DESTLIB)(sys5.o): $(USRINC)/sys/statfs.h
$(DESTLIB)(sys5.o): $(USRINC)/sys/sysi86.h
$(DESTLIB)(sys6.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(sys6.o): $(USRINC)/common/_tricks.h
$(DESTLIB)(sys6.o): $(USRINC)/common/_clktck.h
$(DESTLIB)(sys6.o): $(USRINC)/common/_gregset.h
$(DESTLIB)(sys6.o): $(USRINC)/kernel/sig_lib.h
$(DESTLIB)(sys6.o): $(USRINC)/kernel/proc_lib.h
$(DESTLIB)(sys6.o): $(USRINC)/kernel/cred_lib.h
$(DESTLIB)(sys6.o): $(USRINC)/sys/signal.h
$(DESTLIB)(sys6.o): $(USRINC)/sys/errno.h
$(DESTLIB)(sys6.o): $(USRINC)/sys/stat.h
$(DESTLIB)(sys6.o): $(USRINC)/sys/poll.h
$(DESTLIB)(sys6.o): $(USRINC)/sys/cmn_err.h
$(DESTLIB)(sys6.o): $(USRINC)/sys/cred.h
$(DESTLIB)(sys6.o): $(USRINC)/unistd.h
$(DESTLIB)(sys6.o): $(USRINC)/limits.h
$(DESTLIB)(sys6.o): $(USRINC)/stddef.h
$(DESTLIB)(sys6.o): $(USRINC)/poll.h
$(DESTLIB)(sys6.o): $(USRINC)/sys/sched.h
$(DESTLIB)(sys6.o): $(USRINC)/kernel/dir.h
$(DESTLIB)(sys6.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(sys6.o): $(USRINC)/sys/proc.h
$(DESTLIB)(sys6.o): $(USRINC)/sys/inode.h
$(DESTLIB)(sys6.o): $(USRINC)/sys/io.h
$(DESTLIB)(sys6.o): $(USRINC)/sys/fd.h
$(DESTLIB)(syscoh.o): $(USRINC)/kernel/proc_lib.h
$(DESTLIB)(syscoh.o): $(USRINC)/sys/errno.h
$(DESTLIB)(syscoh.o): $(USRINC)/kernel/trace.h
$(DESTLIB)(syscoh.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(syscoh.o): $(USRINC)/sys/con.h
$(DESTLIB)(syscoh.o): $(USRINC)/sys/types.h
$(DESTLIB)(timeout.o): $(USRINC)/common/_tricks.h
$(DESTLIB)(timeout.o): $(USRINC)/kernel/timeout.h
$(DESTLIB)(timeout.o): $(USRINC)/kernel/_timers.h
$(DESTLIB)(timeout.o): $(USRINC)/stddef.h
$(DESTLIB)(v_proc.o): $(USRINC)/common/feature.h
$(DESTLIB)(v_proc.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(v_proc.o): $(USRINC)/common/xdebug.h
$(DESTLIB)(v_proc.o): $(USRINC)/common/_siginfo.h
$(DESTLIB)(v_proc.o): $(USRINC)/common/_tricks.h
$(DESTLIB)(v_proc.o): $(USRINC)/kernel/ddi_proc.h
$(DESTLIB)(v_proc.o): $(USRINC)/kernel/ddi_glob.h
$(DESTLIB)(v_proc.o): $(USRINC)/kernel/ddi_lock.h
$(DESTLIB)(v_proc.o): $(USRINC)/kernel/sig_lib.h
$(DESTLIB)(v_proc.o): $(USRINC)/sys/cmn_err.h
$(DESTLIB)(v_proc.o): $(USRINC)/sys/debug.h
$(DESTLIB)(v_proc.o): $(USRINC)/sys/types.h
$(DESTLIB)(v_proc.o): $(USRINC)/sys/inline.h
$(DESTLIB)(v_proc.o): $(USRINC)/sys/ksynch.h
$(DESTLIB)(v_proc.o): $(USRINC)/sys/signal.h
$(DESTLIB)(v_proc.o): $(USRINC)/stddef.h
$(DESTLIB)(v_proc.o): $(USRINC)/kernel/v_proc.h
$(DESTLIB)(v_proc.o): $(USRINC)/sys/sched.h
$(DESTLIB)(v_proc.o): $(USRINC)/sys/proc.h
$(DESTLIB)(var.o): $(USRINC)/stddef.h
$(DESTLIB)(var.o): $(USRINC)/kernel/timeout.h
$(DESTLIB)(var.o): $(USRINC)/kernel/systab.h
$(DESTLIB)(var.o): $(USRINC)/kernel/_timers.h
$(DESTLIB)(var.o): $(USRINC)/kernel/trace.h
$(DESTLIB)(var.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(var.o): $(USRINC)/sys/buf.h
$(DESTLIB)(var.o): $(USRINC)/sys/con.h
$(DESTLIB)(var.o): $(USRINC)/sys/inode.h
$(DESTLIB)(var.o): $(USRINC)/sys/mount.h
$(DESTLIB)(var.o): $(USRINC)/sys/proc.h
$(DESTLIB)(var.o): $(USRINC)/sys/ptrace.h
$(DESTLIB)(var.o): $(USRINC)/sys/seg.h
$(DESTLIB)(var.o): $(USRINC)/sys/mmu.h
$(DESTLIB)(var.o): $(USRINC)/sys/ino.h

# DEPENDENCIES MUST END AT END OF FILE: IF YOU PUT STUFF HERE, IT WILL GO AWAY
# See make depend, above
