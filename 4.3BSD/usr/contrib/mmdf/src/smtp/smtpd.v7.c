/*
 *		S M T P D . C
 *
 *	For 4.1a style networking on a PDP-11.
 */

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <NET/longid.h>
#include <NET/net_types.h>
#include <NET/socket.h>
#include <NET/in.h>

extern	int errno;

struct sockaddr_in addr = { AF_INET };
struct sockaddr_in rmtaddr;	

char	*Smtpserver;		/* Actual smtp server process, full path */
char	*Channel;
int	Maxconnections = 4;	/* Maximum simultaneous connections */
int	numconnections = 0;	/* Number of currently active connections */
int	debug = 0;		/* If nonzero, give verbose output on stderr */
char	errbuf[BUFSIZ];		/* Logging will be line buffered */
char	programid[40];		/* identification string for log entries */

main (argc, argv)
int	argc;
char	**argv;
{
	register int	skt;
	int	pid;
	int	i;
	int	alarmtrap();
	char	*them;
	char	thishost[32];
	char	workarea[32];

	setbuf( stderr, errbuf );
	sprintf( programid, "smtpd(%5.5d): ", getpid());
	gethostname( thishost, sizeof(thishost));	/* current hostname */

	/* Parse args; args will override configuration file... */
	arginit( argc, argv );

	/* Alarms will be used to avoid getting trapped in a wait call */
	signal( SIGALRM, &alarmtrap );

	while (1)
	{
		/*
		 *  Create a socket to accept a SMTP connection on.
		 *  The doloop is due to some sort of IPC bug in 4.1a.
		 */
		if (debug) log("opening socket...");
		i = 60;
		addr.sin_port = htons(IPPORT_SMTP);   /* swap bytes */
		do{
			skt = socket( SOCK_STREAM, 0, &addr,
				      SO_ACCEPTCONN | SO_KEEPALIVE );
			if( debug ) log( "socket returned %d", skt);
			if( skt < 0 )
				sleep( 5 );
		} while( skt < 0 && i-- > 0 );
		if( skt < 0 ) {
			log( "can't open socket (errno %d)",errno);
			continue;
		}
		if( debug ) log( "socket open on %d", skt );

		/*
		 *  Accept a connection.
		 */
		if( accept( skt, &rmtaddr )) {
			if( debug ) log( "accept error (%d)", errno);
			close( skt );
			sleep(1);
			continue;
		}

		if(( pid = fork()) < 0 ) {
			log( "could not fork (%d)", errno);
			close( skt );
		} else if( pid == 0 ) {
			/*
			 *  Child
			 */
			them = raddr( rmtaddr.sin_addr );
			if( them == (-1) )  {
				/* This should produce a dotted quad */
				sprintf( workarea, "%u.%u.%u.%u",
					rmtaddr.sin_addr.s_net&0xff,
					rmtaddr.sin_addr.s_host&0xff,
					rmtaddr.sin_addr.s_lh&0xff,
					rmtaddr.sin_addr.s_impno&0xff);
				them = &workarea[0];
			}
			log( "%s started", them);

			dup2( skt, 0 );
			dup2( skt, 1 );
			if( skt > 1 )
				close( skt );
			execl (Smtpserver, "smtpsrvr", them,
				    thishost, Channel, (char *)0);
			log( "server exec error (%d)", errno);
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
		if( numconnections > 1 || Maxconnections == 1 )
			do {
				alarm( 2 );
				if( wait( &i ) > 0 )
					numconnections--;
				alarm( 0 );
			} while( numconnections >= Maxconnections );
	}
}

alarmtrap() {
	signal( SIGALRM, &alarmtrap );
	return;
}

arginit( argc, argv )
int	argc;
char	**argv;
{
	register int	i;
	register char	*arg;

	if( argc == 1 ) {
		log( "Usage: smtpd [-d] [-n #connections] smtpserver channel" );
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

		case 'n':
			if( ++i == argc ) {
				log( "missing number of connections" );
				exit( 99 );
			}
			Maxconnections = atoi( argv[i] );
			if( Maxconnections <= 0 ) {
				log( "Bad number of connections '%s'",
				    argv[i] );
				exit( 99 );
			}
			log( "Maxconnection now %d", Maxconnections );
			break;
		}
	}

	if( i < argc ) {
		if( access( argv[i], 01 ) < 0 )	{	/* execute privs? */
			log( "Cannot access server program '%s'", argv[i] );
			exit( 99 );
		}
		Smtpserver = argv[i];
		if( debug ) log( "server is '%s'", Smtpserver );
	} else {
		log( "Smtpserver program not specified!" );
		log( "Usage: smtpd [-d] [-n #connections] smtpserver channel" );
		exit( 99 );
	}

	if( ++i < argc ) {
		Channel = argv[i];
		if( debug ) log( "channel is '%s'", Smtpserver );
	} else {
		log( "Channel not specified!" );
		log( "Usage: smtpd [-d] [-n #connections] smtpserver channel" );
		exit( 99 );
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
