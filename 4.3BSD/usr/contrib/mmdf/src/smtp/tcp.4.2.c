/*
 *		T C P . 4 . 2 . C
 *
 *	Open a TCP/SMTP connection using Berkeley 4.2 style networking.
 *
 *	Doug Kingston, 3 Jan 83 at BRL.
 */
#include "util.h"		/* includes <sys/types.h>, and others */
#include "mmdf.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

extern LLog *logptr;
extern int errno;

/*ARGSUSED*/
tc_uicp (addr, sock, timeout, fds)
long addr;
long sock;	/* IGNORED */	/* absolute socket number       */
int timeout;			/* time to wait for open        */
Pip *fds;
{
	register int skt;
	struct sockaddr_in haddr;
	short	smtpport = 0;
#ifdef notdef
	int	on = 1;
#endif

	if (smtpport == 0) {
		struct servent *sp;

		if ((sp = getservbyname("smtp", "tcp")) == NULL)
			return (RP_NO);
		smtpport = (short)sp->s_port;
	}

	haddr.sin_family = AF_INET;
	haddr.sin_port = smtpport;
	haddr.sin_addr.s_addr = htonl(addr);

	skt = socket( AF_INET, SOCK_STREAM, 0 );
	if( skt < 0 ) {
		ll_log( logptr, LLOGFST, "Can't get socket (%d)", errno);
		return( RP_LIO );
	}

#ifdef notdef
	if (setsockopt (skt, SOL_SOCKET, SO_DONTLINGER, &on, sizeof on)) {
		ll_log( logptr, LLOGFST, "Can't set socket options (%d)", errno);
		close (skt);
		return( RP_LIO );
	}
#endif notdef
	
        if (setjmp(timerest)) {
            close(skt);
            return ( RP_TIME );
        }
        
        s_alarm( (unsigned) timeout );
	if( connect(skt, &haddr, sizeof haddr) < 0 ) {
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
