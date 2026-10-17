/* $Header: $ */
/*
 * Implementation of ipc_cred_match (), part of the common framework for
 * credentials matching in System V IPC code.
 *
 * $Log: $
 */

#include <common/ccompat.h>
#include <common/_imode.h>
#include <sys/debug.h>
#include <sys/cred.h>
#include <stddef.h>

#include <sys/ipc.h>

#define	_KERNEL		1

#include <sys/proc.h>

#if	__COHERENT__

int		super		__PROTO ((void));

#endif


/*
 * Work out what level of access the current user has to the indicated IPC
 * permissions structure. Note that System V IPC checking only ever uses the
 * effective user ID, not the real user ID.
 */

#if	__USE_PROTO__
int ipc_cred_match (struct ipc_perm * ipcp)
#else
int
ipc_cred_match (ipcp)
struct ipc_perm	* ipcp;
#endif
{
        int old_err;
	ASSERT (ipcp != NULL);

	/*
	 * super() has a side-effect of setting u.u_error to EPERM,
	 * so if we end up with _CRED_OTHER, we need to restore
	 * the old error.  The comma operator is convenient here.
	 */

	return (SELF->p_credp->cr_uid == ipcp->uid ||
		SELF->p_credp->cr_uid == ipcp->cuid) ? _CRED_OWNER :
	       (SELF->p_credp->cr_gid == ipcp->gid ||
		SELF->p_credp->cr_gid == ipcp->cgid) ? _CRED_GROUP :
	       (old_err = get_user_error(), super ()) ? _CRED_OWNER :
	       (set_user_error(old_err), _CRED_OTHER);
}

