/*
 *              G E T P W M I D . C
 *
 *      Given a mailid, get the /etc/passwd info for
 *      the user who is associated with that mailid.
 *      If mid_enable is nonzero, then the mailid cannot
 *      be interpreted as a login id, we must lookup
 *      the mailid in a mailid->loginid table.  Once
 *      we have the login id, we just do a getpwnam();
 *      For regular systems, then we assume that the
 *      mailid must be a login id and do a getpwnam();
 */
#include "util.h"
#include "mmdf.h"
#include "ch.h"
#include <pwd.h>

struct passwd *
getpwmid (mailid)
char *mailid;
{
	extern struct passwd *gpwlnam();
	extern Table tb_mailids;
	extern int mid_enable;
	char    buf[ADDRSIZE];

	if (mid_enable) {
		if (tb_k2val (&tb_mailids, TRUE, mailid, buf) != OK)
			return (NULL);
		return (gpwlnam(buf));
	} else
		return (gpwlnam(mailid));
}
