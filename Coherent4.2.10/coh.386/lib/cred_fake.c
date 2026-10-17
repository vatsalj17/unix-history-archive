/* $Header: $ */
/*
 * Part of the credentials system. This file deals with creating the ersatz
 * credentials structures used in situations such as checking permissions in
 * the access () system call.
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
 * Intialize a credentials structure (presumably allocated on the caller's
 * stack) with information from the other credentials structure such that the
 * new credentials have the effective IDs taken from the real IDs in the
 * source credentials. The destination credentials structure is returned.
 *
 * The target credentials structure is assumed to have a limited lifetime and
 * can be destroyed by the caller by deallocating the memory (preferably by
 * the expedient of going out of scope). Passing such a credentials structure
 * to any routine other than cred_match () is an error.
 */

#if	__USE_PROTO__
cred_t * (cred_fake) (cred_t * dest, __CONST__ cred_t * src)
#else
cred_t *
cred_fake __ARGS ((dest, src))
cred_t	      *	dest;
__CONST__ cred_t
	      *	src;
#endif
{
	dest->cr_ref = 0;
	dest->cr_uid = src->cr_ruid;
	dest->cr_gid = src->cr_rgid;
	dest->cr_ngroups = src->cr_ngroups;
	dest->cr_groups = src->cr_groups;

	return dest;
}
