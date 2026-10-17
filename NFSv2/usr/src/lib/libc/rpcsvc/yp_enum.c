#ifndef lint
static	char sccsid[] = "@(#)yp_enum.c 1.1 85/05/30 Copyr 1985 Sun Micro";
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
extern int _ypclnt_dofirst();
extern int _ypclnt_donext();
extern int _ypsleeptime;
extern int _yp_dobind();

int ypclnt_first();
int ypclnt_next();
int _ypclnt_filter();

static char ypsymbol_prefix[] = YPSYMBOL_PREFIX;
static int ypsymbol_prefix_length = YPSYMBOL_PREFIX_LENGTH;

/*
 * This requests the yp server associated with a given domain to return the
 * first key/value pair from the map data base.  The returned key should be
 * used as an input to the call to yp_next.  This uses ypclnt_first to do
 * the real work, and differs from that function in that the yp internal
 * symbols are filtered out here.
 */
int
yp_first (domain, map, key, keylen, val, vallen)
	char *domain;
	char *map;
	char **key;		/* return: key array */
	int  *keylen;		/* return: bytes in key */
	char **val;		/* return: value array */
	int  *vallen;		/* return: bytes in val */
{
	int result;
	
	result = ypclnt_first (domain, map, key, keylen, val, vallen);

	if (result) {
		return(result);
	} else {
		return(_ypclnt_filter (domain, map, key, keylen, val, vallen) );
	}

}

/*
 * This requests the yp server associated with a given domain to return the
 * "next" key/value pair from the map data base.  The input key should be one
 * returned by yp_first or a previuos call to yp_next.  The returned key should
 * be used as an input to the next call to yp_next. This uses ypclnt_next to
 * do the real work, and differs from that function in that the yp internal
 * symbols are filtered out here.
 */
int
yp_next (domain, map, inkey, inkeylen, outkey, outkeylen, val, vallen)
	char *domain;
	char *map;
	char *inkey;
	int  inkeylen;
	char **outkey;		/* return: key array associated with val */
	int  *outkeylen;	/* return: bytes in key */
	char **val;		/* return: value array associated with outkey */
	int  *vallen;		/* return: bytes in val */
{
	int result;
	
	result = (ypclnt_next (domain, map, inkey, inkeylen, outkey, outkeylen,
	    val, vallen) );

	if (result) {
		return(result);
	} else {
		return(_ypclnt_filter (domain, map, outkey, outkeylen,
		    val, vallen) );
	}

}

/*
 * This requests the yp server associated with a given domain to return the
 * first key/value pair from the map data base.  The returned key should be
 * used as an input to the call to ypclnt_next.
 */
int
ypclnt_first (domain, map, key, keylen, val, vallen)
	char *domain;
	char *map;
	char **key;		/* return: key array */
	int  *keylen;		/* return: bytes in key */
	char **val;		/* return: value array */
	int  *vallen;		/* return: bytes in val */
{
	int domlen;
	int maplen;
	struct dom_binding *pdomb;
	int reason;

	if ( (map == NULL) || (domain == NULL) ) {
		return(YPERR_BADARGS);
	}
	
	domlen = strlen(domain);
	maplen = strlen(map);
	
	if ( (domlen == 0) || (domlen > YPMAXDOMAIN) ||
	    (maplen == 0) || (maplen > YPMAXMAP) ) {
		return(YPERR_BADARGS);
	}

	for (;;) {
		
		if (reason = _yp_dobind(domain, &pdomb) ) {
			return(reason);
		}

		reason = _ypclnt_dofirst(domain, map, pdomb, _ypserv_timeout,
		    key, keylen, val, vallen);

		if (reason == YPERR_RPC) {
			yp_unbind(domain);
			sleep(_ypsleeptime);
		} else {
			break;
		}
	}
	
	return(reason);
}

/*
 * This requests the yp server associated with a given domain to return the
 * "next" key/value pair from the map data base.  The input key should be
 * one returned by ypclnt_first or a previous call to ypclnt_next.  The
 * returned key should be used as an input to the next call to ypclnt_next.
 */
int
ypclnt_next (domain, map, inkey, inkeylen, outkey, outkeylen, val, vallen)
	char *domain;
	char *map;
	char *inkey;
	int  inkeylen;
	char **outkey;		/* return: key array associated with val */
	int  *outkeylen;	/* return: bytes in key */
	char **val;		/* return: value array associated with outkey */
	int  *vallen;		/* return: bytes in val */
{
	int domlen;
	int maplen;
	struct dom_binding *pdomb;
	int reason;


	if ( (map == NULL) || (domain == NULL) ) {
		return(YPERR_BADARGS);
	}
	
	domlen = strlen(domain);
	maplen = strlen(map);
	
	if ( (domlen == 0) || (domlen > YPMAXDOMAIN) ||
	    (maplen == 0) || (maplen > YPMAXMAP) ||
	    (inkeylen == 0) || (inkey == NULL) ) {
		return(YPERR_BADARGS);
	}

	for (;;) {
		if (reason = _yp_dobind(domain, &pdomb) ) {
			return(reason);
		}

		reason = _ypclnt_donext (domain, map, inkey, inkeylen, pdomb,
		    _ypserv_timeout, outkey, outkeylen, val, vallen);

		if (reason == YPERR_RPC) {
			yp_unbind(domain);
			sleep(_ypsleeptime);
		} else {
			break;
		}
	}
	
	return(reason);
}

/*
 * This continues to get "next" key-value pairs from the map while the
 * key-value pairs which come back have keys which are yp private symbols.
 */
int
_ypclnt_filter (domain, map, key, keylen, val, vallen)
	char *domain;
	char *map;
	char **key;		/* return: key array */
	int  *keylen;		/* return: bytes in key */
	char **val;		/* return: value array */
	int  *vallen;		/* return: bytes in val */
{
	char *inkey;
	int inkeylen;
	int result = 0;

	/*
	 * Keep trying to get the next key-value pair as long as we
	 * (1) continue to succeed, and
	 * (2) the key we get back is a yp reserved symbol.
	 */

 	inkey = NULL;
	inkeylen = 0;
 
	while ( (!result) &&
	    (!bcmp(*key, ypsymbol_prefix, ypsymbol_prefix_length) ) ) {
		inkey = *key;
		inkeylen = *keylen;
		*key = NULL;
		*keylen = 0;
		free(*val);
		*val = NULL;
		*vallen = 0;
		result = ypclnt_next (domain, map, inkey, inkeylen,
		    key, keylen, val, vallen);
		free(inkey);
		inkeylen = 0;
	}

	return(result);
}

