#ifndef lint
static  char sccsid[] = "@(#)yppasswdxdr.c 1.1 85/05/30 Copyr 1985 Sun Micro";
#endif

/*
 * Copyright (c) 1985 by Sun Microsystems, Inc.
 */

#include <stdio.h>
#include <rpc/rpc.h>
#include <pwd.h>
#include <rpcsvc/yppasswd.h>

xdr_yppasswd(xdrsp, pp)
	XDR *xdrsp;
	struct yppasswd *pp;
{
	if (xdr_wrapstring(xdrsp, &pp->oldpass) == 0)
		return (0);
	if (xdr_passwd(xdrsp, &pp->newpw) == 0)
		return (0);
	return (1);
}

xdr_passwd(xdrsp, pw)
	XDR *xdrsp;
	struct passwd *pw;
{
	if (xdr_wrapstring(xdrsp, &pw->pw_name) == 0)
		return (0);
	if (xdr_wrapstring(xdrsp, &pw->pw_passwd) == 0)
		return (0);
	if (xdr_int(xdrsp, &pw->pw_uid) == 0)
		return (0);
	if (xdr_int(xdrsp, &pw->pw_gid) == 0)
		return (0);
	if (xdr_wrapstring(xdrsp, &pw->pw_gecos) == 0)
		return (0);
	if (xdr_wrapstring(xdrsp, &pw->pw_dir) == 0)
		return (0);
	if (xdr_wrapstring(xdrsp, &pw->pw_shell) == 0)
		return (0);
}










