/*
 *              G E T M A I L I D . C
 *
 *      This module is used to get the "mailid" for a user given
 *      his username.  Use of the mail-id tables is controlled
 *      by the variable mid_enable.
 */
#include "util.h"
#include "mmdf.h"
#include "ch.h"
#include <pwd.h>

extern LLog *logptr;
extern int mid_enable;

char *
getmailid (username)
char *username;
{
	static char buf[MAILIDSIZ+1];
	extern Table tb_users;

	if (mid_enable) {
		if (tb_k2val (&tb_users, TRUE, username, buf) != OK)
			return (NULL);
		buf[MAILIDSIZ] = '\0';          /* Paranoid */
	} else
		(void) strcpy (buf, username);

#ifdef DEBUG
	ll_log (logptr, LLOGFTR, "getmailid returns '%s'", buf);
#endif
	return (buf);
}
