#include "util.h"
#include "ap.h"
#include "ll_log.h"

/*  Record beginnings of major sections of an address
 *
 *  Returns:    pointer to next node, after address
 *               0 if end of tree
 *              -1 if error
 *
 *  if both rroute and nroute are requested, and the tree has routing
 *  in reverse-path form, then these will point to the same node.
 */


extern LLog *logptr;

LOCVAR	int perlev, grplev;

AP_ptr
	ap_t2parts (tree, group, name, local, domain, route)
    register AP_ptr  tree;      /* the parse tree */
				/* THESE ARE POINTERS TO POINTERS */
				/* where to stuff the relevant pointers */
    AP_ptr  *group,             /* beginning of group name  */
	    *name,              /* beginning of person name  */
	    *local,             /* beginning of local-part */
	    *domain,            /* basic domain reference */
	    *route;             /* beginning of 822 reverse routing */
{
    AP_ptr retptr;
    int gotlocal;

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "ap_t2parts ()");
#endif

    if (tree -> ap_obtype == APV_NIL)
	return ((AP_ptr) OK);      /* Ignore null stuff */

    grplev = perlev = 0;

    if (group   != (AP_ptr *) 0)
	*group   = (AP_ptr)   0;
    if (name    != (AP_ptr *) 0)
	*name    = (AP_ptr)   0;
    if (local   != (AP_ptr *) 0)
	*local   = (AP_ptr)   0;
    if (domain  != (AP_ptr *) 0)
	*domain  = (AP_ptr)   0;
    if (route   != (AP_ptr *) 0)
	*route   = (AP_ptr)   0;

    for (gotlocal = FALSE; ; tree = tree -> ap_chain) {
					/* print munged addr                  */
	switch (tree -> ap_obtype) {
	    case APV_NIL: 
		retptr = (AP_ptr) 0;
		goto endit;

	    case APV_CMNT:        /* Output value as comment */
		break;

	    case APV_NPER: 
		perlev++;
		if (name != (AP_ptr *) 0 && *name == (AP_ptr) 0)
		    *name = tree;
		break;

	    case APV_PRSN:
		break;

	    case APV_EPER: 
		perlev--;
		break;

	    case APV_NGRP: 
		grplev++;
		if (group != (AP_ptr *) 0 && *group == (AP_ptr) 0)
		    *group = tree;
		break;

	    case APV_GRUP:
		break;

	    case APV_EGRP: 
		grplev--;
		break;

	    case APV_WORD: 
	    case APV_MBOX: 
		if (local != (AP_ptr *) 0 && *local == (AP_ptr) 0)
		    *local = tree;
		gotlocal = TRUE;

		break;

	    case APV_DLIT:
	    case APV_DOMN:
		if (gotlocal) {         /* reference after local        */
		    if (domain != (AP_ptr *) 0 && *domain == (AP_ptr) 0)
			*domain = tree;
		} else {                /* must be 822 route    */
					/* domain precedes local-part   */
		    if (route != (AP_ptr *) 0 && *route == (AP_ptr) 0)
			*route = tree;
		}
		break;

	    default: 
		break;
	}

	switch (tree -> ap_ptrtype) {
	    case APP_NXT:
		if (tree -> ap_chain -> ap_obtype != APV_NIL) {
		    retptr = (AP_ptr) tree -> ap_chain;
		    goto endit;
		}
				 /* else DROP ON THROUGH               */
	    case APP_NIL:
		retptr = (AP_ptr) OK;
		goto endit;		/* Courtesy of Steve Kille */
	}
    }

endit:
    return (retptr);
}
