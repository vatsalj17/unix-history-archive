/* $Header: $ */
/*
 * Part of the credentials system. This file deals with allocating memory
 * for credentials.
 *
 * $Log: $
 */

#include <common/ccompat.h>
#include <kernel/cred_lib.h>
#include <sys/debug.h>
#include <sys/types.h>
#include <sys/cred.h>
#include <sys/kmem.h>


/*
 * Allocate a basic, empty credentials structure; assume that using the zero-
 * fill property of kmem_zalloc () is good enough to initialize all the
 * structure members except for "cr_groups". The initial reference count is
 * set to 1; NULL is returned if sufficient memory cannot be allocated.
 */

#if	__USE_PROTO__
cred_t * (cred_alloc) (void)
#else
cred_t *
cred_alloc __ARGS (())
#endif
{
	cred_t	      *	credp;

	if ((credp = (cred_t *) kmem_zalloc (sizeof (* credp),
					     KM_SLEEP)) == NULL)
		return NULL;

	credp->cr_ref = 1;
	credp->cr_groups = NULL;
	return credp;
}


/*
 * Increase the reference count of a credentials structure.
 */

#if	__USE_PROTO__
void (cred_ref) (cred_t * credp)
#else
void
cred_ref __ARGS ((credp))
cred_t	      *	credp;
#endif
{
	ASSERT (credp != NULL);
	ASSERT (credp->cr_ref > 0);

	credp->cr_ref ++;
}


/*
 * Reduce the reference count on a credentials structure, which may result in
 * freeing the memory.
 */

#if	__USE_PROTO__
void (cred_unref) (cred_t * credp)
#else
void
cred_unref __ARGS ((credp))
cred_t	      *	credp;
#endif
{
	ASSERT (credp != NULL);
	ASSERT (credp->cr_ref > 0);

	if (-- credp->cr_ref != 0)
		return;

	if (credp->cr_ngroups > 0)
		groups_unref (credp->cr_groups, credp->cr_ngroups);

	kmem_free (credp, sizeof (* credp));
}
