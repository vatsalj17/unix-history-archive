#include "util.h"
#include "ap.h"
#include "ll_log.h"

/* parse a string into an address tree */

extern LLog *logptr;

LOCVAR char *ap_strptr;

LOCVAR
	getach ()       /* get next character from string */
{
    if (*ap_strptr == '\0')  /* end of the string, of course */
	return (0);
    return (*ap_strptr++);
}

AP_ptr
	ap_s2tree (thestr)
    char thestr[];
{
    short gotone;
    AP_ptr thetree;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ap_s2tree (%s)", thestr);
#endif

    ap_strptr = thestr;

    if ((thetree = ap_pinit (getach)) == (AP_ptr) NOTOK)
	goto badend;
    ap_clear();

    gotone = FALSE;
    for ( ; ; )
	switch (ap_1adr ()) {
	    case NOTOK:
		ap_sqdelete (thetree, (AP_ptr) 0);
		ap_free (thetree);
	badend:
		ap_strptr = (char *) 0;
		return ((AP_ptr) NOTOK);

	    case OK:
	    	gotone = TRUE;
		continue;       /* more to process */

	    case DONE:
		ap_strptr = (char *) 0;
		if (gotone == FALSE)
			return ((AP_ptr) NOTOK);
		return (thetree);
	}
    /* NOTREACHED */
}
