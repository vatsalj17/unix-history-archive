#ifndef lint
static	char sccsid[] = "@(#)yp_match.c 1.1 85/05/30 Copyr 1985 Sun Micro";
#endif

#include <dbm.h>			/* Pull this in first */
#undef NULL				/* Remove dbm.h's definition of NULL */
extern void dbmclose();			/* Refer to dbm routine not in dbm.h */
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <rpcsvc/yp_prot.h>
#include <rpcsvc/ypclnt.h>
extern struct timeval _ypserv_timeout;
extern int _ypclnt_domatch();
extern int _yp_dobind();
extern int _ypsleeptime;

/*
 * Requests the yp server associated with a given domain to attempt to match
 * the passed key datum in the named map, and to return the associated value
 * datum.
 */
int
yp_match (domain, map, key, keylen, val, vallen)
	char *domain;
	char *map;
	char *key;
	int  keylen;
	char **val;		/* returns value array */
	int  *vallen;		/* returns bytes in val */
{
	int domlen;
	int maplen;
	int reason;
	struct dom_binding *pdomb;

	if ( (map == NULL) || (domain == NULL) ) {
		return(YPERR_BADARGS);
	}
	
	domlen = strlen(domain);
	maplen = strlen(map);
	
	if ( (domlen == 0) || (domlen > YPMAXDOMAIN) ||
	    (maplen == 0) || (maplen > YPMAXMAP) ||
	    (key == NULL) || (keylen == 0) ) {
		return(YPERR_BADARGS);
	}

	for (;;) {
		
		if (reason = _yp_dobind(domain, &pdomb) ) {
			return(reason);
		}

		reason = _ypclnt_domatch (domain, map, key, keylen, pdomb,
		    _ypserv_timeout, val, vallen);

		if (reason == YPERR_RPC) {
			yp_unbind(domain);
			sleep(_ypsleeptime);
		} else {
			break;
		}
	}
	
	return(reason);

}
