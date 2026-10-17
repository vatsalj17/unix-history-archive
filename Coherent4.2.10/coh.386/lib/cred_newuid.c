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
 * Set a new user-id code for a credentials structure; this updates the real,
 * effective and saved user-ids. If there is not sufficient available memory
 * to satisfy the request, NULL is returned and the original credentials are
 * unchanged. Upon success, a pointer to a credentials set with the indicated
 * "uid" is returned.
 */

#if	__USE_PROTO__
cred_t * (cred_newuid) (cred_t * credp, n_uid_t uid)
#else
cred_t *
cred_newuid __ARGS ((credp, uid))
cred_t	      *	credp;
n_uid_t		uid;
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

	credp->cr_suid = credp->cr_ruid = credp->cr_uid = uid;
	return credp;
}


/*
 * Set a new user-id code for a credentials structure; this updates only the
 * effective user-id. If there is not sufficient available memory to satisfy
 * the request, NULL is returned and the original credentials are unchanged.
 * Upon success, a pointer to a credentials set with the indicated "uid" is
 * returned.
 */

#if	__USE_PROTO__
cred_t * (cred_neweuid) (cred_t * credp, n_uid_t uid)
#else
cred_t *
cred_neweuid __ARGS ((credp, uid))
cred_t	      *	credp;
n_uid_t		uid;
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

	credp->cr_uid = uid;
	return credp;
}


