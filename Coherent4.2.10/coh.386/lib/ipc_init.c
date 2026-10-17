/* $Header: $ */
/*
 * Implementation of ipc_perm_init (), part of the common framework for
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


/*
 * No cred_t's yet, do it the nasty way.
 */

#define	_KERNEL		1

#include <sys/proc.h>


/*
 * Work out what level of access the current user has to the indicated IPC
 * permissions structure. Note that System V IPC checking only ever uses the
 * effective user ID, not the real user ID.
 */

#if	__USE_PROTO__
void ipc_perm_init (struct ipc_perm * ipcp, __mode_t mode)
#else
void
ipc_perm_init (ipcp, mode)
struct ipc_perm	* ipcp;
__mode_t	mode;
#endif
{
	ASSERT (ipcp != NULL);

	ipcp->cuid = ipcp->uid = SELF->p_credp->cr_uid;
	ipcp->cgid = ipcp->gid = SELF->p_credp->cr_gid;
	ipcp->mode = (mode & __IRWXUGO) | __IPC_ALLOC;
}
