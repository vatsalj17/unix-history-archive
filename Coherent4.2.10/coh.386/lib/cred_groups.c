/* $Header: $ */
/*
 * Part of the credentials system. This file implements a subsystem for
 * managing the list of supplementary group ID's.
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
 * Allocate an area sufficient to hold "ngroups" supplementary group
 * identifiers. The reference count of the allocated storage is set to 1.
 * NULL is returned if sufficient memory cannot be allocated.
 *
 * Note that a request to allocate 0 supplementary group ID's is an error.
 */

#if	__USE_PROTO__
n_gid_t * (groups_alloc) (unsigned short ngroups)
#else
n_gid_t *
groups_alloc __ARGS ((ngroups))
unsigned short	ngroups;
#endif
{
	n_gid_t	      *	groupsp;

	ASSERT (ngroups > 0);

	/*
	 * Allocate room for an additional member to store a reference count
	 * in.
	 */

	if ((groupsp = (n_gid_t *)
			kmem_zalloc ((ngroups + 1) * sizeof (n_gid_t),
				     KM_SLEEP)) == NULL)
		return NULL;

	* groupsp ++ = 1;
	return groupsp;
}


/*
 * Increase the reference count of a supplementary group ID list.
 */

#if	__USE_PROTO__
void groups_ref (n_gid_t * groups, unsigned short __NOTUSED (ngroups))
#else
void
groups_ref __ARGS ((groups, ngroups))
n_gid_t	      *	groups;
unsigned short	ngroups;
#endif
{
	ASSERT (groups != NULL);
	ASSERT (groups [- 1] > 0);

	groups [- 1] ++;
}


/*
 * Decrement the reference count of a supplementary group ID list; if the
 * reference count reaches zero, deallocate the memory.
 */

#if	__USE_PROTO__
void (groups_unref) (n_gid_t * groups, unsigned short ngroups)
#else
void
groups_unref __ARGS ((groups, ngroups))
n_gid_t	      *	groups;
unsigned short	ngroups;
#endif
{
	ASSERT (groups != NULL);
	ASSERT (ngroups > 0);
	ASSERT (groups [- 1] > 0);

	if (-- groups [- 1] == 0)
		kmem_free (groups - 1, (ngroups + 1) * sizeof (n_gid_t));
}
