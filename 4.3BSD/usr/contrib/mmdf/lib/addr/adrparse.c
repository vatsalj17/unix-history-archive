/*
**
**	R E V I S I O N  H I S T O R Y
**
**	03/08/85   LDK	Added compress() in this module to remove
**	           VAK  leading and trailing space.
**
*/

#include "util.h"
#include "mmdf.h"
#include "ap.h"

extern	char *ap_s2p();
extern  char *compress();

parsadr (thestr, name, mbox, host)
register char  *thestr;		  /* string with an address             */
char   *name,			  /* where to put name part             */
       *mbox,			  /* where to put mailbox part          */
       *host;			  /* where to put hostname part         */
{
	register char *cp;
	AP_ptr treep, namep, local, domain;

	if (name != (char *) 0)
		*name = '\0';
	if (mbox != (char *) 0)
		*mbox = '\0';
	if (host != (char *) 0)
		*host = '\0';

	cp = ap_s2p (thestr, &treep, (AP_ptr *)0, &namep,
			&local, &domain, (AP_ptr *)0);
	if (cp == (char *) DONE || cp == (char *) NOTOK)
		return;
	else if (local -> ap_obvalue == NULL)
		return;		/* Bad format */
	if (name && namep != 0 && namep -> ap_obvalue != 0)
		(void) strcpy (name, namep->ap_obvalue);
	if (mbox && local != 0 && local -> ap_obvalue != 0)
		(void) strcpy (mbox, local->ap_obvalue);
	if (host && domain != 0 && domain -> ap_obvalue != 0)
		(void) strcpy (host, domain->ap_obvalue);
	ap_free (treep);
	return;
}
