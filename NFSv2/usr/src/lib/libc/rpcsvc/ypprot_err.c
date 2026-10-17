#ifndef lint
static	char sccsid[] = "@(#)ypprot_err.c 1.1 85/05/30 Copyr 1985 Sun Micro";
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

/*
 * Maps a yp protocol error code (as defined in
 * yp_prot.h) to a yp client interface error code (as defined in
 * ypclnt.h).
 */
int
_map_ypprot_err(yp_protocol_error)
	int yp_protocol_error;
{
	int reason;

	switch (yp_protocol_error) {
	
 	case YP_NOMORE: {
		reason = YPERR_NOMORE;
		break;
	}

 	case YP_NOMAP: {
		reason = YPERR_MAP;
		break;
	}

 	case YP_NODOM: {
		reason = YPERR_DOMAIN;
		break;
	}

 	case YP_NOKEY: {
		reason = YPERR_KEY;
		break;
	}

 	case YP_BADARGS: {
		reason = YPERR_BADARGS;
		break;
	}

	default: {
		reason = YPERR_YPERR;
		break;
	}
	
	}
	
  	return(reason);
}
