#ifndef lint
static	char sccsid[] = "@(#)ypxdr.c 1.1 85/05/30 Copyr 1984 Sun Micro";
#endif

/*
 * ypxdr.c
 * This supplies xdr routines used by the yellowpages rpc interface.
 */

#include <dbm.h>
#undef NULL				/* Remove dbm.h's definition of NULL */
extern void dbmclose();			/* Refer to dbm routine not in dbm.h */
#define NULL (char *) 0
#include <rpc/rpc.h>
#ifdef DEVELOPMENT
#include "yp_prot.h"
#else
#include <rpcsvc/yp_prot.h>
#endif

typedef struct xdr_discrim XDR_DISCRIM;
bool xdr_datum();
bool xdr_ypdomain_wrap_string();
bool xdr_ypmap_wrap_string();
bool xdr_ypreq_key();
bool xdr_ypreq_nokey();
bool xdr_yprequest();
bool xdr_ypresp_val();
bool xdr_ypresp_key_val();
bool xdr_ypresponse();
bool xdr_ypbind_resp ();
bool xdr_yp_inaddr();
bool xdr_yp_binding();
bool xdr_ypmap_parms();
bool xdr_ypowner_wrap_string();

/*
 * xdr_datum
 * Serializes/deserializes a dbm datum data structure.
 */
bool
xdr_datum (xdrs, pdatum)
	XDR * xdrs;
	datum * pdatum;

{
	return (xdr_bytes(xdrs, &(pdatum->dptr), &(pdatum->dsize), YPMAXRECORD));
}


/*
 * Serializes/deserializes a domain name string.  This
 * is only a "wrapper" for xdr_string which knows about the maximum
 * domain name size.
 */
bool
xdr_ypdomain_wrap_string(xdrs, ppstring)
	XDR * xdrs;
	char **ppstring;
{
	return(xdr_string(xdrs, ppstring, YPMAXDOMAIN) );
}

/*
 * Serializes/deserializes a map name string.  This
 * is only a "wrapper" for xdr_string which knows about the maximum
 * map name size.
 */
bool
xdr_ypmap_wrap_string(xdrs, ppstring)
	XDR * xdrs;
	char **ppstring;
{
	return(xdr_string(xdrs, ppstring, YPMAXMAP) );
}

/*
 * Serializes/deserializes a ypreq_key structure.
 */
bool
xdr_ypreq_key (xdrs, ps)
	XDR *xdrs;
	struct ypreq_key *ps;

{
	if (xdr_ypdomain_wrap_string(xdrs, &ps->domain) &&
	    xdr_ypmap_wrap_string(xdrs, &ps->map) &&
	    xdr_datum(xdrs, &ps->keydat) ) {
		return(TRUE);
	} else {
		return(FALSE);
	}
}

/*
 * Serializes/deserializes a ypreq_nokey structure.
 */
bool
xdr_ypreq_nokey (xdrs, ps)
	XDR * xdrs;
	struct ypreq_nokey *ps;
{
	if (xdr_ypdomain_wrap_string(xdrs, &ps->domain) &&
	    xdr_ypmap_wrap_string(xdrs, &ps->map) ) {
		return(TRUE);
	} else {
		return(FALSE);
	}
}


/*
 * xdr discriminant/xdr_routine vector for yp requests.
 */
XDR_DISCRIM yprequest_arms[] = {
	{(int) YPREQ_KEY, (xdrproc_t) xdr_ypreq_key},
	{(int) YPREQ_NOKEY, (xdrproc_t) xdr_ypreq_nokey},
	{(int) YPREQ_MAP_PARMS, (xdrproc_t) xdr_ypmap_parms},
	{__dontcare__, (xdrproc_t) NULL}
};

/*
 * Serializes/deserializes a yprequest structure.
 */
bool
xdr_yprequest (xdrs, ps)
	XDR * xdrs;
	struct yprequest *ps;

{
	return(xdr_union(xdrs, &ps->yp_reqtype, &ps->yp_reqbody,
	    yprequest_arms, NULL) );
}


/*
 * Serializes/deserializes a ypreq_key structure.
 */

bool
xdr_ypresp_val (xdrs, ps)
	XDR * xdrs;
	struct ypresp_val *ps;
{
	if (xdr_u_long(xdrs, &ps->status) && (xdr_datum(xdrs, &ps->valdat))) {
		return(TRUE);
	} else {
		return(FALSE);
	}
}

/*
 * Serializes/deserializes a ypresp_key_val structure.
 */
bool
xdr_ypresp_key_val (xdrs, ps)
	XDR * xdrs;
	struct ypresp_key_val *ps;
{
	if ( (xdr_u_long(xdrs, &ps->status) ) &&
	    (xdr_datum(xdrs, &ps->valdat) ) &&
	    (xdr_datum(xdrs, &ps->keydat)) ) {
		return(TRUE);
	} else {
		return(FALSE);
	}
}

/*
 * xdr discriminant/xdr_routine vector for yp responses
 */
XDR_DISCRIM ypresponse_arms[] = {
	{(int) YPRESP_VAL, (xdrproc_t) xdr_ypresp_val},
	{(int) YPRESP_KEY_VAL, (xdrproc_t) xdr_ypresp_key_val},
	{(int) YPRESP_MAP_PARMS, (xdrproc_t) xdr_ypmap_parms},
	{__dontcare__, (xdrproc_t) NULL}
};

/*
 * Serializes/deserializes a ypresponse structure.
 */
bool
xdr_ypresponse (xdrs, ps)
	XDR * xdrs;
	struct ypresponse *ps;

{
	return(xdr_union(xdrs, &ps->yp_resptype, &ps->yp_respbody,
	    ypresponse_arms, NULL) );
}

/*
 * Serializes/deserializes an in_addr struct.
 * Note:  There is a data coupling between the "definition" of a
 * struct in_addr implicit in this xdr
 * routine, and the true data definition in <netinet/in.h>.
 */
bool
xdr_yp_inaddr(xdrs, ps)
	XDR * xdrs;
	struct in_addr *ps;

{
	return(xdr_u_long(xdrs, &ps->s_addr));
}

/*
 * Serializes/deserializes a ypbind_binding struct.
 */
bool
xdr_yp_binding(xdrs, ps)
	XDR * xdrs;
	struct ypbind_binding *ps;

{
	return(xdr_yp_inaddr(xdrs, &ps->ypbind_binding_addr) &&
            xdr_u_short(xdrs, &ps->ypbind_binding_port));
}

/*
 * xdr discriminant/xdr_routine vector for yp binder responses
 */
XDR_DISCRIM ypbind_resp_arms[] = {
	{(int) YPBIND_SUCC_VAL, (xdrproc_t) xdr_yp_binding},
	{(int) YPBIND_FAIL_VAL, (xdrproc_t) xdr_u_long},
	{__dontcare__, (xdrproc_t) NULL}
};

/*
 * Serializes/deserializes a ypbind_resp structure.
 */
bool
xdr_ypbind_resp (xdrs, ps)
	XDR * xdrs;
	struct ypbind_resp *ps;

{
	return(xdr_union(xdrs, &ps->ypbind_status, &ps->ypbind_respbody,
	    ypbind_resp_arms, NULL) );
}

/*
 * Serializes/deserializes a peer server's node name
 *	(which is basically an arbitrary string).
 */
bool
xdr_ypowner_wrap_string(xdrs, ppstring)
	XDR * xdrs;
	char **ppstring;

{
	return(xdr_string(xdrs, ppstring, YPMAXPEER) );
}

/*
 * Serializes/deserializes a ypmap_parms structure.
 */
bool
xdr_ypmap_parms(xdrs, ps)
	XDR *xdrs;
	struct ypmap_parms *ps;

{
	if (xdr_ypdomain_wrap_string(xdrs, &ps->domain) &&
	    xdr_ypmap_wrap_string(xdrs, &ps->map) &&
	    xdr_u_long(xdrs, &ps->ordernum) &&
	    xdr_ypowner_wrap_string(xdrs, &ps->owner) ) {
		return(TRUE);
	} else {
		return(FALSE);
	}
}

/*
 * Serializes/deserializes a ypbind_setdom structure.
 */
bool
xdr_ypbind_setdom(xdrs, ps)
	XDR *xdrs;
	struct ypbind_setdom *ps;
{
	char *domain = ps->ypsetdom_domain;
	
	return(xdr_ypdomain_wrap_string(xdrs, &domain) &&
	    xdr_yp_binding(xdrs, &ps->ypsetdom_binding) );
}
