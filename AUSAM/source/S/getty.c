#
/*
 * getty -- adapt to terminal speed on dialup, and call login
 */

#include	<local-system>

/*
 *	Modified 10/77 Piers Lauder
 *	changes:
 *		generally tidied up
 *		new terminal types
 */

#define	AUSAM
	/*
	 *	piers lauder 	oct '77
	 *
	 *	"login" incorporated into "getty"
	 *	( users must now type CTRL-d at end of session
	 *	  to allow "init" to collect statistics )
	 *
	 *	Piers Lauder	Feb '78 - May '78
	 *
	 *	the whole AUSAM works ...
	 */

#define	DISK_LIMIT
	/*
	 *	Piers Lauder	Apr '78
	 *
	 *	if disk llimits to be implemented
	 */

#define	LOGIN_TIME
	/*
	 *	Piers Lauder	Apr '78
	 *
	 *	Print out time and date on logon.
	 *	This is mentioned in the manual, so if you want it ...
	 */

#define	T_STOPS		0200
	/*
	**	Piers Lauder	Jul '78
	**
	**	Set tab stops for terminals that need programming
	*/

#define INITIALISE
#define INITFILE ".profile"
#define	RX	05 	/* read and execute */
	/*
	**	Greg Rose 24 July '78
	**
	**	Automatically run an initialisation shell file after logging
	**	in. The name of the file is ".profile" a la PWB-Unix.
	*/

#ifdef	AUSAM
#define	TTY_GROUP
#include	<ttygroup.h>
	/*
	**	Ian Johnstone	Sep '78
	**
	**	Include code for the support of terminal groups
	**	and terminal masks.  A terminal group table from
	**	<local-system> is searched with the third argument
	**	passed to getty - this generates a bit-mask which
	**	is &'ed with the 'pw_tmask' field from the passwd
	**	entry for the user.  If result is non-zero
	**	then a login will be allowed.  This allows good
	**	control over which users can use which terminals.
	**	Will only work if AUSAM defined
	*/
#endif	AUSAM


/*
 * terminals
 */

#ifdef	AGSM
#define	LOG		'#'
#define	TEKTRONICS	'g'
#define	LA36		'd'
#define	DIALUP		'u'
#define	LS120		'D'
#define	STRANSDATA	'T'
#define	TRANSDATA	't'
#define	VT52_5		'V'
#define	ELECTRIC	'S'
#define	QUME		'q'
#define	QUME5		'Q'
#define	TELERAY		'W'
#define	COPS10		'8'	/* at 19200 baud */
#define	LOG1200		'B'	/* specially for basser */
#endif	AGSM

#ifdef	BASSER
#define	TEKTRONICS	'T'
#define	LA36		'l'
#define	CDC713		'c'
#define	DIABLO		('d'|T_STOPS)
#define	SDIABLO		('D'|T_STOPS)
#define	IMLAC		'i'
#define	ASR33		'-'
#define	NCR260		'N'
#define	ADM1_A		'a'
#endif	BASSER

#ifdef	EECF
#define	LOG		'#'
#define	FASTLOG		'4'
#define	VT05		'v'
#define	TEK4014		'T'
#define	LA36		'd'
#define	ELECTRIC	'S'
#define	QUME5		'Q'
#define	LS120		'D'
#define	TELERAY		'W'
#endif	EECF

/*
 * Unused terminals types
 *
#define	DIAL_IN		'0'
#define	IBM2741		'1'
 */

/*
 * tty flags
 */
#define	HUPCL 01
#define	XTABS	02
#define	LCASE	04
#define	ECHO	010
#define	CRMOD	020
#define	RAW	040
#define	ODDP	0100
#define	EVENP	0200
#define	ANYP	0300
#define	INVCASE	04000
#define FLOWCNTRL 0100000

/*
 * Delay algorithms
 */
#define	NL1	000400
#define	NL2	001000
#define	NL3	001400
#define	TAB1	002000
#ifndef	INVCASE
#define	TAB2	004000
#define	TAB3	006000
#endif
#define	CR1	010000
#define	CR2	020000
#define	CR3	030000
#define	FF1	040000
#ifndef	FLOWCNTRL
#define	BS1	0100000
#endif

#ifdef	BASSER
#define	ERASE	'\b'
#else
#define	ERASE	'#'
#endif 	BASSER
#define	KILL	'@'

/*
 * speeds
 */
#define	B110	3
#define B134_5	4
#define	B150	5
#define	B300	7
#define	B600	8
#define B1200	9
#define B2400	11
#define B4800	12
#define	B9600	13
#define	EXTB	15

#define	SIGINT	2
#define	SIGQIT	3

struct	sgtty {
	char	sgispd, sgospd;
	char	sgerase, sgkill;
	int	sgflag;
} tmode;

char	nlbel[]	"\r\n\007";

struct	tab {
	char	tname;		/* this table name */
	char	nname;		/* successor table name */
	int	iflags;		/* initial flags */
	int	fflags;		/* final flags */
	char	ispeed;		/* input speed */
	char	ospeed;		/* output speed */
	char	*message;	/* login message */
} itab[] {

#ifdef	DIAL_IN
/* table DIAL_IN-1-2 300,150,110 */

  {
	DIAL_IN, 1,
	EVENP+RAW+NL1+CR1+HUPCL, EVENP+CR1+XTABS+HUPCL,
	B300, B300,
	"\n\r\033;\007"
  },

  {
	1, 2,
	EVENP+RAW+NL1+CR1, EVENP+ECHO+FF1+CR2+TAB1+NL1,
	B150, B150,
	"\n\r\033:\006\006\017"
  },

  {
	2, DIAL_IN,
	ANYP+RAW+NL1+CR1, ANYP+ECHO+XTABS+CR1,
	B110, B110,
	nlbel
  },
#endif

#ifdef	ASR33
/* table ASR33 -- ASR33 110 */
  {
	ASR33, ASR33,
	ANYP+RAW+NL1+CR1, ANYP+ECHO+CRMOD+XTABS+CR1+LCASE,
	B110, B110,
	nlbel
  },
#endif

#ifdef	IBM2741
/* table IBM2741 -- 134_5 */
  {
	IBM2741, IBM2741,
	EVENP+RAW+NL1, EVENP+NL1,
	B134_5, B134_5,
	"\n"
  },
#endif

#ifdef	UFO
/* table '9' -- 9600 */
  {
	'9', '9',
	ANYP+RAW+NL1+CR1, ANYP+XTABS+ECHO+CRMOD+FF1,
	B9600, B9600,
	"\n\r\033;"
  },
#endif

#ifdef	VT05
/* table VT05 -- vt05 */
  {
	VT05,VT05,
	ANYP+RAW+NL2, ANYP+NL2+ECHO+CRMOD,
	B2400, B2400,
	"\035\035\035\035\037\037\037\037"
  },
#endif

#ifdef	LOG
/* table LOG -- LOG device twixt here and remote UNIX sites .... */
  {
	LOG, LOG,
	ANYP+RAW, ANYP+ECHO+XTABS+CRMOD,
	B4800, B4800,
	nlbel
  },
#endif

#ifdef	FASTLOG
/* table FASTLOG -- FASTLOG device twixt here and our 11/40 .... */
  {
	FASTLOG, FASTLOG,
	ANYP+RAW, ANYP+ECHO+XTABS+CRMOD,
	B9600, B9600,
	nlbel
  },
#endif

#ifdef	LA36
/* table LA36 -- LA36 */
  {
	LA36, LA36,
	ANYP+RAW, ANYP+XTABS+ECHO+CRMOD,
	B300, B300,
	nlbel
  },
#endif

#ifdef	DIALUP
/* table DIALUP -- DIALUP */
  {
	DIALUP, DIALUP,
	ANYP+RAW+HUPCL, ANYP+XTABS+ECHO+CRMOD+HUPCL,
	B300, B300,
	nlbel
  },
#endif

#ifdef	LS120
/* table LS120 -- LS120 */
  {
	LS120, LS120,
	ANYP+RAW, ANYP+NL1+CR1+XTABS+ECHO+CRMOD,
	B1200, B1200,
	nlbel
  },
#endif

#ifdef	LOG1200
/* table LOG1200 -- Basser leased line login */
  {
	LOG1200, LOG1200,
	ANYP+RAW, ANYP+XTABS+ECHO+CRMOD,
	B1200, B1200,
	nlbel
  },
#endif

#ifdef	VT52_5
/* table VT52_5 -- vt52/5 */
  {
	VT52_5, VT52_5,
	ANYP+RAW, ANYP+ECHO+CRMOD,
	B4800, B4800,
	nlbel
  },
#endif

#ifdef	COPS10
/* table COPS10 -- for COPS10 at 19200 */
  {
	COPS10, COPS10,
	ANYP+RAW, ANYP+ECHO+CRMOD+XTABS,
	EXTB, EXTB,
	nlbel
  },
#endif

#ifdef	TELERAY
/* table TELERAY -- for TELERAY at 4800 */
  {
	TELERAY, TELERAY,
	ANYP+RAW, ANYP+ECHO+CRMOD+XTABS,
	B4800, B4800,
	nlbel
  },
#endif

#ifdef	ELECTRIC
/* table ELECTRIC -- for ELECTRIC at 2400 */
  {
	ELECTRIC, ELECTRIC,
	ANYP+RAW, ANYP+ECHO+CRMOD+XTABS,
	B2400, B2400,
	"\035\037",
  },
#endif

#ifdef	DIABLO
/* table DIABLO-'D' -- Diablo Hyterm at 1200-300 baud */
  {
	DIABLO, SDIABLO,
	EVENP+RAW+FLOWCNTRL, EVENP+FLOWCNTRL+CRMOD,
	B1200, B1200,
	nlbel
  },

  {
	SDIABLO, DIABLO,
	EVENP+RAW, EVENP+CRMOD,
	B300, B300,
	nlbel
  },
#endif

#ifdef	IMLAC
/* table IMLAC-9-10 -- IMLAC 9600-4800-300 baud */
  {
	IMLAC, 9,
	ANYP+RAW, ANYP+XTABS,
	B9600, B9600,
	nlbel
  },

  {
	9, 10,
	ANYP+RAW, ANYP+XTABS,
	B4800, B4800,
	nlbel
  },

  {
	10, IMLAC,
	ANYP+RAW, ANYP+XTABS,
	B300, B300,
	nlbel
  },
#endif

#ifdef	NCR260
/* table NCR260 -- NCR260 needs inverted case and special delays */
  {
	NCR260, NCR260,
	EVENP+RAW+NL2+CR3+INVCASE, EVENP+XTABS+ECHO+CRMOD+NL2+CR3+INVCASE,
	B300, B300,
	nlbel
  },
#endif	NCR260

#ifdef	ADM1_A
/* table ADM1_A -- ADM1_A needs special delays */
  {
	ADM1_A, ADM1_A,
	EVENP+RAW+NL1, EVENP+XTABS+ECHO+CRMOD+NL1,
	B9600, B9600,
	"\033*\007"
  },
#endif	MEMOREX

#ifdef	CDC713
/* table CDC713 -- CDC713 */
  {
	CDC713, CDC713,
	EVENP+RAW, EVENP+XTABS+ECHO+CRMOD,
	B300, B300,
	nlbel
  },
#endif

#ifdef	TEKTRONICS
/* table TEKTRONICS -- tektronics at 4800bd */
  {
	TEKTRONICS, TEKTRONICS,
	ANYP+RAW+FF1, ANYP+CRMOD+XTABS+FF1+LCASE+ECHO,
	B4800, B4800,
	"\033\014\007"
  },
#endif

#ifdef	TEK4014
/* table TEK4014 -- tektronics at 1200bd */
  {
	TEK4014, TEK4014,
	ANYP+RAW+FF1, ANYP+CRMOD+XTABS+FF1+ECHO,
	B1200, B1200,
	"\033;\033\014\007"
  },
#endif

#ifdef	QUME
/* table QUME -- qume sprint printer at 300bd */
  {
	QUME, QUME,
	ANYP+RAW, ANYP+ECHO+CRMOD+XTABS,
	B600, B600,
	nlbel
  },
#endif

#ifdef	QUME5
/* table QUME5 -- qume sprint printer at 1200bd */
  {
	QUME5, QUME5,
	ANYP+RAW, ANYP+ECHO+CRMOD,
	B1200, B1200,
	"\r\033\037\013\0332\033(08,16,24,32,40,48,56,64,72,80,88,96,A4,B2,C0,C8,D6,E4,F2.\n\007",
		/* set to 12-pitch; clear tabs; set tabs every 8 */
  },
#endif

#ifdef	TRANSDATA
/* table TRANSDATA-'z' -- transdata at 300-2400bd */
  {
	STRANSDATA, TRANSDATA,
	EVENP+RAW, EVENP+ECHO+CRMOD+XTABS,
	B300, B300,
	nlbel
  },
  {
	TRANSDATA, STRANSDATA,
	EVENP+RAW, EVENP+ECHO+CRMOD+XTABS,
	B2400, B2400,
	nlbel
  },
#endif
};

#define	NITAB	sizeof itab/sizeof itab[0]

#define	NAMZ	133
char	name[NAMZ];
int	crmod;
int	upper;
int	lower;

#ifdef	AUSAM
char	ttyx[]	"/dev/ttyx";

#ifdef	TTY_GROUP
unsigned ttygmask;	/* store tty group mask for current terminal here */
#endif	TTY_GROUP
#endif	TTY_GROUP

main(argc, argv)
char **argv;
{
	register struct tab *tabp;
	register tname;
	register i;

#ifdef	AUSAM
#ifdef	TTY_GROUP
	if( argc > 3 )
	{
		ttyx[8] = *argv[2];
		ttygmask = 0177777; /* assume all unless find matching group */
		for( i = 0; i < NTTYGRPS; i++ )
			if( *argv[3] == tty_groups[i].tchar )
			{
				ttygmask = tty_groups[i].tmask;
				break;
			}
	}
#else
	if ( argc > 2 )
	{
		ttyx[8] = *argv[2];
	}
#endif	TTY_GROUP
	else
	{
		printf( "Getty: wrong number of args !!\n" );
		exit(1);	/* init will not call getty wrongly */
	}
#else	AUSAM
	tname = '0';
	if (argc > 1)
#endif	AUSAM
		tname = *argv[1];

	for (;;)
	{
		nice(-5);	/* give getty a little help */
		for(tabp = itab; tabp < &itab[NITAB]; tabp++)
#			ifndef	T_STOPS
			if(tabp->tname == tname)
#			else
			if ( (tabp->tname & 0177) == tname )
#			endif	T_STOPS
				break;

		if(tabp >= &itab[NITAB])
			tabp = itab;

		tmode.sgispd = tabp->ispeed;
		tmode.sgospd = tabp->ospeed;
		tmode.sgflag = tabp->iflags;
		tmode.sgerase = 0;
		tmode.sgkill = 0;
		stty(0, &tmode);

		printf( "%s%s login: ", tabp->message, SYSTEMID);

		if(getname())
		{
			tmode.sgflag = tabp->fflags;
			if(crmod)
				tmode.sgflag =| CRMOD;
			if(upper)
				tmode.sgflag =| LCASE;
			if(lower)
				tmode.sgflag =& ~LCASE;
#ifdef	BASSER
			tmode.sgerase = tmode.sgflag&LCASE ? '#' : ERASE;
#else
			tmode.sgerase = ERASE;
#endif	BASSER
			tmode.sgkill = KILL;
			stty(0, &tmode);
#			ifdef	T_STOPS
			if ( tabp->tname & T_STOPS )	/* set tab stops */
			{
				printf( "\0332\033\t\001\0339\033B" );
				for ( i = 9 ; i < 132 ; i =+ 8 )
					printf( "\033\t%c\0331", i );
			}
#			endif	T_STOPS

#ifndef	AUSAM
			execl("/bin/login", "login", name, 0);
			exit(1);
#else
			login();
#endif	AUSAM
		}
		sleep( 1 );
		tname = tabp->nname;
	}
}

getname()
{
	register char *np;
	register c;
	static cs;

	crmod = 0;
	upper = 0;
	lower = 0;
	np = name;
	do
	{
		while (read(0, &cs, 1) <= 0)
			sleep(1);
		if ((c = cs&0177) == 0)
			return(0);
		putchar( c );
		if (c>='a' && c <='z')
			lower++;
		else if (c>='A' && c<='Z')
		{
			upper++;
			c =+ 'a'-'A';
		}
		else
		if (c==ERASE)
		{
			if (np > name)
				np--;
			continue;
		}
		else
		if (c==KILL)
		{
			np = name;
			continue;
		}
		*np++ = c;
	} while (c!='\n' && c!='\r' && np <= &name[NAMZ]);
	*--np = 0;
	if ( np == name )
		return( 0 );
	if (c == '\r')
	{
		putchar( '\n' );
		crmod++;
	}
	else
		putchar( c );
	return(1);
}




#ifdef	AUSAM

#ifdef	DISK_LIMIT
/* choose TDU algorithm */
#define SHARELINKS	/* links shared equally -> recursive directory search */
/*#define OWNER		/* only owner charged for links -> inode search */
#endif	DISK_LIMIT

#include	<param.h>
#include	<stat16.h>
struct 	statbuf	statb;
#include	<passwd.h>
#include	<lnode.h>
#include	<utmp.h>
struct	utmp	utmp;


#ifdef	DISK_LIMIT
#ifdef	SHARELINKS
long		tdu(), malformed();
char		*nextpath();
long		t_blocks, t_files;
char		pathname[200];
#define	PATHEND	(&pathname[(sizeof pathname)])
#endif	SHARELINKS
#ifdef	OWNER
unsigned	tdu();
unsigned	t_blocks, t_files;
#endif

unsigned	size();
int		derror;
#endif	DISK_LIMIT

long		time();

#include	"decays.h"

struct { int hiword; unsigned loword; };
struct { unsigned u; };

#define	ETOOMANYU	37




login()
{
	register char	*np;
	register	i;
	long		ttime;
	struct	lnode	llimits;
	struct	pwent	pe;
#	ifdef	SHARELINKS
	unsigned	parino;
#	endif	SHARELINKS
	extern		end, errno;

	/*
	**	extract user particulars from password file
	*/
	pe.pw_strings[LNAME] = name;
	{
		register	t = SSIZ;
		char		*pbuf = &end;

		for ( ;; )
		{
			if ( sbrk( SSIZ ) == -1 )
			{
				perror( "" );
				goto out;
			}
			if ( (i = getpwuid( &pe , pbuf , t )) < 0 )
			{
				printf( "Who?\n" );
				goto out;
			}
			if ( i < t )
				break;
			t =+ SSIZ;
		}
	}

	/*
	**	if password is non-null, verify it
	*/
	if ( *(np = pe.pw_pword) )
	{
		register char	*namep;
		int		sflags;
		char		pwbuf[PASSWDZ];

		sflags = tmode.sgflag;
		if ( sflags & ECHO )
		{
			tmode.sgflag =& ~ ECHO;
			stty(0, &tmode);
		}
		printf( "Password: " );
		namep = pwbuf;
		while ((i=getchar()) != '\n')
		{
			if (i <= 0)
				goto out;
			if (namep < &pwbuf[PASSWDZ-1])
				*namep++ = i;
		}
		*namep++ = '\0';
		if ( sflags & ECHO )
		{
			tmode.sgflag = sflags;
			stty(0, &tmode);
		}
		putchar( '\n' );
		namep = crypt(pwbuf);
		for ( i=0 ; i<8 ; i++ )
			if ( *namep++ != *np++ )
			{
				printf( "Wrong password.\n" );
				goto out;
			}
	}

#	ifdef	TTY_GROUP
	if( pe.pw_uid && !(pe.pw_tmask&ttygmask) )
	{
		if( pe.pw_tmask )
			printf("You are not authorized to use this terminal\n");
		else
			printf("Consult the system supervisor\n");
		sleep(2);
		goto out;
	};

#	endif	TTY_GROUP

#	ifdef	SHARELINKS
	/*
	**	get inode of parent directory of DIRPATH (N.B. ".." is not good enough)
	*/
	{
		register char	*dp;
		char		dbuf[SSIZ];

		if ( (np = pe.pw_strings[DIRPATH]) == 0 || *np == 0 )
			np = "/";

		for ( dp = dbuf; *dp++ = *np++; )
			if ( dp >= &dbuf[SSIZ] )
			{
				printf( "%s: too long!\n", pe.pw_strings[DIRPATH] );
				goto out;
			}

		while ( *--dp != '/' )
			if ( dp <= dbuf )
			{
				printf( "Bad directory path: %s\n", pe.pw_strings[DIRPATH] );
				goto out;
			}

		if ( dp == dbuf )	/* root! */
			dp++;
		*dp = 0;

		if ( newstat( dbuf, &statb ) < 0 )
		{
			printf( "Parent directory: " );
			perror( dbuf );
			goto out;
		}

		parino = statb.sb_inumber;
	}
#	endif	SHARELINKS

	/*
	**	move over to the user's home directory
	*/
	if ( newstat( pe.pw_strings[DIRPATH] , &statb ) < 0 || chdir(pe.pw_strings[DIRPATH]) < 0 )
	{
		perror( pe.pw_strings[DIRPATH] );
		goto out;
	}

	/*
	**	We are now committed to the user, and subsequent errors are fatal
	*/

	/*
	**	copy the message-of-the-day to keep the user guessing...
	*/
	if ((i = open("/etc/motd", 0)) >= 0)
	{
		char	mbuf[512];

		while( write(1, mbuf, read( i, mbuf, 512)) == 512);
		close(i);
	}

	ttime = time();

	/*
	**	get limits from system, if they already exist
	*/
	llimits.l_uid = pe.pw_uid;
	if ( limits( &llimits, L_OTHLIM ) >= 0 )
	{
		printf( "You already have %d process%srunning\n", llimits.l_refcount, llimits.l_refcount == 1 ? " " : "es " );
		pe.pw_flags = llimits.l_flags;
	}
	else
	{
		/*
		**	limits don't exist, so set them up
		*/

#ifdef	DISK_LIMIT
		if ( pe.pw_dlimit && pe.pw_uid )
		{
			register	t;

			derror = 0;
			errno = 0;
#ifdef	SHARELINKS
			t_blocks = 0;
			t_files = 0;
			i = tdu( nextpath( pathname, pe.pw_strings[DIRPATH] ), parino ) >> 16;	/* N.B. statb set up above */
#endif
#ifdef	OWNER
			i = tdu( pe.pw_uid );	/* N.B. statb set up above */
#endif
			if ( i.u > pe.pw_dlimit || derror )
			{
				if ( !derror )
				{
					printf( "\n\t%3.3s WARNING - disc limit of %l blocks exceeded by %l\n"
						, &"1st2nd3rd   "[pe.pw_warn*3], pe.pw_dlimit, i-pe.pw_dlimit
					      );
					printf( "\t(total of %l units consists of %l blocks in %l files)\n"
						,i.u
#ifdef	OWNER
						,t_blocks
						,t_files
#endif
#ifdef	SHARELINKS
						,t_blocks.hiword
						,t_files.hiword
#endif
					      );
				}

				if ( ++pe.pw_warn > NDLIMWARN || derror || i.u > (pe.pw_doverflw+pe.pw_dlimit) )
				{
					printf( "\t-- no more disc space allowed\n" );
					pe.pw_warn = NDLIMWARN;
					pe.pw_flags =| DLIMIT;
				}
				else
				{
					t = NDLIMWARN - pe.pw_warn;
					printf( "\tyou have %d warning%sleft\n"
						,t
						,t==1?" ":"s "
					      );
					pe.pw_flags =& ~DLIMIT;
				}
				putchar( '\n' );
			}
			else
			{
				pe.pw_warn = 0;
				pe.pw_flags =& ~DLIMIT;
			}
		}
		else
		{
			i = 0;
			pe.pw_warn = 0;
			pe.pw_flags =& ~DLIMIT;
		}
		llimits.l_duse = i;
		llimits.l_doverflw = pe.pw_doverflw;
#endif	DISK_LIMIT

		llimits.l_refcount = 0;
		llimits.l_shares = pe.pw_shares;
		for ( i=0 ; i<CMASKSIZE ; i++ )  llimits.l_cmask[i] = pe.pw_cmask[i];
		llimits.l_dlimit = pe.pw_dlimit;
		llimits.l_plimit = pe.pw_plimit;
		llimits.l_climit = pe.pw_climit;
		llimits.l_flags = pe.pw_flags;
		/*
		**	decay usage by time since last access
		**	YES, it could be done better with
		**	floating point but the approx used
		**	here is pretty good and more reliable
		**	than dec FPU.
		*/
		if ( (i = (ttime - pe.pw_extime)/(60*60) -1) >= 0 )
		{
			register unsigned	u;
			u = decays[i >= 240 ? 238 : i ];

			if ( pe.pw_usage > (8 * u) )
				llimits.l_usage = pe.pw_usage - ((pe.pw_usage / u) * i * DECAYMULT);
			else
				llimits.l_usage = pe.pw_usage - ((pe.pw_usage * i * DECAYMULT) / u);
			if ( llimits.l_usage < 0 )
				llimits.l_usage = 0;
		}
		else
			llimits.l_usage = pe.pw_usage;
	}

	/*
	**	tell limits
	*/
	if ( pe.pw_uid && pe.pw_dlimit )
		printf( "\nYou are using %l out of %l disk units\n",
			llimits.l_duse, pe.pw_dlimit );

	/*
	**	update password file
	*/
	updtpwent( &pe );

	/*
	**	make login entries in utmp & wtmp files
	*/
	{
		register char	*namep;

		for ( i=(sizeof utmp.u_u_name)+1 , np=utmp.u_u_name , namep=name ; --i && (*np++ = *namep++) ; );
		if ( !i && *namep )	/* name too big for field */
			utmp.u_u_name[0] = '\0';
	}

	utmp.u_ttyid = ttyx[8];
	utmp.u_u_id = pe.pw_uid;
	utmp.u_type = U_TYPE;
	utmp.u_logintime = ttime;

	{
		register	f;

		if ((f = open("/etc/utmp", 1)) >= 0)
		{
			i = utmp.u_ttyid;
			if ( i >= '0' && i <= '9' )  i =- '0';
			else	if ( i >= 'a' && i <= 'z' )  i =- 'a' - 10;
				else   i =- 'A' - 36;
			seek(f, i*(sizeof utmp), 0);
			write(f, &utmp, (sizeof utmp));
			close(f);
		}

		/*
		**	N.B. wtmp may be AUTOlocked
		*/
		if ((f = open("/usr/adm/wtmp", 1)) >= 0)
		{
			seek(f, 0, 2);
			write(f, &utmp, (sizeof utmp));
			close(f);
		}
	}

#ifdef	LOGIN_TIME
	/*
	**	print date
	*/
	printf( ctime( ttime ) );
#endif

	/*
	**	tell if any mail is waiting
	*/
	if(newstat(".mail", &statb) >= 0 && statb.sb_size1)
		printf( "You have mail.\n" );

	/*
	**	give user control of tty
	*/
	chown(ttyx, pe.pw_uid);
	chmod( ttyx, 0602 );

	/*
	**	now become the user (and lose root priviledges)
	*/
	if ( llimits.l_uid )
	{
		if ( limits( &llimits , L_SETLIM ) < 0 )
		{
			perror( pe.pw_strings[LNAME] );
			sleep( 10 );
			setuid( llimits.l_uid );	/* for wtmp consistency */
			exit( -1 );
		}
		setuid( llimits.l_uid );
	}

	for ( i=3 ; i<15 ; i++ )
		close( i );

	nice(0);
#ifdef INITIALISE
	/*
	**	execute the initialisaion file.
	*/
	{
		int	pid, r, ret;
		if(access(INITFILE,RX) == 0)
		{
			if((pid = fork()) < 0)
			{
				/* parent - error */
				perror("Can't initialise");
			}
			else if(pid == 0)
			{
				/* child */
				execl("/bin/sh","sh",INITFILE,0);
				perror("Can't exec shell");
				exit(-1);
			}
			else
			{
				/* parent */
				while((r = waitx(&ret)) > 0 && r != pid);
				if( ret )
					printf("Initialisation failed\n");
			}
		}
	}

#endif INITIALISE
	/*
	**	become user's shell
	*/
	if ( (np = pe.pw_strings[SHELLPATH]) == 0 || *np == '\0' )
		np = "/bin/sh";
	execl(np, "-", 0);
	printf( "No Shell! " );
	perror( np );
	exit( -1 );

out:
	brk( &end );
}



#ifdef	SHARELINKS

#define	ERROR	0XFFFFFFFFL

struct
{
	int f_ino;
	char f_name[14];
	char f_end;
}
	dirbuf;

/*
 *	"tdu" returns the disc usage of the current directory
 *	      and all sub-directories.
 *		Inodes are weighted at IWEIGHT blocks.
 *		Links are shared equally.
 */

long tdu( pathend, parino )
  char *pathend;
  unsigned parino;
{
	register	fd, i, posn;
	long		inodes, blocks, t;
	unsigned	dino;

  if ( pathend == 0 )
	return( ERROR );

  dino = statb.sb_inumber;

  inodes = 0X10000;
  t_files =+ 0X10000;
  blocks.loword = 0;
  blocks.hiword = size();
  t_blocks =+ blocks;

  posn = 0;
  dirbuf.f_end = '\0';

loop:
  if ( ((fd = open( pathname , 0)) >= 0) && (seek( fd , posn , 0) >= 0) )
  {
	while ( (i = read( fd , &dirbuf , 16 )) == 16 )
	{
		posn =+ i;

		if ( dirbuf.f_ino )
			if ( newstat( dirbuf.f_name , &statb ) >= 0 )
			{
				if ( (statb.sb_flags & IFTYP) == IFDIR )
				{
					if ( statb.sb_inumber != dino && statb.sb_inumber != parino )
					{
						if ( dots() )
							return( malformed( "separate directory!" ) );

						if ( posn <= 32 )
							return( malformed( "\".\" or \"..\" displaced!" ) );

						if ( chdir( dirbuf.f_name ) >= 0 )
						{
							close( fd );
							blocks =+ tdu( nextpath( pathend, dirbuf.f_name ), dino );
							*pathend = 0;
							chdir( pathname );
							if ( derror )
								return( ERROR );
							goto loop;
						}
						else
							return( malformed() );
					}
					else
						if ( posn > 32 )
							return( malformed( "link displaced" ) );
						else
							if ( !dots() )
								return( malformed( "\".\" or \"..\" renamed!" ) );
				}
				else
				{
					/* fixed point arithmetic */
					t = (0X10000 / (statb.sb_nlinks&0377));
					t_files =+ t;
					inodes =+ t;
					t = ((0X10000 * size()) / (statb.sb_nlinks&0377));
					t_blocks =+ t;
					blocks =+ t;
				}
			}
			else
				return( malformed() );
	}

	close( fd );

	if ( i )
		return( malformed( "bad length" ) );

	return( blocks + inodes*IWEIGHT );
  }
  else
  {
	dirbuf.f_name[0] = 0;
	return( malformed() );
  }
}


unsigned size()
{
	register unsigned b, ib;

  b = ((statb.sb_size0 & 0377) << 7) + (((statb.sb_size1 + 0777) >> 9) & 0177);
  if ( ib = b>>3 )
  {
	b++;
	if ( ib =>> 5 )
	{
		b =+ ib;	/* indirect */
		if ( ib > 8 )
			b++;	/* double indirect */
	}
  }

  return( b );
}


long malformed( s )
  char *s;
{
	extern	errno;

  putchar( '\n' );
  printf( "MALFORMED DIRECTORY in %s\n\t", pathname );
  if ( errno )
	perror( dirbuf.f_name );
  else
	printf( "%s: %s\n", dirbuf.f_name, s );
  derror++;
  return( ERROR );
}


dots()
{
	register char	*sp = dirbuf.f_name;

	if ( *sp++ == '.'
	     && ( *sp == '\0'
		  || ( *sp++ == '.' && *sp == '\0' )
		)
	   )
		return( 1 );

	return( 0 );
}


char *nextpath( at, name )
  register char *at, *name;
{
  if ( *name != '/' )
	*at++ = '/';

  do
	if ( at >= PATHEND )
	{
		derror++;
		return( 0 );
	}
  while
	( *at++ = *name++ );

  return( --at );
}
#endif	SHARELINKS



#ifdef	OWNER
#include	<ino.h>
#include	<filsys.h>
struct	def_fs
{
	char		fs_bmajor;	/* major device number for block special file */
	char		*fs_rname;	/* raw special file */
}
	defv[]
{
  {
	'\003', "/dev/rms0"  }
};
#define	NDEF	((sizeof defv)/(sizeof defv[0]))

#define	NINODES	512			/* read 4K of inodes at a time */
#define	BUFSIZ	(NINODES*INODEZ)
#define	ERROR	0177777			/* return maximum usage on error */

/*
 *	"tdu" returns the total disc usage for "uid" on the current file system .
 *		Inodes are weighted at IWEIGHT blocks.
 *		Links are ignored.
 *		Errors return maximum usage.
 */
unsigned tdu( uid )
  unsigned uid;
{
	register char *buf;
	register union
	{
		char *cp; int i; } r;
	register union
	{
		struct def_fs	*dp;
		struct filsys	*fp;
		struct inode	*ip;
	} p;
	unsigned  ninodes, blocks, inodes;
	char  *sbrk();
	int	f;
	unsigned size();

  if ( !uid )  return( 0 );	/* don't bother with root */

  for ( p.dp = defv ; p.dp < &defv[NDEF] ; p.dp++ )
	if ( p.dp->fs_bmajor == statb.sb_major )
		goto found;
  printf( "Unknown file system! - major device = 0%o\n", statb.sb_major );
  derror++;
  return( ERROR );

found:
  for ( r.cp = p.dp->fs_rname ; *r.cp++ ; );
  r.cp[-2] =+ statb.sb_minor;
  if ( (f = open( p.dp->fs_rname , 0 )) < 0 )
  {
	perror( p.dp->fs_rname );
	derror++;
	return( ERROR );
  }

  if ( (buf = p.fp = sbrk( BUFSIZ )) == -1 )
  {
	perror( "" );
	close( f );
	derror++;
	return( ERROR );
  }

  seek( f , 512 , 0 );
  read( f , buf , 512 );	/* super block */
  ninodes = p.fp->s_isize*(512/INODEZ);

  inodes = 0;
  blocks = 0;
  for ( r.i = 0 ; r.i < ninodes ; )
  {
	read( f , buf , BUFSIZ );
	for ( p.ip = buf ; (r.i < ninodes) && (p.ip < &buf[BUFSIZ]) ; r.i++ , p.ip++ )
		if ( ((p.ip->i_uidhib<<8)|(p.ip->i_uidlob&0377)) == uid )
		{
			inodes++;
			blocks =+ size( p.ip );
		}
  }

  brk( buf );
  close( f );
  t_blocks = blocks;
  t_files = inodes;
  return( blocks + inodes*IWEIGHT );
}

unsigned size( ip )
  register struct inode *ip;
{
	register unsigned b, ib;

  b = ((ip->i_size0 & 0377) << 7) | (((ip->i_size1 + 0777) >> 9) & 0177);
  if ( ib = b>>3 )
  {
	b++;
	if ( ib =>> 5 )
	{
		b =+ ib;	/* indirect */
		if ( ib > 8 )
			b++;	/* double indirect */
	}
  }

  return( b );
}
#endif	OWNER


#endif	AUSAM
