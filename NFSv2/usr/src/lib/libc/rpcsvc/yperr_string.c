#ifndef lint
static	char sccsid[] = "@(#)yperr_string.c 1.1 85/05/30 Copyr 1985 Sun Micro";
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
 * This returns a pointer to an error message string appropriate to an input
 * yp error code.  An input value of zero will return a success message.
 * In all cases, the message string will start with a lower case chararacter,
 * and will be terminated neither by a period (".") nor a newline.
 */
char *
yperr_string(code)
	int code;
{
	char *pmesg;
	
	switch (code) {

	case 0:  {
		pmesg = "yp operation succeeded";
		break;
	}
		
	case YPERR_BADARGS:  {
		pmesg = "args to yp function are bad";
		break;
	}
	
	case YPERR_RPC:  {
		pmesg = "RPC failure on yp operation";
		break;
	}
	
	case YPERR_DOMAIN:  {
		pmesg = "can't bind to a server which serves domain";
		break;
	}
	
	case YPERR_MAP:  {
		pmesg = "no such map in server's domain";
		break;
	}
		
	case YPERR_KEY:  {
		pmesg = "no such key in map";
		break;
	}
	
	case YPERR_YPERR:  {
		pmesg = "internal yp server or client error";
		break;
	}
	
	case YPERR_RESRC:  {
		pmesg = "local resource allocation failure";
		break;
	}
	
	case YPERR_NOMORE:  {
		pmesg = "no more records in map database";
		break;
	}
	
	case YPERR_PMAP:  {
		pmesg = "can't communicate with portmapper";
		break;
		}
		
	case YPERR_YPBIND:  {
		pmesg = "can't communicate with ypbind";
		break;
		}
		
	case YPERR_YPSERV:  {
		pmesg = "can't communicate with ypserv";
		break;
		}
		
	case YPERR_NODOM:  {
		pmesg = "local domain name not set";
		break;
	}

	default:  {
		pmesg = "unknown yp client error code";
		break;
	}
	
	}

	return(pmesg);
}
