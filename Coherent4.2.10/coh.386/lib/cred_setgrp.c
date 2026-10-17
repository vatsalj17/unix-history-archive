/* $Header: $ */
/*
 * Part of the credentials system. This file deals with setting a new
 * supplementary group ID list for a set of credentials.
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
 * Change the credentials set so as to supply sufficient room in the
 * "cr_credp" member for the user to copy in a new supplementary group ID list
 * with room for the indicated number of members. If there is insufficient
 * memory to make the requested change, NULL is returned and the original
 * structure is unchanged. Upon success, a pointer to a credentials set with
 * the appropriate "cr_ngroups" setting is returned.
 */

#if	__USE_PROTO__
cred_t * (cred_setgrp) (cred_t * credp, unsigned short ngroups)
#else
cred_t *
cred_setgrp __ARGS ((credp, ngroups))
cred_t	      *	credp;
unsigned short	ngroups;
#endif
{
	n_gid_t	      *	newgroups;

	ASSERT (credp != NULL);
	ASSERT (credp->cr_ref > 0);

	if (ngroups > 0) {
		if ((newgroups = groups_alloc (ngroups)) == NULL)
			return NULL;
	} else
		newgroups = NULL;

	if (credp->cr_ref > 1) {
		/*
		 * Do not disturb the original credentials; allocate a
		 * fresh credentials structure.
		 */

		if ((credp = cred_copy (credp)) == NULL) {
			if (newgroups != NULL)
				groups_unref (newgroups, ngroups);
			return NULL;
		}
	}

	/*
	 * Since we are overlaying the old supplementary group ID
	 * list, release our reference to it.
	 */

	if (credp->cr_ngroups)
		groups_unref (credp->cr_groups, credp->cr_ngroups);


	/*
	 * Since we assume the caller holds the only active reference to the
	 * credentials set, (trivially true if it was created above) we know
	 * that no-one else may capture a reference to this set (or if they
	 * have, then they should see the new data...).
	 */

	credp->cr_ngroups = ngroups;
	credp->cr_groups = newgroups;

	return credp;
}

