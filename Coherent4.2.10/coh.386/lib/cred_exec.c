/* $Header: $ */
/*
 * Part of the credentials system. This file deals with setting a new user ID
 * for a set of credentials.
 *
 * $Log: $
 */

#include <common/ccompat.h>
#include <kernel/cred_lib.h>
#include <sys/debug.h>
#include <sys/types.h>
#include <sys/cred.h>
#include <stddef.h>


/*
 * Set a new user-id code and group-id code for a credentials structure; this
 * updates the effective and saved user-id and group-id, but leaves the real
 * user-id and group-id unchanged. If there is not sufficient available memory
 * to satisfy the request, NULL is returned and the original credentials are
 * unchanged.
 */

#if	__USE_PROTO__
cred_t * (cred_execid) (cred_t * credp, n_uid_t uid, n_gid_t gid)
#else
cred_t *
cred_execid __ARGS ((credp, uid, gid))
cred_t	      *	credp;
n_uid_t		uid;
n_gid_t		gid;
#endif
{
	ASSERT (credp != NULL);
	ASSERT (credp->cr_ref > 0);

	if (credp->cr_ref > 1) {
		/*
		 * Do not disturb the original credentials; duplicate the
		 * reference to the supplementary group ID list and allocate a
		 * fresh credentials structure.
		 */

		if ((credp = cred_copy (credp)) == NULL)
			return NULL;
	}


	/*
	 * Since we assume the caller holds the only active reference to the
	 * credentials set, (trivially true if it was created above) we know
	 * that no-one else may capture a reference to this set (or if they
	 * have, then they should see the new data...).
	 */

	credp->cr_suid = credp->cr_uid = uid;
	credp->cr_sgid = credp->cr_gid = gid;
	return credp;
}

