/*
 *                      S M T P D A E M O N . C
 *
 *  Server SMTP daemon, for 4.1a/4.2 BSD by DPK at BRL, 1 Jan 82 (Sigh...)
 *
 *              R E V I S I O N   H I S T O R Y
 *
 *      02/02/83  MJM   Modified to be more robust.
 *
 *      03/18/83  DPK   Modified for 4.1c.
 */

#include "util.h"
#include <signal.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/ioctl.h>

extern  int errno;

struct sockaddr_in addr;

char    *Smtpserver = 0;        /* Actual smtp server process, full path */
char    *Channel = 0;           /* Logical channel mail will arrive on */
int     Maxconnections = 4;     /* Maximum simultaneous connections */
int     numconnections = 0;     /* Number of currently active connections */
int     debug = 0;              /* If nonzero, give verbose output on stderr */
char    errbuf[BUFSIZ];         /* Logging will be line buffered */
char    programid[40];          /* identification string for log entries */
int     stricked;               /* force full validity of source address */

main (argc, argv)
int     argc;
char    **argv;
{
	register int    skt;
	int     pid;
	int     i;
	char    thishost[64];
	char    workarea[32];
	struct  servent *sp;
	struct  hostent *hp;

	setbuf( stderr, errbuf );
	sprintf( programid, "smtpdae(%5.5d): ", getpid());
	gethostname( thishost, sizeof(thishost));       /* current hostname */
	if ((sp = getservbyname("smtp", "tcp")) == NULL) {
		fprintf(stderr, "Cannot find service smtp/tcp\n");
		exit(-1);
	}

	/*
	 * try to check if name is complete.
	 */
	if( (hp = gethostbyname(thishost)) != NULL)
	    strcpy(thishost, hp->h_name);

	/* Parse args; args will override configuration file... */
	arginit( argc, argv );

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = (short)sp->s_port;

	disconnect ();          /* close off contact with controlling TTY */

	while (1)
	{
		struct sockaddr_in rmtaddr;
		int     tmpskt;
		int     status;

		/*
		 *  Create a socket to accept a SMTP connection on.
		 */

		if (debug) log("opening socket...");
		skt = socket(SOCK_STREAM, 0, &addr, SO_ACCEPTCONN);
		if( skt < 0 ) {
			if (debug) log( "can't open socket (errno %d)", errno);
			exit (99);
		}
		if( debug ) log( "socket open on %d", skt );
		/*
		 *  Accept a connection.
		 */
		if(accept(skt, &rmtaddr) < 0) {
			if( debug ) log( "accept error (%d)", errno);
			sleep(1);
			continue;
		}

		/* We have a valid open connection, start a server... */
		if(( pid = fork()) < 0 ) {
			if (debug) log( "could not fork (%d)", errno);
			close( skt );
		} else if( pid == 0 ) {
			/*
			 *  Child
			 */
			char    *rmt;

			hp = gethostbyaddr(&rmtaddr.sin_addr,
					sizeof(rmtaddr.sin_addr), AF_INET);
			if((hp == NULL) || !isstr(hp->h_name))
				rmt = inet_ntoa(rmtaddr.sin_addr);
			else
				rmt = hp->h_name;

			if (debug) log( "%s started", rmt);
			dup2( skt, 0 );
			dup2( skt, 1 );
			if( skt > 1 )
				close( skt );
			execl (Smtpserver, stricked? "rsmtpsrvr" : "smtpsrvr",
						rmt, thishost, Channel, 0);
			if (debug) log( "server exec error (%d)", errno);
			exit (99);
		}

		/*
		 *  Parent
		 */
		close( skt );
		numconnections++;

		/*
		 *  This code collects ZOMBIES and implements load
		 *  limiting by staying in the do loop while the
		 *  Maxconnections active.
		 */
		while (wait2 (&status, numconnections < Maxconnections
		    ? WNOHANG : 0, 0) > 0)
			numconnections--;
	}
}

arginit( argc, argv )
int     argc;
char    **argv;
{
	register int    i;
	register char   *arg;

	if( argc == 1 ) {
		log( "Usage: %s [-d] [-n #connections] smtpserver channel", 
			argv[0]);
		exit( 1 );
	}

	for( i = 1; i < argc; i++ ) {
		if( *argv[i] != '-' )
			break;
		arg = argv[i];
		arg++;
		switch( *arg ) {
		case 'd':
			debug++;
			break;

		case 'f':       /* force correctness of addresses */
			stricked++;
			break;

		case 'n':
			if( ++i == argc ) {
				if (debug) log( "missing number of connections" );
				exit( 99 );
			}
			Maxconnections = atoi( argv[i] );
			if( Maxconnections <= 0 ) {
				if (debug) log( "Bad number of connections '%s'",
				    argv[i] );
				exit( 99 );
			}
			if (debug) log( "Maxconnection now %d", Maxconnections );
			break;
		}
	}

	if( i < argc ) {
		if( access( argv[i], 01 ) < 0 ) {       /* execute privs? */
			if (debug) log( "Cannot access server program '%s'", argv[i] );
			exit( 99 );
		}
		Smtpserver = argv[i];
		if( debug ) log( "server is '%s'", Smtpserver );
	} else {
		if (debug) log( "Smtpserver program not specified!" );
		exit( 99 );
	}

	if( ++i < argc ) {
		Channel = argv[i];
		if( debug ) log( "channel is '%s'", Channel);
	} else {
		if (debug) log( "Channel not specified!" );
		exit (99);
	}
}

/* VARARGS */
log( fmt, a, b, c, d )
char *fmt;
{
	fputs( programid, stderr );
	fprintf( stderr, fmt, a, b, c, d );
	fputc( '\n', stderr );
	fflush( stderr );
}
/*
**  DISCONNECT -- remove our connection with any foreground process
**
**      Parameters:
**              fulldrop -- if set, we should also drop the controlling
**                      TTY if possible -- this should only be done when
**                      setting up the daemon since otherwise UUCP can
**                      leave us trying to open a dialin, and we will
**                      wait for the carrier.
**
**      Returns:
**              none
**
**      Side Effects:
**              Trys to insure that we are immune to vagaries of
**              the controlling tty.
*/

disconnect ()
{
	int fd;

	if( (fd = open("/dev/tty", 2)) >= 0){
		(void) ioctl(fd, TIOCNOTTY, 0);
		(void) close(fd);
	}
	errno = 0;
}
