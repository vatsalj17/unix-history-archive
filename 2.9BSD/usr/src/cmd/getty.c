#
/*
 * @(#)getty.c	1.5 -- adapt to terminal speed on dialup, and call login
 */
#include <whoami.h>
#include <sgtty.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/file.h>

#define IDENT	"/etc/ident"	/* contains login banner for sys */

#define SP_SESS			/* "special session"-- root logins only */
#define DO_CRTBS	/* allow backspace as erase; stty erase if >2400 baud */

#define ERASE	'#'
#define KILL	'@'
#define CNTL(x)	('x'&037)
#define DEL	'\177'
#define UNDEF	'\377'

struct sgttyb tmode;
struct tchars tchars = 
	{ DEL, CNTL(\\), CNTL(Q), CNTL(S), CNTL(D), UNDEF
};
#ifdef UCB_NTTY
struct	ltchars ltc =
	{ CNTL(z), CNTL(y), CNTL(r), CNTL(o), CNTL(w), CNTL(v)
};
#endif

struct	tab {
	char	tname;		/* this table name */
	char	nname;		/* successor table name */
	int	iflags;		/* initial flags */
	int	fflags;		/* final flags */
	int	ispeed;		/* input speed */
	int	ospeed;		/* output speed */
	char	*message;	/* login message */
} itab[] = {

/* table '0'-1-2-3 300,1200,150,110 */

	'0', 1,
	ANYP+RAW+NL1+CR1, ANYP+ECHO+CR1,
	B300, B300,
	"\n\r\033;\007login: ",

	1, 2,
	ANYP+RAW+NL1+CR1, ANYP+XTABS+ECHO+CRMOD+FF1,
	B1200, B1200,
	"\n\r\033;login: ",

	2, 3,
	ANYP+RAW+NL1+CR1, EVENP+ECHO+FF1+CR2+TAB1+NL1,
	B150, B150,
	"\n\r\033:\006\006\017login: ",

	3, '0',
	ANYP+RAW+NL1+CR1, ANYP+ECHO+CRMOD+XTABS+LCASE+CR1,
	B110, B110,
	"\n\r:login: ",

/* table '-' -- Console TTY 110 */
	'-', '-',
	ANYP+RAW+NL1+CR1, ANYP+ECHO+CRMOD+XTABS+LCASE+CR1,
	B110, B110,
	"\n\r:login: ",

/* table '1' -- 150 */
	'1', '1',
	ANYP+RAW+NL1+CR1, EVENP+ECHO+FF1+CR2+TAB1+NL1,
	B150, B150,
	"\n\r\033:\006\006\017login: ",

/* table '2' -- 9600 */
	'2', '2',
	ANYP+RAW+NL1+CR1, ANYP+XTABS+ECHO+CRMOD,
	B9600, B9600,
	"\n\r;login: ",

/* table '3'-'5' -- 1200,300 */
	'3', '5',
	ANYP+RAW+NL1+CR1, ANYP+XTABS+ECHO+CRMOD+FF1,
	B1200, B1200,
	"\n\r:login: ",

/* table '5'-'3' -- 300,1200 */
	'5', '3',
	ANYP+RAW+NL1+CR1, ANYP+ECHO+CR1+XTABS,
	B300, B300,
	"\n\r:login: ",

/* table '4' -- Console Decwriter */
	'4', '4',
	ANYP+RAW+NL1+CR2, ANYP+ECHO+CRMOD+XTABS+CR2,
	B300, B300,
	"\n\r:login: ",

/* table '6' -- 1200 */
	'6', '6',
	ANYP+RAW+NL1+CR1, ANYP+XTABS+ECHO+CRMOD,
	B1200, B1200,
	"\n\r;login: ",

/* table '7' -- 2400 */
	'7', '7',
	ANYP+RAW+NL1+CR1, ANYP+XTABS+ECHO+CRMOD,
	B2400, B2400,
	"\n\r;login: ",

/* table '8' -- 4800 */
	'8', '8',
	ANYP+RAW+NL1+CR1, ANYP+XTABS+ECHO+CRMOD,
	B4800, B4800,
	"\n\r;login: ",

/* table '9' -- 9600 */
	'9', '9',
	ANYP+RAW+NL1+CR1, ANYP+XTABS+ECHO+CRMOD,
	B9600, B9600,
	"\n\r;login: ",

/* table 'i' -- Interdata Console */
	'i', 'i',
	RAW+CRMOD, CRMOD+ECHO+LCASE,
	0, 0,
	"\n\r:login: ",

#ifdef	TEXAS_AUTOBAUD
/* table 'B' -- Do autobauding */

#define	NTRIES	5
#define	WAITSEC	60
#define	TB_AUTOBAUD	'B'
	TB_AUTOBAUD, TB_AUTOBAUD,
	ANYP+RAW,	ANYP+XTABS+ECHO+CRMOD,
	B9600, B9600,
	"\n\r;login: ",
#endif

/* table 'T' -- 9600 tektronix 4014 */
	'T', 'T',
	ANYP+RAW+NL1+CR1, ANYP+XTABS+ECHO+CRMOD+FF1,
	B9600, B9600,
	"\n\r\033;login: ",
};

#define	NITAB	sizeof itab/sizeof itab[0]
#define	EOT	04		/* EOT char */

char	name[16];
int	crmod;
int	upper;
int	lower;
#ifdef	DO_CRTBS
int	can_erase;
#endif
#ifdef	SP_SESS
int	sp_sess	= { 0 };	/* flag special session */
#endif

char partab[] = {
	0001,0201,0201,0001,0201,0001,0001,0201,
	0202,0004,0003,0205,0005,0206,0201,0001,
	0201,0001,0001,0201,0001,0201,0201,0001,
	0001,0201,0201,0001,0201,0001,0001,0201,
	0200,0000,0000,0200,0000,0200,0200,0000,
	0000,0200,0200,0000,0200,0000,0000,0200,
	0000,0200,0200,0000,0200,0000,0000,0200,
	0200,0000,0000,0200,0000,0200,0200,0000,
	0200,0000,0000,0200,0000,0200,0200,0000,
	0000,0200,0200,0000,0200,0000,0000,0200,
	0000,0200,0200,0000,0200,0000,0000,0200,
	0200,0000,0000,0200,0000,0200,0200,0000,
	0000,0200,0200,0000,0200,0000,0000,0200,
	0200,0000,0000,0200,0000,0200,0200,0000,
	0200,0000,0000,0200,0000,0200,0200,0000,
	0000,0200,0200,0000,0200,0000,0000,0201
};

main(argc, argv)
char **argv;
{
	register struct tab *tabp;
	int tname;
	int ldisc;
	int bf;
#ifdef	UCB_NTTY
	int lmode = 0;
#endif

	ldisc = DFLT_LDISC;
	ioctl(0, TIOCSETD, &ldisc);
#ifdef UCB_NTTY
	ioctl(0, TIOCLSET, &lmode);
	ioctl(0, TIOCSLTC, &ltc);
#endif
	ioctl(0, TIOCSETC, &tchars);

	tname = '0';
#ifdef	SP_SESS
	/*
	 * If we are called with a "+" name, flag special session.
	 */
	if(argv[0][0] == '+')
		sp_sess++;
#endif

	if (argc > 1)
		tname = argv[1][0];


	switch (tname) {

	/*
	 *	If hardware has decoded high speed (or line is open),
	 *	get the line speed.
	 */
	case '5':
		ioctl(0,TIOCGETP, &tmode);
		if (tmode.sg_ispeed == B1200)
			tname = '3';
		break;

#ifdef	DO_CRTBS
	/*
	 *	Hard-wired crt ports (2400-9600 baud)
	 *	allow use of backspace-erase.
	 */
	case '7':
	case '8':
	case '9':
		can_erase++;
		lmode = LCRTERA | LCRTBS;
		ioctl(0, TIOCLSET, &lmode);
		break;
#endif
	}
	for (;;) {
		for(tabp = itab; tabp < &itab[NITAB]; tabp++)
			if(tabp->tname == tname)
				break;
		if(tabp >= &itab[NITAB])
			tabp = itab;
		tmode.sg_flags = tabp->iflags;
		tmode.sg_ispeed = tabp->ispeed;
		tmode.sg_ospeed = tabp->ospeed;
		ioctl(0, TIOCSETP, &tmode);
		ioctl(0, TIOCSETC, &tchars);
#ifdef	TEXAS_AUTOBAUD
		if ((tabp->tname == TB_AUTOBAUD) && set_baud_rate())
		{	/* autobaud line */
			ioctl(0, TIOCGETP, &tmode); /* pick up baud rate */
			ioctl(0, TIOCSETP, &tmode); /* flush input */
		}
#endif

		if(tmode.sg_ospeed > B1200)
			puts("\n\r");
		else
			puts("\n\r\r\r\r\r\n\r\r\r\r\r");
		if ((bf = open(IDENT, FATT_RDONLY)) > 0) {
			char buf[81];
			int i;
			register char *s;
			while ((i = read(bf, buf, sizeof (buf) - 1)) > 0) {
				buf[i] = 0;
				for (s = buf; *s; s++) {
					if (*s == '\n')
						putchr('\r');
					putchr(*s);
				}
			}
			close(bf);
		}
#ifdef	SP_SESS
		if(sp_sess)
			puts("\n\r\r\r>>> Unix Maintenance Mode <<<");
#endif
		puts(tabp->message);
		/*
		 * wait, then flush input to get rid
		 * of line noise
		 */
		sleep(1);
		ioctl(0, TIOCSETP, &tmode);
		if(getname()) {
			if(upper==0 && lower==0)
				continue;
#ifdef	DO_CRTBS
			tmode.sg_erase = can_erase? '\b': ERASE;
#else
			tmode.sg_erase = ERASE;
#endif
			tmode.sg_kill = KILL;
			tmode.sg_flags = tabp->fflags;
			if(crmod)
				tmode.sg_flags |= CRMOD;
			if(upper)
				tmode.sg_flags |= LCASE;
			if(lower)
				tmode.sg_flags &= ~LCASE;
			ioctl(0, TIOCSETP, &tmode);
			putchr('\n');
#ifdef	SP_SESS
			if(sp_sess)
				execl("/bin/login", "slogin", name, 0);
#endif
			execl("/bin/login", "login", name, 0);
			exit(1);
		}
		tname = tabp->nname;
	}
}

getname()
{
	register char *np;
	register c;
	char cs;

	crmod = 0;
	upper = 0;
	lower = 0;
	np = name;
	for (;;) {
		if (read(0, &cs, 1) <= 0)
			exit(0);
		if ((c = cs&0177) == 0)
			return(0);
		if (c==EOT)
			exit(1);
		if (c=='\r' || c=='\n' || np >= &name[16])
			break;
#ifdef	DO_CRTBS
		if (c != '\b')
#endif
			putchr(cs);
		if (c>='a' && c <='z')
			lower++;
		else if (c>='A' && c<='Z') {
			upper++;
			c += 'a'-'A';
#ifdef	DO_CRTBS
		} else if (c=='\b') {
			if (np > name) {
				np--;
				puts("\b \b");
			}
			continue;
#endif
		} else if (c==ERASE) {
			if (np > name)
				np--;
			continue;
		} else if (c==KILL) {
			putchr('\r');
			putchr('\n');
			np = name;
			continue;
		} else if(c == ' ')
			c = '_';
		*np++ = c;
	}
	*np = 0;
	if (c == '\r')
		crmod++;
	return(1);
}

puts(as)
char *as;
{
	register char *s;

	s = as;
	while (*s)
		putchr(*s++);
}

putchr(cc)
{
	char c;
	c = cc;
	c |= partab[c&0177] & 0200;
	write(1, &c, 1);
}

#ifdef	TEXAS_AUTOBAUD

/* Autobauding tables */
struct match
{	
	char  m_char;	/* bit pattern to look for */
	char  m_mask;	/* bits to ignore in input character */
	short m_speed;	/* what speed to set if matched */
};

struct match match[] = {	/* cr,int            */
	{ 0xfc, 0x03, B9600 },	/* 111111xx (cr,int) */
	{ 0x0d, 0x80, B4800 },	/* x0001101 (cr)     */
	{ 0x03, 0x80, B4800 },	/* x0000011 (int)    */
	{ 0xe6, 0x00, B2400 },	/* 11100110 (cr)     */
	{ 0x1e, 0x00, B2400 },	/* 00011110 (int)    */
	{ 0x8c, 0x12, B1800 },	/* 100x11x0 (cr)     */
	{ 0x7c, 0x02, B1800 },	/* 011111x0 (int)    */
	{ 0x78, 0x80, B1200 },	/* x1111000 (cr,int) */
	{ 0x80, 0x00, B600  },	/* 10000000 (cr,int) */
	{ 0x00, 0x00, B300  },	/* 00000000 (cr,int) */
	{ 0,    0,    0     }
};

/* Determine baud rate of terminal line
 *     Rich Wales (UCLA), June 1982
 *
 * Determine the baud rate of a terminal from a single
 * carriage-return or ^C.  It can identify 300, 600, 1200, 1800, 2400,
 * 4800, and 9600 baud.
 *
 * If the new speed is below 2400 baud, it is advisable to do a "sleep"
 * before the "ioctl".  This allows any extra garbage from the input to
 * make it from the multiplexer (DH or DZ) to the kernel input queue,
 * where the speed change will cause it to be flushed.
 */
timeout()
{
	puts("\7Connection timed out.\n\r");
	exit(1);
};

set_baud_rate()
{   
	register struct match *m;
	register int i;
	int lmode;
	struct sgttyb sv, st;
	char c;

	/* save current TTY mode, set new mode */
	ioctl(0, TIOCGETP, &sv);
	st = sv;
	st.sg_flags  = RAW | ANYP;
	ioctl(0, TIOCLGET, &lmode);
	st.sg_ispeed = st.sg_ospeed = B4800;
	ioctl(0, TIOCSETP, &st);
	ioctl(0, TIOCLSET, &lmode);
	if (ioctl(0, TIOCSIMG, 0) < 0)
		return(0);
	alarm(0);
	for (i=0; i < NTRIES; i++) {
		/* read a character and try to match it */
		signal(14,timeout);
		alarm(WAITSEC);
		read(0,&c,1);
		alarm(0);
		c &= 0377;
		for (m = match; m->m_speed != 0; m++) {
			if ((c &~ m->m_mask) == m->m_char) {
			/* success -- restore old modes, but with new speed */
				sv.sg_ispeed = sv.sg_ospeed = m->m_speed;
				if (sv.sg_ispeed < B2400)
					sleep(1);
				ioctl(0, TIOCCIMG, 0);
				ioctl(0, TIOCSETP, &sv);
				ioctl(0, TIOCLSET, &lmode);
				return(1);
			}
		}
	}
	/* no match -- restore old modes */
	sleep(1);
	ioctl(0, TIOCCIMG, 0);
	ioctl(0, TIOCSETP, &sv);
	ioctl(0, TIOCLSET, &lmode);
	return(0);
}
#endif
