#include "util.h"               /* from ../utildir */
#include "conf.h"               /* from ../mmdf/h */
#include "ch.h"
#include "ap.h"
#include "dm.h"
#include "ll_log.h"

/*  Format one address from parse tree, into a string
 *
 *  Returns:    pointer to next node, after address
 *               0 if end of tree
 *              NOTOK if error
 */

extern LLog *logptr;
extern char *strdup();
extern char *ap_p2s();
extern AP_ptr ap_t2parts();

AP_ptr
	ap_t2s (thetree, strpp)
    AP_ptr thetree;             /* the parse tree */
    char **strpp;               /* where to stuff the string */
{
    AP_ptr locptr,              /* in case we need to fake personal name */
	   grpptr,
	   namptr,
	   domptr,
	   routptr,
	   rtntree;

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "ap_t2s ()");
#endif

    rtntree = ap_t2parts (thetree, &grpptr, &namptr,
			  &locptr, &domptr, &routptr);
    if (rtntree == (AP_ptr)NOTOK) {
	ll_log (logptr, LLOGTMP, "ap_t2s: error from ap_t2parts()");
	*strpp = strdup ("(MMDF Error!)");
	return ((AP_ptr)NOTOK);
    }

    *strpp = ap_p2s (grpptr, namptr, locptr, domptr, routptr);

    if(*strpp == (char *)MAYBE){
	*strpp = strdup("");        /* is this needed ?? */
	return ((AP_ptr)MAYBE);
    }
    else if (*strpp == (char *)NOTOK) {
	ll_log (logptr, LLOGTMP, "ap_t2s: error from ap_p2s()");
	*strpp = strdup ("(MMDF Error!)");
	return ((AP_ptr)NOTOK);
    }
    return (rtntree);
}
