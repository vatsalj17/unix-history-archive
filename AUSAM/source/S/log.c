/*
 *	log  remote_tty
 *
 *  File exchange package for remote unices.
 *
 *  Also has transparent mode for talking to
 *  other operating systems as if local tty.
 *
 *  For the following reasons this program must be
 *  setuid to root for successful use:
 *	1. The tty to be used for communication with other machine
 *	   is to be owned by root and is to have a mode of 600.
 *	2. Needs to run at fairly high priority,
 *	   so nice will be used to obtain same.
 *	3. The connect system call only allowed to be
 *	   used by root, to prevent abuse.
 *
 * Multiple opens of the same tty are assumed
 *  to be taken care of by the appropriate tty
 *  drivers (eg dz & dj) supporting the notion
 *  of certain ttys being single open, and that
 *  ttys used by 'log' for communications are
 *  setup as single open.
 *
 *	Ian Johnstone - AGSM - 1976
 *	Peter Ivanov  - UNSW - 1976
 *
 *	Modified for flow control
 *	 Piers Lauder - Sydney University - Aug '77
 *
 *	Modified for binary file protocol
 *	 Piers Lauder - Sydney University - Sep '77
 *
 *	Modified for settable quit code & shell escape & default file args
 *	 Piers Lauder - Sydney University - Feb '78
 *	 Ian Johnstone - AGSM - Feb '78
 *
 *	Updated to use 'connect' system call.
 *	 Ian Johnstone - AGSM - MAY '78
 *
 *	Updated to use 'access' to validate file access,
 *	 since 'log' will always be setuid to root from
 *	 this time onwards.
 *	 Ian Johnstone - AGSM - AUG '78
 *	 Piers Lauder - Sydney University - Aug '78
 *
 *	Corrected a timing bug that only showed up when using
 *	 high-speed log lines (say >= 4800 baud).  Corrected
 *	 some minor bugs and generally tidied up the program
 *	 (rewrote some grotty procedures!).
 *	 Kevin Hill - UNSW - JAN '79
 *
 *	Enabled handling of multiple lines to remote site.  Lines
 *	 to be called (in order of trying) xxx,xxx1,xxx2,...xxx9.
 *	 This enables a user to merely say 'log xxx', to have a
 *	 log attempted on all appropraitely named lines.
 *	 Ian Johnstone - AGSM - AUG '79
 *
 *	To enable realistic use of the new 'disconnect' command
 *	 (disconnect breaks a connection) the log special file
 *	 being used by log is set up to be owned by the real
 *	 user of log so that that user may reasonably break his/her
 *	 own log !!
 *	 Ian Johnstone - AGSM - AUG '79
 *
 *	Fix some more little bugs.  Only the super-user may log to
 *	 a device not in the `quits' table.  `Access' used if the user
 *	 tries to change directory.  Change owner of remote special back
 *	 to root when the logger has finished.  Complain if super-user
 *	 tries to log to a non-existent device.
 *	 Kevin Hill - UNSW - AUG 79
 */

#include	<local-system>
#include	<errnos.h>

#ifdef	AUSAM
#include	<stat16.h>
	/*
	 *	Piers Lauder	Feb '78
	 *
	 *	Use 16-bit user id.
	 */
#else
#include	<stat.h>
#endif	AUSAM


#define	HIPRIORITY	-40
#define	USERPRI		0


/*
 * Regulate file transfer with a file protocol as follows:-
 *
 *  Blocks consist of a header followed by data followed by a trailer.
 *  The header contains a sequence number (0/1) and the size
 *   (transmitted twice, 2nd as complement of first).
 *  The trailer contains a longitudinal parity check of the data,
 *   followed by a sum check of the data.
 *
 *  Block format is
 *	STX SEQ SIZ ~SIZ ...DATA... LPC SUM
 *
 *  Blocks are acknowledged by a 2 byte reply consisting of
 *   ACK or NAK followed by the relevant sequence number.
 *
 * Note: that the DATAZ is really dependant on the accuracy
 *	 of the crc check.
 */

#define	REPLYZ		2
#define	RTYPE		0
#define	RSEQ		1

struct
{
	char r_typ;
	char r_seq;
}
reply;

#define	HEADER		4
#define	BSEQ		1
#define	TRAILER		2
#define OVERHEAD	(HEADER+TRAILER)
#define	DATAZ		64
#define	DATAZL		64L
#define	BUFZ		(DATAZ+OVERHEAD)

/*
 *  Timeouts - these must be tailored to the line speed,
 *   the basic formulae used being:-
 *	receive timeout = receive time + turnaround time
 *	transmit   "    = 2 * receive timeout
 *	(all times in seconds)
 */
#define	STARTTIMEOUT	10
#define	FLUSHTIMEOUT	1
#define	TURNAROUND	3
#define	XRETRYS		4
#define	RRETRYS		5
#define	GARBAGE		2	/* max garbage chars tolerated before reset */
#define	EXTA		200	/* per DZ11 */
#define	EXTB		1920	/* per DZ11 */
#define	DEFAULTRATE	120	/* 1200 baud default */

int	byterate[16]	/* per DH11 */
{
	0,5,8,10,15,15,20,30,60,120,
	180,240,480,960,EXTA,EXTB
};

int	xtimeout, rtimeout;
long	xrate;			/* currently effective data transfer rate */
long	xtotal;			/* total bytes received/transmitted */
long	xtime;			/* total transmission time */

#define	SOH		001	/* remote accept */
#define	STX		002	/* start of block */
#define	ETX		003	/* remote finish */
#define	EOT		004	/* remote file access error */
#define	ENQ		005	/* remote timeout */
#define	ACK		006	/* xfer ok */
#define	DLE		020	/* file already exists */
#define	NAK		025	/* xfer failed */

#define	MSOH		"\01\01",2
#define	METX		"\03\03\03\03",4
#define	MEOT		"\04\04",2
#define	MTIMEOUT	"\05\05",2
#define	MDLE		"\020\020",2

#define	SIGTIMEOUT	15
#define	SIGQUIT		3
#define	SIGINTERRUPT	2

int	sav_modes;
#define	RAW	040
int	ttymode[3];
int	narg;
char	*cmd[10];
int	xfd	2;		/* fd for remote port */
char	otherm[]	"/dev/xxxxxxxxxxxxxxx";
char	c;
unsigned uid;
long	time();
long	filesize;

char exists[] "%s already exists - are you sure? ";

/* defines for use with access calls */
#define	R	04	/* read permission */
#define W	02	/* write permission */
#define	X	01	/* execute / directory search permission */


#define	ESCAPE	021	/* default escape char is ctrl/q */
#define	ESCAPED	"^q"	/* default escape description */
char	quit;		/* escape code */
char	*quitdescrpt;	/* escape code description */
struct
{
	char *name;
	char *descpt;	/* description of control code that follows */
	char  escape;	/* default escape code for this line */
	char  speed;	/* sets log line to this speed if non-zero */
}
quits[]
{
#ifdef	BASSER
	"agsm",		"^a",	001,	 9,
	"cyber",	"^d",   004,	 7,
#else
	"agsm",		"^a",	001,	12,
	"basser",	"^b",	002,	 9,
	"elec",		"^e",	005,	 0,
	"csu",		"^c",	003,	12,
	"forty",	"^f",	006,	13,
	"arch",		"^r",	022,	11,
	"cyber",	"^k",   013,	 7,
#endif	BASSER
};

struct	err
{
	int	e_count;
	char	*e_mess;
}
errs[]
{
#define	E_RETRYS	0
	{ 0,	"retrys" },
#define	E_SEQUENCE	1
	{ 0,	"seq" },
#define	E_SIZE		2
	{ 0,	"size" },
#define	E_OUTSIZE	3
	{ 0,	"outsize" },
#define	E_TIMEOUTS	4
	{ 0,	"timeouts" },
#define	E_CRC		5
	{ 0,	"crc" },
#define	E_WRITE		6
	{ 0,	"local write" },
#define	E_NAKS		7
	{ 0,	"naks" },
#define E_SYNC		8
	{ 0,	"sync" },
#define	E_READ		9
	{ 0,	"local read" }
};
#define	NERRS		((sizeof errs)/(sizeof errs[0]))

char	buf[BUFZ];

struct
{
	int hiword;
	unsigned loword;
};



/****************************************************************/

main (argc,argv)
register char	**argv;
register	argc;
{
	signal(SIGINTERRUPT, 1);
	signal(SIGQUIT, 1);
	nice(HIPRIORITY);
#ifdef	AUSAM
	uid = getreal();
#else
	uid = getuid() & 0377;
#endif	AUSAM

	sentinel();	/* catch all real-time limit expiries */

	switch (**argv)
	{
    case 'l':	if (okuse())
		{
			log (argc,argv);
			errors();	/* tell about the errors encountered */
		}
		else printf("Illegal usage!\n");
		break;

    case 's':	send (argc,argv);
		break;

    case 'r':	receive (argc,argv);
		break;

    default:	write(2, MEOT);
		return(1);
	}
}

/*****************************************************************/

log(argc,argv)
char **argv;
int argc;
{
	register char	*arg;
	register char	*otp;
	register int	i;
	extern char	*fquit();
	extern		finish();
	extern int errno;

	if (argc != 2)
	{
argerr:
		printf("Usage: log remote-special-file\n");
		exit(1);
	}

	arg = argv[1];
	otp = &otherm[5];
	for (i = 0; *otp++ = *arg++; ) if (++i >= 15) goto argerr;

	*otp-- = 0;		/* extra-null, address last null of name */
	while( (xfd = open(otherm,2)) < 0)
		if( (errno == EOPENFAIL) && (*otp!='9') )
		{
			if( *otp )
				(*otp)++;
			else
				*otp = '1';
		}
		else
		{
			if( errno == ENOENT && *otp)
				printf("%s busy\n",argv[1]);
			else
				perror(argv[1]);
			exit (2);
		}

	quit = ESCAPE;
	quitdescrpt = ESCAPED;
	for (i = 0; i < sizeof quits / sizeof quits[0]; i++)
	{
		arg = argv[1];
		otp = quits[i].name;
		while (*otp && *otp == *arg++) otp++;
		if (*otp == '\0')
		{
			quit = quits[i].escape;
			quitdescrpt = quits[i].descpt;
			break;
		}
	}
	if (i == sizeof quits / sizeof quits[0])	/* ran off the end */
	{
		if (uid)
		{
			printf("%s: illegal device\n", argv[1]);
			exit(1);
		}
		init_remote(xfd, 0);
	}
	else init_remote(xfd, quits[i].speed);
	chown(otherm,uid);	/* indicate current user of the log line */
				/* quicker than a ps */
	chmod(otherm,0600);	/* protect against other access, especially */
				/* from the disconnect command */
	signal(SIGINTERRUPT, finish);

	printf("* tty\n");
	tty();

	for (;;)
	{
		errors();
		printf("* ");

		switch(getline())
		{
	    case 'p':	if (narg == 3) put();	/* put a file to remote */
			break;

	    case 'g':	if (narg == 3) get();	/* get a file from remote */
			break;

	    case 't':	if (narg == 1) tty();	/* talk to remote as a terminal */
			break;

	    case 's':	if (narg == 1) finish();	/* stop this sillyness */
			break;

	    case 'q':	if (narg == 2)  quitdescrpt = fquit();	/* change quit code */
			break;

	    case '!':	sh();			/* local escape to shell */
			break;

	    case 'c':	if( narg == 2 )
			{
				if (access(cmd[1], X) ||
				 chdir(cmd[1]) == -1 ) perror(cmd[1]);
				break;
			}

	    default:	help();			/* unknown command */
			break;
		}
	}
}

/**************/
getline()
{
	static char	line[100];
	register char	*q;
	register	*p;
	register	toggle;

	if ((toggle = read(2,line,sizeof line)) == 1) return(-1);
	if (toggle <= 0) finish();
	if (line[toggle - 1] != '\n')
	{
		printf("Too verbose!!\n");
		do
			toggle = read(2, line, sizeof line);
		while (toggle > 0 && line[toggle - 1] != '\n');
		return -1;
	}
	for (q = line; q < &line[sizeof line] && *q == ' '; q++);
	if (*q == '!')
	{
		cmd[0] = q;
		narg = 1;
		q = &line[toggle - 1];
	}
	else
	{
		p = cmd; narg = 0; toggle = 1;
		for ( ; *q != '\n'; q++)
			if (*q == ' ')
			{
				toggle = 1;
				*q = 0;
			}
			else if (toggle)
			{
				*p++ = q;
				narg++;
				toggle = 0;
			}
	}
	*q = 0;
	q = cmd[0];
	if ((narg == 2) && ((*q == 'p') || (*q == 'g')))
	{
		*p++ = cmd[1];
		*p = 0;
		narg++;
	}
	if (narg == 0) return -1;
	else return *q;
}

/*******************/


fini_remote(fd)
register int fd;
{
	ttymode[2] = sav_modes;
	stty (fd, ttymode);
}

init_remote(fd, spd)
char spd;
register int fd;
{
	register speed;

	gtty (fd, ttymode);
	sav_modes = ttymode[2];
	ttymode[2] = RAW;
	if (spd) ttymode[0] = (spd << 8) | spd;
	stty (fd, ttymode);
	if ((speed = byterate[(ttymode[0] & 017)]) == 0)
		speed = DEFAULTRATE;
	xtimeout = 2 * (rtimeout = BUFZ / speed + TURNAROUND);
	xrate = (speed * DATAZL) / BUFZ;
}


set_rate(t)
long t;
{
	xtotal =+ filesize;
	xtime =+ t;
	xrate = xtotal/xtime;
}

/*********************/
finish()
{
	fini_remote(xfd);
	chown(otherm, 0);	/* change back to root */
#ifdef	BASSER
	chmod(otherm, 0606);	/* multi use line */
#endif	BASSER
	exit(0);
}

/*************************************************************/

char *fquit()
{
	static char	qs[3];
	register char	*qp = qs;

	if ((quit = *cmd[1] & 0177) == 037) return("US");	/* clear-screen char. */
	if (quit < 040)
	{
		*qp++ = '^';
		*qp++ = quit + 0140;
	}
	else *qp++ = quit;
	*qp = 0;
	return(qs);
}

/*************************************************************/

sh()
{
	char *args[4];
	register char **ap = args;
	register char *bp;
	register i = 0;
	static char shf[] "/bin/sh";

	cmd[0]++;
	*ap++ = shf+5;
	*ap++ = "-c";
	*ap++ = cmd[0];
	*ap = 0;

	if (!(i = fork()))
	{
		nice(USERPRI);
		setuid(uid);
		execv(shf, args);
		perror(shf);
		exit(-1);
	}
	if (i > 0)
	{
		signal(SIGINTERRUPT, 1);
		waitx(ap);
		signal(SIGINTERRUPT, finish);
	}
	else perror("try again");
}

/*************************************************************/

tty()
{

	/*
	 * logically connect local terminal to remote until quit char received
	 */
	printf("  %s -> %s   ('%s'=quit)\n",SYSTEMID,&otherm[5],quitdescrpt);
	if (connect(2, xfd, quit) < 0)
	{
		perror("connect failed");
		finish();
	}
}

/*************************************************************/

get()
{
	register	ofd;
	register	n;
	long		t;

	if ((ofd = accreat(cmd[2], 0)) < 0) return;

	transfer("send",cmd[1], 0);

	if (sync("open"))
	{
		printf(" -- receive rate approx. %D bytes/sec.\n", xrate);
		t = time();
		if (rproto(ofd, xfd) == 0)
		{
			if (!(t = time() - t))  t++;
			set_rate(t);
			printf("%s: %D bytes received in %D secs (%D bytes/sec)\n", cmd[2], filesize, t, filesize/t);
		}
		else printf(" -- transfer aborted after %D bytes\n", filesize);
	}

	lflush(xfd);
	close(ofd);
}

/*************************************************************/

put()
{
	register	ifd;
	register	n;
	register	fred;
	long		t;
	struct statbuf	statb;

	if (access(cmd[1],R) || ((ifd = open(cmd[1],0)) < 0))
	{
		perror(cmd[1]);
		return;
	}

	fstat(ifd, &statb);
	filesize.loword = statb.sb_size1;
	filesize.hiword = statb.sb_size0 & 0377;

	transfer("receive",cmd[2], 0);

	if ((n = sync("create")))
	{
		if (n < 0)
		{
			printf(exists, cmd[2]);
			fred = 0;
			if ((n = read(2, &c, 1)) == 1 && c == 'y') fred++;
			if (n == 1 && c != '\n')
				do
					n = read(2, &c, 1);
				while (n > 0 && c != '\n');
			if (fred == 0) return;
			lflush(xfd);
			transfer("receive", cmd[2], "-");
			if (sync("create") != 1) goto home;
		}

		printf(" -- estimated transmit time: %D secs\n", filesize/xrate);
		t = time();

		if ((n = xproto(ifd, xfd)) == 0)
		{
			if (!(t = time() - t))  t++;
			set_rate(t);
			printf("%s: %D bytes transmitted in %D secs (%D bytes/sec)\n", cmd[2], filesize, t, filesize/t);
		}
		else printf(" --%stransfer aborted after %D bytes\n", (n == 2) ? " remote terminate -- " : " ", filesize);
	}

home:
	lflush(xfd);
	close (ifd);
}

/*******************************/

transfer(s1, s2, s3)
char	*s1,*s2;
{
	register char	*p, *q;

	q = buf;

	*q++ = '@';
	*q++ = '@';	/* two will ALWAYS do the trick! */

	for (p = s1; *q++ = *p++; );
	q[-1] = ' ';
	for (p = s2; *q++ = *p++; );
	if (s3)
	{
		q[-1] = ' ';
		for (p = s3; *q++ = *p++; );
	}
	q[-1] = '\n';

	write (xfd, buf, q-buf);
}


/*******************************/

send(argc,argv)
char **argv;
{
	register  fd;

	init_remote(2, 0);

	if (argc != 2 || access(argv[1],R) || ((fd = open(argv[1],0))<0)) write(2, MEOT);
	else
	{
		write(2, MSOH);
		sleep(FLUSHTIMEOUT);
		xproto(fd, 2);
	}

	lflush(2);
	fini_remote(2);
}

/*******************************/

receive(argc,argv)
char **argv;
{
	register	fd;

	init_remote(2, 0);

	if (argc > 3 || (fd = accreat(argv[1], argc == 3 ? 2 : 1)) < 0) write(2, MEOT);
	else
	{
		write(2, MSOH);
		rproto(fd, 2);
	}

	lflush(2);
	fini_remote(2);
}

/********************************************************************/

int	timedout;

sentinel()
{
	signal(SIGTIMEOUT, sentinel);
	timedout++;
}


tread(f, b, s, timeout)
{
	clktim(timeout);
	timedout = 0;
	read(f, b, s);
	clktim(0);
	return(timedout);
}


lflush(fd)
{
	while (!tread(fd, &c, 1, FLUSHTIMEOUT));
	return(1);
}

/********************************************************************/

sync(remote_err)
char	*remote_err;
{
	do
	{
		if (tread(xfd, &c, 1, STARTTIMEOUT))
		{
			printf("no response from remote!\n");
			return(0);
		}
		if (c == EOT)
		{
			printf("can't %s remote file!\n", remote_err);
			return(0);
		}
		if (c == DLE) return -1;
	} while (c != SOH);

	lflush(xfd);
	return(1);
}

/******************************************************************/

xproto(local, remote)
{
	register  n, retrys, sync;

	buf[BSEQ] = 1;
	filesize = 0;

	while ((n = read(local, buf+HEADER, DATAZ)) >= 0)
	{
		crc(n);
		retrys = 0;

		do
		{
			write(remote, buf, n+OVERHEAD);

			sync = 0;
			while (tread(remote, &reply, 1, xtimeout) == 0)
			{
				switch (reply.r_typ)
				{
			     default:
					errs[E_SYNC].e_count++;
					if (++sync > GARBAGE)
						break;
					else
						continue;

			     case ENQ:
			     case ACK:
			     case NAK:
			     case ETX:
					tread(remote, &reply.r_seq, 1, xtimeout);
				}
				break;
			}

			if (reply.r_typ == ETX && reply.r_seq == ETX)
				return(n ? 2 : n);	/* remote finish */

		} while (

			  ( (sync > GARBAGE && lflush(remote))

			  || (timedout && ++errs[E_TIMEOUTS].e_count)

			  || (reply.r_typ != ACK && ++errs[E_NAKS].e_count)

			  || (reply.r_seq != buf[BSEQ] && ++errs[E_SEQUENCE].e_count)

			 )

			 && retrys++ < XRETRYS

			);

		errs[E_RETRYS].e_count =+ retrys;
		if (retrys > XRETRYS)
		{
			errs[E_RETRYS].e_count--;
			return(1);
		}

		if (n == 0)	/* eof */
			return(n);

		filesize =+ n;
	}

	errs[E_READ].e_count++;

	return(3);
}

/*******************************************************************/

/* case labels */
#define	NEWBLOCK	0
#define	SEQUENCE	1
#define	SIZE1		2
#define	SIZE2		3

rproto(local, remote)
{
	register  size, state, x;
	int  wretrys, lastsize, retrys, garbage;
	char  lastseq;

	state = NEWBLOCK;
	retrys = 0;
	reply.r_seq = 0;
	filesize = 0;
	lastseq = -1;
	garbage = 0;
	lastsize = 0;

	for (;;)
	{

		while (tread(remote, &c, 1, rtimeout))
			if (retrys++ < RRETRYS)
			{
				errs[E_TIMEOUTS].e_count++;
				write(remote, MTIMEOUT);
			}
			else
			{
	fail:
				write(remote, METX);
				return(1);
			}

		x = c & 0377;

		switch(state)
		{

	    case NEWBLOCK:
			if (x == STX)
			{
				state = SEQUENCE;
				retrys = 0;
				garbage = 0;
			}
			else
			{
				if (garbage++ >= GARBAGE)
				{
					lflush(remote);
					garbage = 0;
				}
			}
			break;

	    case SEQUENCE:
			if (x & ~1)
			{
				errs[E_SEQUENCE].e_count++;
fnak:
				state = NEWBLOCK;
				lflush(remote);
nak:
				reply.r_typ = NAK;
				errs[E_NAKS].e_count++;
rply:
				write(remote, &reply, REPLYZ);
				break;
			}
			reply.r_seq = x;
			state = SIZE1;
			break;

	    case SIZE1:
			if ((size = x) > DATAZ)
			{
				errs[E_OUTSIZE].e_count++;
				goto fnak;
			}
			state = SIZE2;
			break;

	    case SIZE2:
			state = NEWBLOCK;
			if (size != (~x & 0377))
			{
				errs[E_SIZE].e_count++;

				goto fnak;
			}
			if (size == 0)	/* eof */
			{
				write(remote, METX);
				return(0);
			}

			if ((tread(remote, buf+HEADER, size+TRAILER, rtimeout) && ++errs[E_TIMEOUTS].e_count)
				|| (crc(size) && ++errs[E_CRC].e_count)
			  )
				goto nak;

			if (reply.r_seq == lastseq && lastsize > 0)
			{
				seek(local, -lastsize, 1);
				filesize =- lastsize;
			}

			wretrys = 0;
			while ((lastsize = write(local, buf+HEADER, size)) != size)
			{
				errs[E_WRITE].e_count++;
				if (wretrys++ >= XRETRYS)
					goto fail;
				if (lastsize > 0)
					seek(local, -lastsize, 1);
			}
			filesize =+ size;

			reply.r_typ = ACK;
			lastseq = reply.r_seq;
			goto rply;
		}
	}
}

/********************************************************************/

/*
 *	not a true crc, but nearly so for small 's' (< 100 say)
 */

crc(s)
{
	register char *p;
	register char lpc, sum;
	char  *end;
	int  error;

	p = buf;  lpc = 0;  sum = 0;  error = 0;

	*p++ = STX;
	*p++ =^ 1;		/* flip sequence number */
	*p++ = s;  *p++ = ~s;

	end = buf+HEADER+s;
	for ( ; p < end; p++)
	{
		lpc =^ *p;
		sum =+ *p;
	}

	if (lpc != *p)  error++;
	*p++ = lpc;
	if (sum != *p)  error++;
	*p++ = sum;

	return(error);
}

/**********************************************************************/

errors()
{
	register struct err *ep;
	register nerrs;

	nerrs = 0;

	for (ep = errs; ep < &errs[NERRS]; ep++)
	if (ep->e_count)
	{
		if (nerrs++ == 0)
			printf("  file transfer protocol errors:-\n");
		printf("  %s: %d", ep->e_mess, ep->e_count);
		ep->e_count = 0;
	}

	if (nerrs)  printf("\n");
	return(nerrs);
}

/***********************************************************************/

help()
{
	printf("\nuse:\n");
	printf("\tt[ty]\n");
	printf("\tp[ut] %s-file [%s-file]\n", SYSTEMID, &otherm[5]);
	printf("\tg[et] %s-file [%s-file]\n", &otherm[5], SYSTEMID);
	printf("\tq[uit] new-code\n");
	printf("\t!{shell command}\n");
	printf("\tc[d] directory\n");
	printf("\ts[top]\n\n");
}

/*******************************************************************/

#ifdef	SPRINTF
#define		NUMBERS
#define		LONG
#include	<sprintf.h>
#endif	SPRINTF

/*
**	check for appropriate permissions on a file
**	passed as 'arg' and creat it if allowable.
*/

accreat(arg, flag)
char *arg;
{
	register char *p;
	int n;
	extern int errno;

	if ((n = access(arg, W)) == 0)		/* file already exists and have write permission */
	{
		if (flag == 0)
		{
			printf(exists, arg);
			if (getline() != 'y') return -1;
		}
		else if (flag == 1)
		{
			write(2, MDLE);
			return -1;
		}
	}
	else if (n && errno == ENOENT)
	{				/* isolate directory name and check it */
		p = arg;
		while (*++p);
		while (*--p != '/' && p > arg);
		if (*p != '/') n = access("", W|X);
		else if (p == arg) n = access("/", W|X);
		else
		{
			*p = 0;
			n = access(arg, W|X);
			*p = '/';
		}
	}
	if (n == 0)
	{			/* creat the file since access is granted */
		n = creat(arg, 0600);
		chown(arg, uid);
	}
	if (n < 0)
		perror(arg);
	return n;
}

/*
 *	validate that log is not being used with any
 *	standard input or output redirection - this
 *	includes asynchronous use.
 *	The test checks that file descriptors 0, 1, and 2 represent
 *	the same tty.
 */

okuse()
{
	struct
	{
		int majmin;	/* major minor device number for i-node */
		int inum;	/* inode number on device */
		int flags;	/* flags from inode */
#define	IFMT	060000
#define	ICHR	020000
		char dumy[30];	/* the rest ... */
	}
	sbuf[3];
	register i;

	for (i = 0; i < 3; i++)
		if (fstat(i, &sbuf[i]) == -1 ||		/* stat failed ??? */
		    sbuf[i].flags&IFMT != ICHR ||	/* not a tty */
		    i &&
		    (sbuf[i].majmin != sbuf[i-1].majmin ||
			sbuf[i].inum != sbuf[i-1].inum))	/* some sort of redirection */
			return 0;
	return 1;
}
