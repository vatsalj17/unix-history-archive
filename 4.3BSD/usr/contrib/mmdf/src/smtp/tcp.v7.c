/*
 *		T C P _ U I C P . C
 *
 *	Open a TCP/SMTP connection using Berkeley 4.2 style networking.
 *
 *	Doug Kingston, 3 Jan 83 at BRL.
 */
#include "util.h"		/* includes <sys/types.h>, and others */
#include "mmdf.h"
#include <NET/longid.h>
#include <NET/net_types.h>
#include <NET/socket.h>
#include <NET/in.h>

extern LLog *logptr;
extern int errno;

tc_uicp (addr, sock, timeout, fds)
long addr;
long sock;	/* IGNORED */	/* absolute socket number       */
int timeout;			/* time to wait for open        */
Pip *fds;
{
	register int skt;
	struct sockaddr_in saddr;

	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(IPPORT_SMTP);
	saddr.sin_addr.s_addr = htonl(addr);

	skt = socket( SOCK_STREAM, 0, (struct sockaddr_in *) 0, SO_DONTLINGER );
	if( skt < 0 ) {
		ll_log( logptr, LLOGFST, "Can't get socket (%d)", errno);
		return( RP_LIO );
	}

	if (setjmp(timerest)) {
	    close(skt);
	    return ( RP_TIME );
	}

	s_alarm( timeout );
	if( connect(skt, &saddr) < 0 ) {
		close( skt );
		s_alarm( 0 );
		ll_log( logptr, LLOGFST, "Can't get connection (%d)", errno);
		return( RP_DHST );
	}
	s_alarm( 0 );
	fds -> pip.prd = skt;
	fds -> pip.pwrt = dup(skt);
	return (RP_OK);
}
