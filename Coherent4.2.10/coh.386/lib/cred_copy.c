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
 * Return a fresh duplicate of a set of credentials, or NULL if there is no
 * memory available.
 */

#if	__USE_PROTO__
cred_t * (cred_copy) (cred_t * credp)
#else
cred_t *
cred_copy __ARGS ((credp))
cred_t	      *	credp;
#endif
{
	cred_t	      *	newcredp;

	ASSERT (credp != NULL);
	ASSERT (credp->cr_ref > 0);
	
	if ((newcredp = cred_alloc ()) == NULL)
		return NULL;

	* newcredp = * credp;
	newcredp->cr_ref = 1;

	if (newcredp->cr_ngroups > 0)
		groups_ref (newcredp->cr_groups, newcredp->cr_ngroups);
	cred_unref (credp);

	return newcredp;
}
