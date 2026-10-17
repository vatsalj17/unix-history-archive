#
/*
 * getty -- adapt to terminal speed on dialup, and call login
 */

/*
 * tty flags
 */
#define	HUPCL	01
#define	XTABS	02
#define	LCASE	04
#define	ECHO	010
#define	CRMOD	020
#define	RAW	040
#define	ODDP	0100
#define	EVENP	0200
#define	ANYP	0300

/*
 * Delay algorithms
 */
#define	CR1	010000
#define	CR2	020000
#define	CR3	030000
#define	NL1	000400
#define	NL2	001000
#define	NL3	001400
#define	TAB1	002000
#define	FF1	040000

#define	ERASE	'#'
#define	KILL	'@'

/*
 * speeds
 */
#define	B110	3
#define	B150	5
#define	B300	7
#define B1200	9
#define B2400	11
#define	B9600	13


struct	tty {
	char	ispd, ospd;
	char	erase, kill;
	int	flags;
} tmode;

struct	tab {
	int	tname;		/* this table name */
	int	nname;		/* successor table name */
	int	iflags;		/* initial flags */
	int	fflags;		/* final flags */
	int	ispeed;		/* input speed */
	int	ospeed;		/* output speed */
	char	*message;	/* login message */
} itab[] {

/* table '0'-1-2 300,150,110 */

	'0', 1,
	ANYP+RAW+NL1+CR1+HUPCL, ANYP+ECHO+CR1,
	B300, B300,
	"\n\r\033;\007login: ",

	1, 2,
	ANYP+RAW+NL1+CR1+HUPCL, EVENP+ECHO+FF1+TAB1+NL1,
	B150, B150,
	"\n\r\033:\006\006\017login: ",

	2, '0',
	ANYP+RAW+NL1+CR1+HUPCL, ANYP+ECHO+CRMOD+XTABS+LCASE+CR1,
	B110, B110,
	"\n\rlogin: ",

/* table '-' -- Console TTY 110 */
	'-', '-',
	ECHO+CRMOD+XTABS, ECHO+CRMOD+XTABS,
	B300, B300,
	"\n\rlogin: ",

/* table '1' -- 1200 */
	'1', '1',
	EVENP+RAW, EVENP+ECHO+CRMOD+NL2+CR2,
	B1200, B1200,
	"\n\r\033:login: ",

/* table '2' -- 2400 */
	'2', 3,
	ANYP+RAW, ANYP+XTABS+ECHO+CRMOD,
	B2400, B2400,
	"\n\rlogin: ",

	3, 4,
	ANYP+RAW, ANYP+XTABS+ECHO+CRMOD+CR1,
	B9600, B9600,
	"\n\rlogin: ",

	4, '2',
	ANYP+RAW, ANYP+XTABS+ECHO+CRMOD,
	B1200, B1200,
	"\n\rlogin: ",

};

#define	NITAB	sizeof itab/sizeof itab[0]
#define	EOT	04		/* EOT char */

char	name[16];
int	crmod, upper, lower;

main(argc, argv)
char **argv;
{
	register struct tab *tabp;
	register tname;

	tname = '0';
	if (argc > 1)
		tname = *argv[1];
	for (;;) {
		for(tabp = itab; tabp < &itab[NITAB]; tabp++)
			if(tabp->tname == tname)
				break;
		if(tabp >= &itab[NITAB])
			tabp = itab;
		tmode.ispd = tabp->ispeed;
		tmode.ospd = tabp->ospeed;
		tmode.flags = tabp->iflags;
		stty(0, &tmode);
		puts(tabp->message);
		stty(0, &tmode);
		if(getname()) {
			tmode.erase = ERASE;
			tmode.kill = KILL;
			tmode.flags = tabp->fflags;
			if(crmod)
				tmode.flags =| CRMOD;
			if(upper)
				tmode.flags =| LCASE;
			if(lower)
				tmode.flags =& ~LCASE;
			stty(0, &tmode);
			if((tabp->iflags&ECHO)==0)
				write(1, "\n", 1);
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
	extern f();
	static cs;

	crmod = upper = lower = 0;
	np = name;
	for (;;) {
		if(tmode.flags&HUPCL)
			{signal(14,f);
			alarm(120);}
		if (read(0, &cs, 1) <= 0)
			exit(0);
		alarm(0);
		if ((c = cs&0177) == 0)
			return(0);
		if (c==EOT)
			exit(1);
		if (c=='\r' || c=='\n' || np >= &name[16])
			break;
		if((tmode.flags&ECHO)==0)
			write(1, &cs, 1);
		if (c>='a' && c <='z')
			lower++;
		else if (c>='A' && c<='Z') {
			upper++;
			c =+ 'a'-'A';
		} else if (c==ERASE) {
			if (np > name)
				np--;
			continue;
		} else if (c==KILL) {
			np = name;
			continue;
		} else if (c==' ')
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
	while (*s) {
		if (parity(*s))
			*s =| 0200;
		write(1, s++, 1);
	}
}

parity(ac)
{
	register bit, c, sum;

	c = ac;
	sum = 0;
	for (bit=1; bit<0400; bit=<<1)
		if (bit&c)
			sum++;
	return(sum&1);
}

f() {}
