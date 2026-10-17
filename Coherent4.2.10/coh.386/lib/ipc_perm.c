/* $Header: $ */
/*
 * Implementation of ipc_lack_perm (), part of the common framework for
 * credentials matching in System V IPC code.
 *
 * $Log: $
 */

#include <common/ccompat.h>
#include <common/_imode.h>
#include <sys/debug.h>
#include <sys/cmn_err.h>
#include <sys/errno.h>
#include <sys/ipc.h>
#include <stddef.h>

#define	_KERNEL		1

#include <sys/uproc.h>

#if	__COHERENT__

int		super		__PROTO ((void));

#endif


/*
 * Work out what level of access the current user has to the indicated IPC
 * permissions structure. Note that System V IPC checking only ever uses the
 * effective user ID, not the real user ID.
 */

#if	__USE_PROTO__
int ipc_lack_perm (struct ipc_perm * ipcp, __mode_t mode)
#else
int
ipc_lack_perm (ipcp, mode)
struct ipc_perm	* ipcp;
__mode_t	mode;
#endif
{
	int		cred_lvl;

	ASSERT (ipcp != NULL);

	/*
	 * I do not really know why this is done, but Vlad's code took pains
	 * to do so; this needs verifying against some SVR4 system, since this
	 * is not documented behaviour.
	 */

	if ((mode & __IRWXUGO) == 0)
		return 0;

	/*
	 * Work out what level of access the current process has. The mode
	 * bits requested must match exactly.
	 */

	cred_lvl = ipc_cred_match (ipcp);

	if ((mode = __PERM_EXTRACT (mode, cred_lvl) & __PERM_MASK) != 0 &&
	    (__PERM_EXTRACT (ipcp->mode, cred_lvl) & mode) == mode)
		return 0;

	/*
	 * The super-user can always do things; note that by making this check
	 * last (where it should be), we cause the accounting record to show
	 * use of super-user permissions correctly.
	 */

	if (super ())
		return 0;

	cmn_err (CE_NOTE, "IPC access failed, lvl = %d req = %x target = %x",
		 cred_lvl, mode, ipcp->mode);

	set_user_error (EACCES);
	return 1;
}
