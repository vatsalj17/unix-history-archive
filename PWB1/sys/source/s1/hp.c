/*	hp 2.5 of 4/10/77
 *	hp [-e] [-m]
 *	convert nroff TTY37 output to right form for HP2640x terminals.
 *	-e indicates enhanced terminal with underline, etc.
 *	-m requests minimization of output by removing multiple newlines
 */
#define	ESC	033	/* escape */
#define	HFWD	'9'
#define	HREV	'8'
#define	FREV	'7'
#define	SO	016	/* shift out - enter greek */
#define	SI	017	/* shift in */
#define LF	'\n'
#define CR	015
#define	NFLAG	0200	/* hi-bit flag for non-showing chars */
#define	NMASK	0177
#define	BFSIZ	300	/* size of output buffer */

int	nlcnt,		/* accumulated newline count */
	frevcnt,	/* accumulated reverse line-feeds */
	halfpos,	/* half-line position: -1 = above, +1 = below */
	restrict 1,	/* 0==> full terminal, 1==> no display enhancements */
	minimiz;	/* 0==> normal, 1==> remove extra newlines */
char	ttydev[10];	/* modification area for following */
char	*ttydevx "/dev/ttyx";
int	svflags;	/* for mesg restore */
int	restore();
int	svmode, mode[3];
int	retcode	1;	/* return code */

char	outbf[BFSIZ];	/* output assembly buffer */
char	*bfnext outbf,	/* addr of next empty byte */
	*bflast &outbf[BFSIZ-1];	/* addr of last usable byte */

/*	normal display enhancement strings */
char	*enunder	"\033&dD",	/* underline */
	*ennorml	"\033&d@",	/* normal output */
	*ensuper	"\033&dH",	/* superscript (half) */
	*ensubsc	"\033&dL",	/* subscript (half, underlined */
	*enrestr	"\033&dB";	/* restricted (inverse only) */

struct buf {
	int fildes;	/* file descriptor */
	int nleft;	/* bytes left */
	char *nextp;	/* ptr to next char */
	char buffer[512];	/* i/o area */
} fin;

static char SCCSID[] "@(#)hp.c	2.5";
main(argc,argv) int argc; char **argv; {
	register int c;
	extern int fout;
	copy(ttydevx,ttydev);	/* initialize string work area */
	scanarg(argc,argv);
	fout = dup(1);	/* buffer output */
		/* catch interrupt, if not already ignored */
	if ((signal(2,1) & 01) == 0) signal(2,&restore);
	if (gtty(1,mode) == 0) fixtty();
	while((c = getchal()) >= 0) {
		if (nlcnt && c != LF) flushnl();
		if (frevcnt && c != ESC) flushrv();
		if (c == LF) {
			if (++nlcnt == 1) flushln();
			continue;
		}
		if (c == ESC)
			escape();
		else if (c == SO)
			special();
		else if (c == '\b')
			backsp();
		else if (c == '_') {		/* C nroff */
			if ((c = getchal()) == '\b')
				undersc();
			else {
				putx('_'); putx(c);
			}
		}
		else putx(c);
	}
	flusher();
	retcode = 0;
	restore();
}

/*	getchal: local variant of getchar with stop mode processing:
	note EOF return is -1 rather than 0 */
char *peekstr "\0";	/* lookahead ptr */
getchal()
{
	if (*peekstr)
		return(*peekstr++);
	if (fin.nleft == 0) {
		if (fin.nextp != 0 && *--fin.nextp == LF) flusher();
		fin.nleft = read(fin.fildes,fin.buffer,512);
		fin.nextp = fin.buffer;
	}
	if (fin.nleft-- > 0) return(*fin.nextp++);
	return(-1);
}

/*	scanarg: scan arguments and set flags; ignore unknown args */
scanarg(argc,argv)
	int argc; char **argv;
{
	register char *p;
	while ( --argc > 0) {
		p = *++argv;
		if (*p == '-') {
			++p;
			if (*p == 'e') restrict = 0;
			else if (*p == 'm') minimiz = 1;
		}
	}
	if (restrict)
		enunder = ensuper = ensubsc = enrestr;
	return;
}

/*	fixtty: get tty status and save; remove delay and CR-LF mapping */
fixtty()
{

struct ibuf {
	int	idev;
	int	inum;
	int	iflags;
	char	inl;
	char	iuid;
	char	igid;
	char	isize0;
	int	isize;
	int	iaddr[8];
	char	*iatime[2];
	char	*imtime[2];
} ibufx;
	svmode = mode[2];
	mode[2] =& 0000357;	/* remv all delays for sure; CR-LF mapping */
	stty(1,mode);	/* stty nl nl0 cr0 tab0 ff0 */
	fstat(1,&ibufx);
	svflags = ibufx.iflags;
	ttydev[8] = ttyn(1);
	chmod(ttydev,0600);	/* mesg n */
	return;
}

/*	flusher: flush accumulated newlines, reverse line feeds, buffer */
flusher()
{
	flushln();
	if (nlcnt) flushnl();
	if (frevcnt) flushrv();
	flush();
	return;
}

/*	flushrv: flush accumulated reverse line feeds */
flushrv()
{
	while (frevcnt--) {
		putchar(ESC);
		putchar('T');	/* roll down */
	}
	frevcnt = 0;
	return;
}

/*	flushnl: flush accumulated newlines (count in nlcnt) */
flushnl()
{
	if (minimiz != 0 && nlcnt > 2) nlcnt = 2;
	putchar(CR);
	while (nlcnt--)
		putchar(LF);
	nlcnt = 0;
	return;
}

putstr(p)
char *p;
{
	register char *pp;
	pp = p;
	while (*pp) puty(*pp++);
	return;
}

restore(){
	if (ttydev[8] != 'x') {
		mode[2] = svmode;
		stty(1,mode);
		chmod(ttydev,svflags);
	}
	exit(retcode);
}

char	tab[]{
	'A','A',	/* alpha */
	'B','B',	/* beta */
	'D','W',	/* delta */
	'W','V',	/* DELTA */
	'S','E',	/* epsilon */
	'N','H',	/* eta */
	'\\','Q',	/* gamma */
	'G','+',	/* GAMMA */
	'o','<',	/* infinity - not in M37 */
	'^','\'',	/* integral */
	'L','G',	/* lambda */
	'E',';',	/* LAMBDA */
	'M','M',	/* mu */
	'[','$',	/* nabla (del) */
	'_','\\',	/* not */
	'@','N',	/* nu */
	'C','L',	/* omega */
	'Z',':',	/* OMEGA */
	']','F',	/* partial */
	'U','D',	/* phi */
	'F','.',	/* PHI */
	'V','C',	/* psi */
	'H',',',	/* PSI */
	'J','P',	/* pi */
	'P','*',	/* PI */
	'K','O',	/* rho */
	'Y','S',	/* sigma */
	'R','?',	/* SIGMA */
	'I','T',	/* tau */
	'T','R',	/* theta */
	'O','J',	/* THETA */
	'X','U',	/* xi */
	'Q','Z',	/* zeta */
	'v','Y',
	0
};

char spec1[] {LF,SO,0};
special(){
	register int c;
	register char *p;
	puty(SO);
	while ((c = getchal()) >= 0) {
		if (c == SI) {
			puty(c);
			return;
		}
		if (c == LF) {
			peekstr = spec1;
			return;
		}
		for (p = tab; *p != 0; p =+ 2)
			if (c == *p) {
				c = *++p;
				break;
			}
		putx(c);
	}
	return;
}

/*	escape: handle escape sequences */
escape()
{
	register int c;
	c = getchal();
	if (frevcnt && c != FREV) flushrv();
	switch (c) {
	case FREV:
		frevcnt++;
		break;
	case HREV:
		if (halfpos == 0) {
			putstr(ensuper);
			halfpos--;
		}
		else if (halfpos > 0) {
			putstr(ennorml);
			halfpos--;
		}
		else {
			putstr("\033T"); /* roll back 1 */
			putstr(ennorml);
			halfpos = 0;
		}
		break;
	case HFWD:
		if (halfpos == 0) {
			putstr(ensubsc);
			halfpos++;
		}
		else if (halfpos < 0) {
			putstr(ennorml);
			halfpos++;
		}
		else {
			putstr("\033S");	/* roll up 1, i.e., LF w/o CR */
			putstr(ennorml);
			halfpos = 0;
		}
		break;
	case '&':
		putstr("\033&");
		puty(c = getchal());
		if (c == 'd') puty(getchal());
		break;
	case ')':
		putstr("\033)");
		puty(getchal());
		break;
default:
		puty(ESC);
		puty(getchal());
		break;
	}
	return;
}

/*	backsp: handle backspacing */
backsp()
{
	register char c;
	register char *bftmp;
	char *bfhi;
	int i,bscnt;
	bscnt = 1;
	while ((c = getchal()) == '\b') bscnt++;
	for (i = 1; i < bscnt; i++)
		getchal();	/* throw away chars, assumed _ */
	bftmp = bfhi = bfnext +4;
	for (i = 0; i < bscnt;)
		if (((*--bftmp = *--bfnext) & NFLAG) == 0) i++;
	putstr(enunder);
	bfnext = bfhi;
	putstr(ennorml);
	return;
}

/*	undersc: handle C nroff's (_ BS char)+ sequences
 *	assumes _ BS already found
 */
char	peeku[3];
undersc()
{
	register char c;

	putstr(enunder);
	while (1) {
		putx(getchal());
		if ((c = getchal()) != '_') {
			peeku[0] = c; peeku[1] = '\0';
			peekstr = peeku;
			break;
		}
		if ((c = getchal()) != '\b') {
			peeku[0] = '_'; peeku[1] = c;
			peekstr = peeku;
			break;
		}
	}
	putstr(ennorml);
	return;
}

/*	flushln: flush out accumulated line */
flushln()
{
register char c, *p;
	putx('\0');
	p = outbf;
	while (c = (*p++ & NMASK)) putchar(c);
	bfnext = outbf;
	return;
}

/*	putx: add normal (printing) char to output */
putx(c)
char c;
{
	if (bfnext <= bflast)
		*bfnext++ = c;
	else abend("line too long\n");
	return;
}

/*	puty: add control char to output buffer */
/*	mark by setting hi bit on */
puty(c)
char c;
{
	if (bfnext <= bflast)
		*bfnext++ = c | NFLAG;
	else abend("line too long\n");
	return;
}

/*	copy: copy string s1 to s2 */
copy(s1,s2)
register char *s1, *s2;
{
	while (*s2++ = *s1++);
	return;
}

abend(mesg)
char *mesg;
{
	register char *p;
	p = mesg;
	while (*p)
		write(2,p++,1);
	restore();
}
