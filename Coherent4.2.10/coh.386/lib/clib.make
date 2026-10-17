# Sub-makefile for batching up compilation and archiving.

USRINC=/usr/include
DESTLIB=$(K386LIB)/k386.a
MAKEFILE=clib.make

OBJS=   $(DESTLIB)(proc_destroy.o) $(DESTLIB)(proc_find.o) \
	$(DESTLIB)(proc_init.o)	$(DESTLIB)(proc_rderr.o) \
	$(DESTLIB)(proc_setrun.o) $(DESTLIB)(proc_sdel.o) \
	$(DESTLIB)(proc_smask.o) $(DESTLIB)(proc_smisc.o) \
	$(DESTLIB)(proc_ssig.o) $(DESTLIB)(proc_wake.o) \
	$(DESTLIB)(proc_pid.o) \
\
	$(DESTLIB)(ddi_alloc.o) $(DESTLIB)(ddi_base.o) $(DESTLIB)(ddi_cpu.o) \
	$(DESTLIB)(ddi_data.o) $(DESTLIB)(ddi_defer.o) \
	$(DESTLIB)(ddi_devno.o) $(DESTLIB)(ddi_drv.o) $(DESTLIB)(ddi_getp.o) \
	$(DESTLIB)(ddi_glob.o) $(DESTLIB)(ddi_hz.o) $(DESTLIB)(ddi_id.o) \
	$(DESTLIB)(ddi_init.o) $(DESTLIB)(ddi_poll.o) $(DESTLIB)(ddi_priv.o) \
	$(DESTLIB)(ddi_proc.o) $(DESTLIB)(ddi_ref.o) $(DESTLIB)(ddi_run.o) \
	$(DESTLIB)(ddi_setp.o) $(DESTLIB)(ddi_time.o) \
	$(DESTLIB)(ddi_unref.o) \
\
	$(DESTLIB)(ipc_cred.o) $(DESTLIB)(ipc_perm.o) $(DESTLIB)(ipc_init.o) \
\
	$(DESTLIB)(cred_copy.o) $(DESTLIB)(cred_exec.o) \
	$(DESTLIB)(cred_fake.o) $(DESTLIB)(cred_groups.o) \
	$(DESTLIB)(cred_match.o) $(DESTLIB)(cred_mem.o) \
	$(DESTLIB)(cred_newgid.o) $(DESTLIB)(cred_newuid.o) \
	$(DESTLIB)(cred_setgrp.o) \
\
	$(DESTLIB)(kmem_alloc.o) $(DESTLIB)(kmem_free.o) \
	$(DESTLIB)(kmem_zalloc.o) $(DESTLIB)(kmem_init.o) \
\
	$(DESTLIB)(sig_act.o) $(DESTLIB)(sig_mask.o) \
	$(DESTLIB)(sig_misc.o) $(DESTLIB)(sig_pend.o) \
\
	$(DESTLIB)(st_alloc.o) $(DESTLIB)(st_alloc.o) \
\
	$(DESTLIB)(intr_init.o) $(DESTLIB)(ksynch.o) $(DESTLIB)(reg_dump.o) \
	$(DESTLIB)(posix_time.o) $(DESTLIB)(cmn_err.o) \
	$(DESTLIB)(uio.o) $(DESTLIB)(assert.o)

# Removed from the library while testing trace features.
#	$(DESTLIB)(cmn_put.o) \

.c.a:
	echo $< >>sources; echo $% >> members

all:	prelude $(OBJS) postlude

prelude:
	echo > sources; echo > members

postlude:
	@set -x; \
	case "$${members=$$(cat members)}" in \
	"")	;; \
	*)	case "$${sources=$$(cat sources)}" in \
		"")	;; \
		*)	echo Compiling $$sources; \
			$(CC) $(CFLAGS) -c $$sources || exit 1 ;; \
		esac; \
		echo Archiving $$members ; \
		$(AR) $(ARFLAGS) $(DESTLIB) $$members || exit 1; \
	esac

#		rm -f $$members ;; \
#	rm sources; rm members

$(DESTLIB)(st_alloc.o): st_alloc.c
	$(CC) $(CFLAGS) -DNDEBUG -o $% -c $*.c
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

$(DESTLIB)(assert.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(assert.o): $(USRINC)/sys/cmn_err.h
$(DESTLIB)(assert.o): $(USRINC)/stddef.h
$(DESTLIB)(assert.o): $(USRINC)/sys/debug.h
$(DESTLIB)(cmn_err.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(cmn_err.o): $(USRINC)/common/xdebug.h
$(DESTLIB)(cmn_err.o): $(USRINC)/sys/inline.h
$(DESTLIB)(cmn_err.o): $(USRINC)/sys/ksynch.h
$(DESTLIB)(cmn_err.o): $(USRINC)/sys/ddi.h
$(DESTLIB)(cmn_err.o): $(USRINC)/sys/types.h
$(DESTLIB)(cmn_err.o): $(USRINC)/limits.h
$(DESTLIB)(cmn_err.o): $(USRINC)/stdarg.h
$(DESTLIB)(cmn_err.o): $(USRINC)/stddef.h
$(DESTLIB)(cmn_err.o): $(USRINC)/sys/cmn_err.h
$(DESTLIB)(cmn_put.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(cmn_put.o): $(USRINC)/common/xdebug.h
$(DESTLIB)(cred_copy.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(cred_copy.o): $(USRINC)/kernel/cred_lib.h
$(DESTLIB)(cred_copy.o): $(USRINC)/sys/debug.h
$(DESTLIB)(cred_copy.o): $(USRINC)/sys/types.h
$(DESTLIB)(cred_copy.o): $(USRINC)/sys/cred.h
$(DESTLIB)(cred_copy.o): $(USRINC)/stddef.h
$(DESTLIB)(cred_exec.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(cred_exec.o): $(USRINC)/kernel/cred_lib.h
$(DESTLIB)(cred_exec.o): $(USRINC)/sys/debug.h
$(DESTLIB)(cred_exec.o): $(USRINC)/sys/types.h
$(DESTLIB)(cred_exec.o): $(USRINC)/sys/cred.h
$(DESTLIB)(cred_exec.o): $(USRINC)/stddef.h
$(DESTLIB)(cred_fake.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(cred_fake.o): $(USRINC)/kernel/cred_lib.h
$(DESTLIB)(cred_fake.o): $(USRINC)/sys/debug.h
$(DESTLIB)(cred_fake.o): $(USRINC)/sys/types.h
$(DESTLIB)(cred_fake.o): $(USRINC)/sys/cred.h
$(DESTLIB)(cred_fake.o): $(USRINC)/sys/kmem.h
$(DESTLIB)(cred_groups.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(cred_groups.o): $(USRINC)/kernel/cred_lib.h
$(DESTLIB)(cred_groups.o): $(USRINC)/sys/debug.h
$(DESTLIB)(cred_groups.o): $(USRINC)/sys/types.h
$(DESTLIB)(cred_groups.o): $(USRINC)/sys/cred.h
$(DESTLIB)(cred_groups.o): $(USRINC)/sys/kmem.h
$(DESTLIB)(cred_match.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(cred_match.o): $(USRINC)/common/_imode.h
$(DESTLIB)(cred_match.o): $(USRINC)/kernel/cred_lib.h
$(DESTLIB)(cred_match.o): $(USRINC)/sys/debug.h
$(DESTLIB)(cred_match.o): $(USRINC)/sys/types.h
$(DESTLIB)(cred_match.o): $(USRINC)/sys/cred.h
$(DESTLIB)(cred_match.o): $(USRINC)/stddef.h
$(DESTLIB)(cred_mem.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(cred_mem.o): $(USRINC)/kernel/cred_lib.h
$(DESTLIB)(cred_mem.o): $(USRINC)/sys/debug.h
$(DESTLIB)(cred_mem.o): $(USRINC)/sys/types.h
$(DESTLIB)(cred_mem.o): $(USRINC)/sys/cred.h
$(DESTLIB)(cred_mem.o): $(USRINC)/sys/kmem.h
$(DESTLIB)(cred_newgid.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(cred_newgid.o): $(USRINC)/kernel/cred_lib.h
$(DESTLIB)(cred_newgid.o): $(USRINC)/sys/debug.h
$(DESTLIB)(cred_newgid.o): $(USRINC)/sys/types.h
$(DESTLIB)(cred_newgid.o): $(USRINC)/sys/cred.h
$(DESTLIB)(cred_newgid.o): $(USRINC)/stddef.h
$(DESTLIB)(cred_newuid.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(cred_newuid.o): $(USRINC)/kernel/cred_lib.h
$(DESTLIB)(cred_newuid.o): $(USRINC)/sys/debug.h
$(DESTLIB)(cred_newuid.o): $(USRINC)/sys/types.h
$(DESTLIB)(cred_newuid.o): $(USRINC)/sys/cred.h
$(DESTLIB)(cred_newuid.o): $(USRINC)/stddef.h
$(DESTLIB)(cred_setgrp.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(cred_setgrp.o): $(USRINC)/kernel/cred_lib.h
$(DESTLIB)(cred_setgrp.o): $(USRINC)/sys/debug.h
$(DESTLIB)(cred_setgrp.o): $(USRINC)/sys/types.h
$(DESTLIB)(cred_setgrp.o): $(USRINC)/sys/cred.h
$(DESTLIB)(cred_setgrp.o): $(USRINC)/stddef.h
$(DESTLIB)(ddi_alloc.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(ddi_alloc.o): $(USRINC)/stddef.h
$(DESTLIB)(ddi_alloc.o): $(USRINC)/string.h
$(DESTLIB)(ddi_alloc.o): $(USRINC)/kernel/ddi_cpu.h
$(DESTLIB)(ddi_base.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(ddi_base.o): $(USRINC)/kernel/ddi_cpu.h
$(DESTLIB)(ddi_base.o): $(USRINC)/kernel/ddi_base.h
$(DESTLIB)(ddi_base.o): $(USRINC)/sys/io.h
$(DESTLIB)(ddi_base.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(ddi_cpu.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(ddi_cpu.o): $(USRINC)/kernel/ddi_cpu.h
$(DESTLIB)(ddi_data.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(ddi_data.o): $(USRINC)/kernel/ddi_cpu.h
$(DESTLIB)(ddi_data.o): $(USRINC)/kernel/ddi_glob.h
$(DESTLIB)(ddi_defer.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(ddi_defer.o): $(USRINC)/kernel/ddi_cpu.h
$(DESTLIB)(ddi_defer.o): $(USRINC)/kernel/ddi_glob.h
$(DESTLIB)(ddi_defer.o): $(USRINC)/kernel/ddi_lock.h
$(DESTLIB)(ddi_defer.o): $(USRINC)/sys/debug.h
$(DESTLIB)(ddi_defer.o): $(USRINC)/sys/ksynch.h
$(DESTLIB)(ddi_defer.o): $(USRINC)/sys/cmn_err.h
$(DESTLIB)(ddi_defer.o): $(USRINC)/sys/types.h
$(DESTLIB)(ddi_defer.o): $(USRINC)/stddef.h
$(DESTLIB)(ddi_defer.o): $(USRINC)/kernel/defer.h
$(DESTLIB)(ddi_devno.o): $(USRINC)/sys/types.h
$(DESTLIB)(ddi_devno.o): $(USRINC)/limits.h
$(DESTLIB)(ddi_drv.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(ddi_drv.o): $(USRINC)/common/xdebug.h
$(DESTLIB)(ddi_drv.o): $(USRINC)/kernel/strmlib.h
$(DESTLIB)(ddi_drv.o): $(USRINC)/sys/debug.h
$(DESTLIB)(ddi_drv.o): $(USRINC)/sys/types.h
$(DESTLIB)(ddi_drv.o): $(USRINC)/sys/cred.h
$(DESTLIB)(ddi_drv.o): $(USRINC)/sys/cmn_err.h
$(DESTLIB)(ddi_drv.o): $(USRINC)/sys/file.h
$(DESTLIB)(ddi_drv.o): $(USRINC)/sys/open.h
$(DESTLIB)(ddi_drv.o): $(USRINC)/sys/errno.h
$(DESTLIB)(ddi_drv.o): $(USRINC)/sys/uio.h
$(DESTLIB)(ddi_drv.o): $(USRINC)/sys/poll.h
$(DESTLIB)(ddi_drv.o): $(USRINC)/stddef.h
$(DESTLIB)(ddi_drv.o): $(USRINC)/sys/confinfo.h
$(DESTLIB)(ddi_drv.o): $(USRINC)/kernel/ddi_base.h
$(DESTLIB)(ddi_drv.o): $(USRINC)/sys/stat.h
$(DESTLIB)(ddi_drv.o): $(USRINC)/sys/inode.h
$(DESTLIB)(ddi_getp.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(ddi_getp.o): $(USRINC)/sys/debug.h
$(DESTLIB)(ddi_getp.o): $(USRINC)/sys/types.h
$(DESTLIB)(ddi_getp.o): $(USRINC)/sys/cred.h
$(DESTLIB)(ddi_getp.o): $(USRINC)/stddef.h
$(DESTLIB)(ddi_getp.o): $(USRINC)/limits.h
$(DESTLIB)(ddi_getp.o): $(USRINC)/sys/types.h
$(DESTLIB)(ddi_getp.o): $(USRINC)/kernel/ddi_base.h
$(DESTLIB)(ddi_getp.o): $(USRINC)/kernel/param.h
$(DESTLIB)(ddi_getp.o): $(USRINC)/kernel/_timers.h
$(DESTLIB)(ddi_getp.o): $(USRINC)/sys/proc.h
$(DESTLIB)(ddi_glob.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(ddi_glob.o): $(USRINC)/kernel/ddi_glob.h
$(DESTLIB)(ddi_hz.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(ddi_hz.o): $(USRINC)/sys/debug.h
$(DESTLIB)(ddi_hz.o): $(USRINC)/sys/types.h
$(DESTLIB)(ddi_hz.o): $(USRINC)/stddef.h
$(DESTLIB)(ddi_hz.o): $(USRINC)/sys/types.h
$(DESTLIB)(ddi_hz.o): $(USRINC)/kernel/param.h
$(DESTLIB)(ddi_id.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(ddi_id.o): $(USRINC)/kernel/ddi_cpu.h
$(DESTLIB)(ddi_init.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(ddi_init.o): $(USRINC)/kernel/x86lock.h
$(DESTLIB)(ddi_init.o): $(USRINC)/kernel/ddi_lock.h
$(DESTLIB)(ddi_init.o): $(USRINC)/sys/types.h
$(DESTLIB)(ddi_init.o): $(USRINC)/sys/ksynch.h
$(DESTLIB)(ddi_init.o): $(USRINC)/sys/kmem.h
$(DESTLIB)(ddi_init.o): $(USRINC)/stddef.h
$(DESTLIB)(ddi_init.o): $(USRINC)/string.h
$(DESTLIB)(ddi_init.o): $(USRINC)/kernel/ddi_base.h
$(DESTLIB)(ddi_init.o): $(USRINC)/kernel/ddi_proc.h
$(DESTLIB)(ddi_init.o): $(USRINC)/kernel/ddi_cpu.h
$(DESTLIB)(ddi_init.o): $(USRINC)/kernel/ddi_glob.h
$(DESTLIB)(ddi_poll.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(ddi_poll.o): $(USRINC)/common/_tricks.h
$(DESTLIB)(ddi_poll.o): $(USRINC)/kernel/ddi_cpu.h
$(DESTLIB)(ddi_poll.o): $(USRINC)/kernel/ddi_glob.h
$(DESTLIB)(ddi_poll.o): $(USRINC)/kernel/ddi_lock.h
$(DESTLIB)(ddi_poll.o): $(USRINC)/sys/cmn_err.h
$(DESTLIB)(ddi_poll.o): $(USRINC)/sys/debug.h
$(DESTLIB)(ddi_poll.o): $(USRINC)/sys/kmem.h
$(DESTLIB)(ddi_poll.o): $(USRINC)/sys/ksynch.h
$(DESTLIB)(ddi_poll.o): $(USRINC)/sys/errno.h
$(DESTLIB)(ddi_poll.o): $(USRINC)/sys/types.h
$(DESTLIB)(ddi_poll.o): $(USRINC)/sys/inline.h
$(DESTLIB)(ddi_poll.o): $(USRINC)/stddef.h
$(DESTLIB)(ddi_poll.o): $(USRINC)/limits.h
$(DESTLIB)(ddi_poll.o): $(USRINC)/common/_poll.h
$(DESTLIB)(ddi_poll.o): $(USRINC)/sys/poll.h
$(DESTLIB)(ddi_poll.o): $(USRINC)/poll.h
$(DESTLIB)(ddi_poll.o): $(USRINC)/stropts.h
$(DESTLIB)(ddi_priv.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(ddi_priv.o): $(USRINC)/sys/debug.h
$(DESTLIB)(ddi_priv.o): $(USRINC)/sys/types.h
$(DESTLIB)(ddi_priv.o): $(USRINC)/sys/cred.h
$(DESTLIB)(ddi_priv.o): $(USRINC)/stddef.h
$(DESTLIB)(ddi_priv.o): $(USRINC)/sys/types.h
$(DESTLIB)(ddi_proc.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(ddi_proc.o): $(USRINC)/kernel/ddi_proc.h
$(DESTLIB)(ddi_proc.o): $(USRINC)/sys/proc.h
$(DESTLIB)(ddi_ref.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(ddi_ref.o): $(USRINC)/stddef.h
$(DESTLIB)(ddi_ref.o): $(USRINC)/kernel/ddi_cpu.h
$(DESTLIB)(ddi_run.o): $(USRINC)/kernel/ddi_cpu.h
$(DESTLIB)(ddi_run.o): $(USRINC)/kernel/ddi_glob.h
$(DESTLIB)(ddi_run.o): $(USRINC)/sys/confinfo.h
$(DESTLIB)(ddi_run.o): $(USRINC)/sys/types.h
$(DESTLIB)(ddi_run.o): $(USRINC)/sys/cmn_err.h
$(DESTLIB)(ddi_run.o): $(USRINC)/stdlib.h
$(DESTLIB)(ddi_setp.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(ddi_setp.o): $(USRINC)/sys/types.h
$(DESTLIB)(ddi_setp.o): $(USRINC)/sys/ddi.h
$(DESTLIB)(ddi_spl.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(ddi_spl.o): $(USRINC)/sys/inline.h
$(DESTLIB)(ddi_spl.o): $(USRINC)/sys/types.h
$(DESTLIB)(ddi_time.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(ddi_time.o): $(USRINC)/kernel/ddi_glob.h
$(DESTLIB)(ddi_time.o): $(USRINC)/kernel/ddi_cpu.h
$(DESTLIB)(ddi_time.o): $(USRINC)/kernel/ddi_lock.h
$(DESTLIB)(ddi_time.o): $(USRINC)/kernel/defer.h
$(DESTLIB)(ddi_time.o): $(USRINC)/sys/debug.h
$(DESTLIB)(ddi_time.o): $(USRINC)/sys/types.h
$(DESTLIB)(ddi_time.o): $(USRINC)/sys/inline.h
$(DESTLIB)(ddi_time.o): $(USRINC)/sys/ksynch.h
$(DESTLIB)(ddi_time.o): $(USRINC)/sys/kmem.h
$(DESTLIB)(ddi_time.o): $(USRINC)/sys/cmn_err.h
$(DESTLIB)(ddi_unref.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(ddi_unref.o): $(USRINC)/stddef.h
$(DESTLIB)(ddi_unref.o): $(USRINC)/kernel/ddi_cpu.h
$(DESTLIB)(intr_init.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(intr_init.o): $(USRINC)/kernel/ddi_lock.h
$(DESTLIB)(intr_init.o): $(USRINC)/sys/types.h
$(DESTLIB)(intr_init.o): $(USRINC)/sys/ksynch.h
$(DESTLIB)(intr_init.o): $(USRINC)/stddef.h
$(DESTLIB)(intr_init.o): $(USRINC)/kernel/ddi_cpu.h
$(DESTLIB)(intr_init.o): $(USRINC)/sys/inline.h
$(DESTLIB)(ipc_cred.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(ipc_cred.o): $(USRINC)/common/_imode.h
$(DESTLIB)(ipc_cred.o): $(USRINC)/sys/debug.h
$(DESTLIB)(ipc_cred.o): $(USRINC)/sys/cred.h
$(DESTLIB)(ipc_cred.o): $(USRINC)/stddef.h
$(DESTLIB)(ipc_cred.o): $(USRINC)/sys/ipc.h
$(DESTLIB)(ipc_cred.o): $(USRINC)/sys/proc.h
$(DESTLIB)(ipc_init.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(ipc_init.o): $(USRINC)/common/_imode.h
$(DESTLIB)(ipc_init.o): $(USRINC)/sys/debug.h
$(DESTLIB)(ipc_init.o): $(USRINC)/sys/cred.h
$(DESTLIB)(ipc_init.o): $(USRINC)/stddef.h
$(DESTLIB)(ipc_init.o): $(USRINC)/sys/ipc.h
$(DESTLIB)(ipc_init.o): $(USRINC)/sys/proc.h
$(DESTLIB)(ipc_perm.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(ipc_perm.o): $(USRINC)/common/_imode.h
$(DESTLIB)(ipc_perm.o): $(USRINC)/sys/debug.h
$(DESTLIB)(ipc_perm.o): $(USRINC)/sys/cmn_err.h
$(DESTLIB)(ipc_perm.o): $(USRINC)/sys/errno.h
$(DESTLIB)(ipc_perm.o): $(USRINC)/sys/ipc.h
$(DESTLIB)(ipc_perm.o): $(USRINC)/stddef.h
$(DESTLIB)(ipc_perm.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(kmem_alloc.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(kmem_alloc.o): $(USRINC)/kernel/ddi_lock.h
$(DESTLIB)(kmem_alloc.o): $(USRINC)/kernel/ddi_glob.h
$(DESTLIB)(kmem_alloc.o): $(USRINC)/kernel/st_alloc.h
$(DESTLIB)(kmem_alloc.o): $(USRINC)/sys/types.h
$(DESTLIB)(kmem_alloc.o): $(USRINC)/sys/debug.h
$(DESTLIB)(kmem_alloc.o): $(USRINC)/sys/ksynch.h
$(DESTLIB)(kmem_alloc.o): $(USRINC)/sys/kmem.h
$(DESTLIB)(kmem_free.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(kmem_free.o): $(USRINC)/kernel/ddi_lock.h
$(DESTLIB)(kmem_free.o): $(USRINC)/kernel/ddi_glob.h
$(DESTLIB)(kmem_free.o): $(USRINC)/kernel/st_alloc.h
$(DESTLIB)(kmem_free.o): $(USRINC)/sys/types.h
$(DESTLIB)(kmem_free.o): $(USRINC)/sys/debug.h
$(DESTLIB)(kmem_free.o): $(USRINC)/sys/ksynch.h
$(DESTLIB)(kmem_free.o): $(USRINC)/sys/cmn_err.h
$(DESTLIB)(kmem_free.o): $(USRINC)/sys/kmem.h
$(DESTLIB)(kmem_init.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(kmem_init.o): $(USRINC)/kernel/ddi_lock.h
$(DESTLIB)(kmem_init.o): $(USRINC)/kernel/ddi_glob.h
$(DESTLIB)(kmem_init.o): $(USRINC)/kernel/st_alloc.h
$(DESTLIB)(kmem_init.o): $(USRINC)/sys/types.h
$(DESTLIB)(kmem_init.o): $(USRINC)/sys/debug.h
$(DESTLIB)(kmem_init.o): $(USRINC)/sys/ksynch.h
$(DESTLIB)(kmem_init.o): $(USRINC)/sys/cmn_err.h
$(DESTLIB)(kmem_init.o): $(USRINC)/sys/kmem.h
$(DESTLIB)(kmem_realloc.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(kmem_realloc.o): $(USRINC)/kernel/st_alloc.h
$(DESTLIB)(kmem_realloc.o): $(USRINC)/sys/types.h
$(DESTLIB)(kmem_realloc.o): $(USRINC)/sys/ksynch.h
$(DESTLIB)(kmem_realloc.o): $(USRINC)/sys/kmem.h
$(DESTLIB)(kmem_zalloc.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(kmem_zalloc.o): $(USRINC)/sys/types.h
$(DESTLIB)(kmem_zalloc.o): $(USRINC)/string.h
$(DESTLIB)(kmem_zalloc.o): $(USRINC)/sys/kmem.h
$(DESTLIB)(ksynch.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(ksynch.o): $(USRINC)/common/_tricks.h
$(DESTLIB)(ksynch.o): $(USRINC)/kernel/ddi_cpu.h
$(DESTLIB)(ksynch.o): $(USRINC)/kernel/ddi_lock.h
$(DESTLIB)(ksynch.o): $(USRINC)/kernel/v_proc.h
$(DESTLIB)(ksynch.o): $(USRINC)/sys/debug.h
$(DESTLIB)(ksynch.o): $(USRINC)/sys/types.h
$(DESTLIB)(ksynch.o): $(USRINC)/sys/inline.h
$(DESTLIB)(ksynch.o): $(USRINC)/sys/cmn_err.h
$(DESTLIB)(ksynch.o): $(USRINC)/sys/kmem.h
$(DESTLIB)(ksynch.o): $(USRINC)/stddef.h
$(DESTLIB)(ksynch.o): $(USRINC)/sys/ksynch.h
$(DESTLIB)(posix_time.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(posix_time.o): $(USRINC)/common/_time.h
$(DESTLIB)(posix_time.o): $(USRINC)/kernel/_timers.h
$(DESTLIB)(posix_time.o): $(USRINC)/sys/inline.h
$(DESTLIB)(posix_time.o): $(USRINC)/sys/types.h
$(DESTLIB)(proc_destroy.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(proc_destroy.o): $(USRINC)/kernel/cred_lib.h
$(DESTLIB)(proc_destroy.o): $(USRINC)/sys/types.h
$(DESTLIB)(proc_destroy.o): $(USRINC)/sys/debug.h
$(DESTLIB)(proc_destroy.o): $(USRINC)/sys/kmem.h
$(DESTLIB)(proc_destroy.o): $(USRINC)/stdlib.h
$(DESTLIB)(proc_destroy.o): $(USRINC)/kernel/proc_lib.h
$(DESTLIB)(proc_destroy.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(proc_destroy.o): $(USRINC)/sys/proc.h
$(DESTLIB)(proc_find.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(proc_find.o): $(USRINC)/kernel/proc_lib.h
$(DESTLIB)(proc_find.o): $(USRINC)/sys/types.h
$(DESTLIB)(proc_find.o): $(USRINC)/stddef.h
$(DESTLIB)(proc_find.o): $(USRINC)/limits.h
$(DESTLIB)(proc_find.o): $(USRINC)/sys/proc.h
$(DESTLIB)(proc_init.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(proc_init.o): $(USRINC)/kernel/sig_lib.h
$(DESTLIB)(proc_init.o): $(USRINC)/kernel/cred_lib.h
$(DESTLIB)(proc_init.o): $(USRINC)/sys/types.h
$(DESTLIB)(proc_init.o): $(USRINC)/sys/kmem.h
$(DESTLIB)(proc_init.o): $(USRINC)/stdlib.h
$(DESTLIB)(proc_init.o): $(USRINC)/kernel/proc_lib.h
$(DESTLIB)(proc_init.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(proc_init.o): $(USRINC)/sys/proc.h
$(DESTLIB)(proc_pid.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(proc_pid.o): $(USRINC)/kernel/proc_lib.h
$(DESTLIB)(proc_pid.o): $(USRINC)/sys/proc.h
$(DESTLIB)(proc_rderr.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(proc_rderr.o): $(USRINC)/sys/errno.h
$(DESTLIB)(proc_rderr.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(proc_sdel.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(proc_sdel.o): $(USRINC)/kernel/sig_lib.h
$(DESTLIB)(proc_sdel.o): $(USRINC)/sys/debug.h
$(DESTLIB)(proc_sdel.o): $(USRINC)/stddef.h
$(DESTLIB)(proc_sdel.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(proc_sdel.o): $(USRINC)/sys/proc.h
$(DESTLIB)(proc_setrun.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(proc_setrun.o): $(USRINC)/kernel/sig_lib.h
$(DESTLIB)(proc_setrun.o): $(USRINC)/sys/types.h
$(DESTLIB)(proc_setrun.o): $(USRINC)/stdlib.h
$(DESTLIB)(proc_setrun.o): $(USRINC)/kernel/proc_lib.h
$(DESTLIB)(proc_setrun.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(proc_setrun.o): $(USRINC)/sys/proc.h
$(DESTLIB)(proc_smask.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(proc_smask.o): $(USRINC)/kernel/sig_lib.h
$(DESTLIB)(proc_smask.o): $(USRINC)/sys/debug.h
$(DESTLIB)(proc_smask.o): $(USRINC)/stddef.h
$(DESTLIB)(proc_smask.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(proc_smask.o): $(USRINC)/sys/proc.h
$(DESTLIB)(proc_smisc.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(proc_smisc.o): $(USRINC)/kernel/sig_lib.h
$(DESTLIB)(proc_smisc.o): $(USRINC)/sys/debug.h
$(DESTLIB)(proc_smisc.o): $(USRINC)/stddef.h
$(DESTLIB)(proc_smisc.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(proc_smisc.o): $(USRINC)/sys/proc.h
$(DESTLIB)(proc_ssig.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(proc_ssig.o): $(USRINC)/kernel/sig_lib.h
$(DESTLIB)(proc_ssig.o): $(USRINC)/kernel/proc_lib.h
$(DESTLIB)(proc_ssig.o): $(USRINC)/sys/debug.h
$(DESTLIB)(proc_ssig.o): $(USRINC)/sys/types.h
$(DESTLIB)(proc_ssig.o): $(USRINC)/string.h
$(DESTLIB)(proc_ssig.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(proc_ssig.o): $(USRINC)/sys/proc.h
$(DESTLIB)(proc_wake.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(proc_wake.o): $(USRINC)/kernel/sig_lib.h
$(DESTLIB)(proc_wake.o): $(USRINC)/sys/debug.h
$(DESTLIB)(proc_wake.o): $(USRINC)/stddef.h
$(DESTLIB)(proc_wake.o): $(USRINC)/kernel/proc_lib.h
$(DESTLIB)(proc_wake.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(proc_wake.o): $(USRINC)/sys/proc.h
$(DESTLIB)(reg_dump.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(reg_dump.o): $(USRINC)/common/_gregset.h
$(DESTLIB)(reg_dump.o): $(USRINC)/sys/cmn_err.h
$(DESTLIB)(reg_dump.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(reg_dump.o): $(USRINC)/sys/proc.h
$(DESTLIB)(sig_act.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(sig_act.o): $(USRINC)/kernel/sig_lib.h
$(DESTLIB)(sig_act.o): $(USRINC)/sys/debug.h
$(DESTLIB)(sig_act.o): $(USRINC)/stddef.h
$(DESTLIB)(sig_act.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(sig_act.o): $(USRINC)/sys/proc.h
$(DESTLIB)(sig_mask.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(sig_mask.o): $(USRINC)/kernel/sig_lib.h
$(DESTLIB)(sig_mask.o): $(USRINC)/sys/debug.h
$(DESTLIB)(sig_mask.o): $(USRINC)/stddef.h
$(DESTLIB)(sig_mask.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(sig_mask.o): $(USRINC)/sys/proc.h
$(DESTLIB)(sig_misc.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(sig_misc.o): $(USRINC)/kernel/sig_lib.h
$(DESTLIB)(sig_misc.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(sig_misc.o): $(USRINC)/sys/proc.h
$(DESTLIB)(sig_pend.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(sig_pend.o): $(USRINC)/kernel/sig_lib.h
$(DESTLIB)(sig_pend.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(sig_pend.o): $(USRINC)/sys/proc.h
$(DESTLIB)(st_alloc.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(st_alloc.o): $(USRINC)/common/xdebug.h
$(DESTLIB)(st_alloc.o): $(USRINC)/common/tricks.h
$(DESTLIB)(st_alloc.o): $(USRINC)/stddef.h
$(DESTLIB)(st_alloc.o): $(USRINC)/kernel/st_alloc.h
$(DESTLIB)(st_alloc.o): $(USRINC)/assert.h
$(DESTLIB)(st_alloc.o): $(USRINC)/sys/debug.h
$(DESTLIB)(st_alloc.o): $(USRINC)/string.h
$(DESTLIB)(st_alloc.o): $(USRINC)/sys/cmn_err.h
$(DESTLIB)(st_test.o): $(USRINC)/stdio.h
$(DESTLIB)(st_test.o): $(USRINC)/stdlib.h
$(DESTLIB)(st_test.o): $(USRINC)/kernel/st_alloc.h
$(DESTLIB)(st_test.o): $(USRINC)/time.h
$(DESTLIB)(uio.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(uio.o): $(USRINC)/sys/debug.h
$(DESTLIB)(uio.o): $(USRINC)/sys/types.h
$(DESTLIB)(uio.o): $(USRINC)/sys/errno.h
$(DESTLIB)(uio.o): $(USRINC)/limits.h
$(DESTLIB)(uio.o): $(USRINC)/string.h
$(DESTLIB)(uio.o): $(USRINC)/sys/uio.h

# DEPENDENCIES MUST END AT END OF FILE: IF YOU PUT STUFF HERE, IT WILL GO AWAY
# See make depend, above
