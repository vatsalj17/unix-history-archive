/*
 *  V 7 . L O C A L . C 
 *
 *  EE/CIS Computer Lab
 *  Department of Computer and Information Sciences
 *  Department of Electrical Engineering
 *  University of Delaware
 *
 *  REVISION HISTORY:
 *
 *  $Revision: 1.4 $
 *
 *  $Log:	v7.local.c,v $
 * Revision 1.4  85/12/18  01:54:04  galvin
 * Create maildrop using MMDF default protection modes.
 * 
 * Revision 1.4  85/12/18  01:44:59  galvin
 * Create the mailbox using the MMDF default protection mode
 * as defined by sentprotect.
 * 
 * Revision 1.3  85/11/20  14:38:17  galvin
 * Change findmail to locate the maildrop via MMDF defaults.
 * 
 * Revision 1.2  85/11/17  21:33:07  galvin
 * Added comment header for revision history.
 * 
 *
 */

/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char *sccsid = "@(#)v7.local.c	5.2 (Berkeley) 6/21/85";
#endif not lint

/*
 * Mail -- a mail program
 *
 * Version 7
 *
 * Local routines that are installation dependent.
 */

#include "./rcv.h"
#include "./mmdf.h"

/*
 * Locate the user's mailbox file (ie, the place where new, unread
 * mail is queued).  In Version 7, it is in /usr/spool/mail/name.
 */

findmail()
{
	sprintf( mailname, "%s/%s",
		(mldfldir==0 || isnull(mldfldir[0])) ? homedir : mldfldir,
		(mldflfil==0 || isnull(mldflfil[0])) ? myname : mldflfil);
}

/*
 * Get rid of the queued mail.
 */

demail()
{

	if (value("keep") != NOSTR)
		close(creat(mailname, sentprotect));
	else {
		if (remove(mailname) < 0)
			close(creat(mailname, sentprotect));
	}
}

/*
 * Discover user login name.
 */

username(l_uid, namebuf)
	char namebuf[];
{
	register char *np;

	if (l_uid == getuid() && (np = getenv("USER")) != NOSTR) {
		strncpy(namebuf, np, PATHSIZE);
		return(0);
	}
	return(getname(l_uid, namebuf));
}
