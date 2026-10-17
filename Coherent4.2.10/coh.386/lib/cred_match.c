/* $Header: $ */
/*
 * Part of the credentials system. This file deals with working out whether a
 * credentials set matches a given user- and group-id pair.
 *
 * $Log: $
 */

#include <common/ccompat.h>
#include <common/_imode.h>
#include <kernel/cred_lib.h>
#include <sys/debug.h>
#include <sys/types.h>
#include <sys/cred.h>
#include <stddef.h>


/*
 * Return an enumeration constant for the level of match between the given
 * credentials and the supplied user- and group-id pair.
 */

#if	__USE_PROTO__
int (cred_match) (cred_t * credp, n_uid_t uid, n_gid_t gid)
#else
int
cred_match __ARGS ((credp, uid, gid))
cred_t	      *	credp;
n_uid_t		uid;
n_gid_t		gid;
#endif
{
	int		i;

	ASSERT (credp != NULL);

	/*
	 * Note that we do not check "credp->cr_ref" on entry here, because
	 * cred_fake () creates such entities.
	 */

	if (credp->cr_uid == uid)
	    	return _CRED_OWNER;

	if (credp->cr_gid == gid)
	    	return _CRED_GROUP;

	for (i = 0 ; i < credp->cr_ngroups ; i ++)
		if (credp->cr_groups [i] == gid)
			return _CRED_GROUP;

	/*
	 * We do not try matching against superuser permissions here; that is
	 * not what we are here for. Our clients will usually want to check
	 * for this, but using a strategy of their choosing.
	 */

	return _CRED_OTHER;
}
