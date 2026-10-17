#ifndef lint
static  char sccsid[] = "@(#)rusersxdr.c 1.1 85/05/30 Copyr 1984 Sun Micro";
#endif

/*
 * Copyright (c) 1984 by Sun Microsystems, Inc.
 */

#include <rpc/rpc.h>
#include "rusers.h"
#include <utmp.h>

rusers(host, up)
	char *host;
	struct utmparr *up;
{
	return callrpc(host, RUSERSPROG, RUSERSVERS, RUSERSPROC_NAMES,
	    xdr_void, 0, xdr_utmparr, up);
}

rnusers(host, up)
	char *host;
	struct utmparr *up;
{
	int nusers;
	
	if (callrpc(host, RUSERSPROG, RUSERSVERS, RUSERSPROC_NUM,
	    xdr_void, 0, xdr_u_long, &nusers) < 0)
		return -1;
	else
		return (nusers);
}

xdr_utmp(xdrsp, up)
	XDR *xdrsp;
	struct utmp *up;
{
	int len;
	char *p;

	len = sizeof(up->ut_line);
	p = up->ut_line;
	if (xdr_bytes(xdrsp, &p, &len, len) == FALSE)
		return 0;
	len = sizeof(up->ut_name);
	p = up->ut_name;
	if (xdr_bytes(xdrsp, &p, &len, len) == FALSE)
		return 0;
	len = sizeof(up->ut_host);
	p = up->ut_host;
	if (xdr_bytes(xdrsp, &p, &len, len) == FALSE)
		return 0;
	if (xdr_long(xdrsp, &up->ut_time) == FALSE)
		return 0;
	return 1;
}

xdr_utmpptr(xdrsp, up)
	XDR *xdrsp;
	struct utmp **up;
{
	return (xdr_reference(xdrsp, up, sizeof (struct utmp), xdr_utmp));
}

xdr_utmparr(xdrsp, up)
	XDR *xdrsp;
	struct utmparr *up;
{
	return (xdr_array(xdrsp, &up->uta_arr, &(up->uta_cnt),
	    MAXUSERS, sizeof(struct utmp *), xdr_utmpptr));
}
