#
/*	gsi 2.11 of 5/31/77 */
/*
/*	gsi [+12] [-n] [-o] [-dt,l,c]
 */
#define	PLOT	006	/* ack, on/off plot mode */
#define BEL    007     /* exit plot mode */
#define	ESC	033	/* escape */
#define	HFWD	'9'
#define	HREV	'8'
#define	FREV	'7'
#define	SO	016	/* shift out - enter greek */
#define	SI	017	/* shift in */
#define	UP	013
#define	DN	012
#define	RT	' '
#define	LF	'\b'
#define LI	'\n'
#define CR	015
#define TABVAL	4	/* approx equivalent of blanks per tab */
#define FLIPMODE	intrmod =^ 01

int	nlcnt,		/* accumulated newline count */
	frevcnt,	/* accumulated reverse line-feeds */
	pitch12,	/* 1==> 12-pitch&6lines/inch, 0==>10-pitch */
	fullsiz,	/* # increments for full line */
	halfsiz 4;	/* # increments for halfline */
int	tabcnt,		/* accumulated tabs in 1 line */
	charcnt,	/* # chars in line */
	nblcnt,		/* nonblanks contiguous */
	msgrest 1;	/* 1==> restore mesg status, 0==> don't */
int	delay[3] {3,90,20};
			/* tab limit, characters/line, contiguous lim*/
char 	*ttydev;	/* will pt to /dev/ttyx */
int	svflags;	/* for mesg restore */
int	restore();
int	svmode, mode[3];
char	*scanptr;	/* side-effect of getnum() */

int	intrmod	1,	/* internal mode, 1==> text, 0 ==> plot */
	extrmod 1;	/* external, same */

struct buf {
	int fildes;	/* file descriptor */
	int nleft;	/* bytes left */
	char *nextp;	/* ptr to next char */
	char buffer[512];	/* i/o area */
} fin, fout;

char gsi_____[] "@(#)gsi.c	2.11";
main(argc,argv) int argc; char **argv; {
	register int c;
	ttydev = "/dev/ttyx";	/* initialize tty name */
	scanarg(argc,argv);
	fout.fildes = 1;
		/* catch interrupt, if not already ignored */
	if ((signal(2,1) & 01) == 0) signal(2,&restore);
	if (gtty(1,mode) == 0) fixtty();
	for(;;) {
		if (fin.nleft-- > 0)
			c = *fin.nextp++;
		else if ((c = getchal()) < 0 )
			break;
		if (intrmod) {
			if (c == '\n') {
				if (frevcnt) flushrv();
				nlcnt++;
				continue;
			}
			else if (nlcnt) flushnl();
			else if (frevcnt && c != ESC) flushrv();
		}
		if (c == PLOT) {
			FLIPMODE;
			continue;
		}
		if (c == BEL) {
			extrmod = intrmod = 1;
			putc(BEL,&fout);
			continue;
		}
		if(c == SO){
			special();
			continue;
		}
		if( c != ESC ){
			charcnt++;
			if (c == ' ') nblcnt = 0;
			else if (c == '\t') {
				charcnt =+ TABVAL;
				nblcnt = 0;
				if (++tabcnt >= delay[0]) {
					putx('\0');
					tabcnt = 0;
					if (delay[0] == 0)	/* 2 nulls per tab */
						putx('\0');
				}
			}
			else if (c == '\b') charcnt--;
			else if (++nblcnt >= delay[2]) {
				nblcnt = 0;
				putx('\0');
				if (delay[2] == 0) putx('\0');
			}
			if (intrmod != extrmod) {
				putc(PLOT,&fout);	/* flip real state */
				extrmod = intrmod;
			}
			putc(c,&fout);	/* most frequent put */
			continue;
		}
		FLIPMODE;
		c = getchal();
		if (frevcnt && c != FREV) {
			FLIPMODE;
			flushrv();
			FLIPMODE;
		}
		if( c == HREV )
			nplot(halfsiz,UP);
		else if( c == HFWD )
			nplot(halfsiz,DN);
		else if( c == FREV ) frevcnt++;
		FLIPMODE;
	}
	flusher();
	restore();
}

/*	getchal: local variant of getchar with stop mode processing:
	note EOF return is -1 rather than 0 */
getchal()
{
	if (fin.nleft <= 0) {
		/* make sure output flushed if last char was newline,
		thus allowing nroff -s or .rd requests, with small overhead */
		if (fin.nextp != 0 && *--fin.nextp == LI) flusher();
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
		if (*p == '+' && *++p == '1' && *++p == '2')
			pitch12 = 1;
		else if (*p == '-') {
			if (*++p == 'o') msgrest = 0;
			else if (*p == 'd') getdelay(++p);
			else if (*p > '0' && *p <= '9') halfsiz = *p - '0';
		}
	}
	fullsiz = pitch12 ? 6 : 8;
	return;
}

/*	getdelay: scan fields of delay arg */
getdelay(p)
char *p;
{
	register int i,j;
	for( i = 0; i <= 2; i++) {
		if (*p == ',') {
			p++;
			continue;
		}
		if (*p == '\0') break;
		delay[i] = getnum(p);
		p = scanptr+1;
		if (*scanptr != ',') break;
	}
	return;
}

/*	fixtty: get tty status and save; remove CR-LF mapping */
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
	mode[2] =& 0177757;	/* turn off cr-lf  */
	/* leave delay alone for time being, until stty settles down */
	stty(1, mode);
	if (msgrest) {
		fstat(1,&ibufx);
		svflags = ibufx.iflags;
	}
	ttydev[8] = ttyn(1);
	chmod(ttydev,0600);	/* mesg n */
	return;
}

getnum(p1)
char *p1;
{
	register int i;
	register char *p;
	p = p1;
	i = 0;
	while (*p >= '0' && *p <= '9') i = 10*i + *p++ - '0';
	scanptr = p;
	return(i);
}

/*	flusher: flush accumulated newlines, reverse line feeds, buffer */
flusher()
{
	if (nlcnt) flushnl();
	if (frevcnt) flushrv();
	fflush(&fout);
	return;
}

/*	flushrv: flush accumulated reverse line feeds */
/*	note: expects to be out of plot mode on entry */
char frv1[] {UP,PLOT,UP,UP,PLOT,0};	/* 1 FREV leftover */
char frv2[] {UP,UP,PLOT,UP,UP,UP,UP,PLOT,0};	/* 2 of them */
char frvadj[] {UP,UP,UP,LI,LI,LI,0};	/* forms tractor fixup */
flushrv()
{
	register int numleft;
	if (pitch12) {
		numleft = frevcnt % 3;
		frevcnt = 4 * (frevcnt / 3);
	}
	else numleft = 0;
	while (frevcnt--) {
		putx(UP);
		nplot(5,'\0');		/* slow down somewhat */
	}
	if (numleft == 1) putstr(frv1);
	else if (numleft == 2) putstr(frv2);
	putstr(frvadj);	/* up and down for forms tractor */
	frevcnt = 0;
	return;
}

/*	flushnl: flush accumulated newlines (count in nlcnt) */
char nl1[] {LI, PLOT, LI, LI, PLOT, 0}; /* 12pitch: 1 nl */
char nl2[] {LI, LI, PLOT, LI, LI, LI, LI, PLOT, 0}; /* 2 nls */
flushnl()
{
	register int numleft;
	if (pitch12) {
		numleft = nlcnt % 3;
		nlcnt = 4 * (nlcnt/3);
	}
	else numleft = 0;
	putx(CR);	/* must have 1 CR; only 1 needed */
	while (nlcnt--)
		putx(LI);	/* no plot mode needed for these */
	if (numleft == 1) putstr(nl1);
	else if (numleft == 2) putstr(nl2);
	if (charcnt > delay[1]) nplot(1 + charcnt/20,'\0');
	nlcnt = charcnt = nblcnt = tabcnt = 0;
	return;
}

putstr(p)
char *p;
{
	register char *pp;
	pp = p;
	while (*pp) putx(*pp++);
	return;
}

char bell BEL;
restore(){
	write(1,&bell,1);	/* make sure back in text mode */
	if (ttydev[8] != 'x') {
		mode[2] = svmode;
		stty(1,mode);
		if (msgrest) chmod(ttydev,svflags);
	}
	exit(0);
}

char alpha[]	{LF,'c',RT,RT,'(',LF,0};
char beta[]	{'B',LF,LF,DN,DN,'|',RT,RT,UP,UP,0};
char delta[]	{'o',UP,UP,'<',DN,DN,0};
char DELTA[]	{LF,LF,'/',-3,DN,'-',-4,RT,'-',-3,UP,'\\',LF,LF,0};
char epsilon[]	{'<','-',0};
char eta[]	{'n',RT,RT,DN,DN,'|',LF,LF,UP,UP,0};
char gamma[]	{')',RT,'/',LF,0};
char GAMMA[]	{LF,LF,'|',RT,RT,-3,UP,'-',-3,DN,RT,RT,'`',LF,LF,0};
char infinity[]	{LF,LF,'c',-4,RT,'o',LF,LF,0};
char integral[]	{'|','\'',RT,RT,'`',-3,LF,-6,DN,'\'',LF,'`',RT,RT,-6,UP,0};
char lambda[]	{'\\',-4,DN,LF,'\'',DN,LF,'\'',-5,UP,RT,RT,0};
char LAMBDA[]	{LF,LF,'/',-4,RT,'\\',LF,LF,0};
char mu[]	{'u',LF,LF,',',RT,RT,0};
char nabla[]	{LF,LF,'\\',-3,UP,'-',-4,RT,'-',-3,DN,'/',LF,LF,0};
char not[]	{'-',-2,RT,UP,',',DN,-2,LF,0};
char nu[]	{LF,'(',-3,RT,'/',LF,LF,0};
char omega[]	{LF,'u',-3,RT,'u',LF,LF,0};
char OMEGA[]	{'O',DN,DN,LF,'-',RT,RT,'-',LF,UP,UP,0};
char partial[]	{'o',RT,DN,'`',LF,UP,'`',LF,UP,'`',RT,DN,0};
char phi[]	{'o','/',0};
char PHI[]	{'o','[',']',0};
char psi[]	{'/','-',DN,DN,RT,RT,'\'',-4,LF,'\'',RT,RT,UP,UP,0};
char PSI[]	{'[',']','-',DN,DN,RT,RT,'\'',-4,LF,'`',RT,RT,UP,UP,0};
char pi[]	{UP,'-',-3,DN,'"',DN,'"',-3,UP,0};
char PI[]	{LF,LF,'[',']',-4,RT,'[',']',LF,LF,-3,UP,'-',-3,DN,0};
char rho[]	{'o',LF,LF,DN,DN,'|',UP,UP,RT,RT,0};
char sigma[]	{'o',DN,RT,RT,'~',UP,LF,LF,0};
char SIGMA[]	{'>',-2,DN,'-',-5,UP,'-',DN,DN,0};
char tau[]	{'t',DN,RT,RT,'~',LF,LF,LF,'~',RT,UP,0};
char theta[]	{'O','-',0};
char THETA[]	{'O','=',0};
char xi[]	{'c',RT,DN,',',LF,-3,UP,'c',LF,DN,'`',RT,DN,0};
char zeta[]	{'c',RT,DN,',',LF,-3,UP,'<',DN,DN,0};

char	tab[]{
	'A',	/* alpha */
	'B',	/* beta */
	'D',	/* delta */
	'W',	/* DELTA */
	'S',	/* epsilon */
	'N',	/* eta */
	'\\',	/* gamma */
	'G',	/* GAMMA */
	'o',	/* infinity - not in M37 */
	'^',	/* integral */
	'L',	/* lambda */
	'E',	/* LAMBDA */
	'M',	/* mu */
	'[',	/* nabla (del) */
	'_',	/* not */
	'@',	/* nu */
	'C',	/* omega */
	'Z',	/* OMEGA */
	']',	/* partial */
	'U',	/* phi */
	'F',	/* PHI */
	'V',	/* psi */
	'H',	/* PSI */
	'J',	/* pi */
	'P',	/* PI */
	'K',	/* rho */
	'Y',	/* sigma */
	'R',	/* SIGMA */
	'I',	/* tau */
	'T',	/* theta */
	'O',	/* THETA */
	'X',	/* xi */
	'Q',	/* zeta */
	0
};
int	trans[]{
	alpha,
	beta,
	delta,
	DELTA,
	epsilon,
	eta,
	gamma,
	GAMMA,
	infinity,
	integral,
	lambda,
	LAMBDA,
	mu,
	nabla,
	not,
	nu,
	omega,
	OMEGA,
	partial,
	phi,
	PHI,
	psi,
	PSI,
	pi,
	PI,
	rho,
	sigma,
	SIGMA,
	tau,
	theta,
	THETA,
	xi,
	zeta,
	0
};

special(){
	register int c, i;
   loop:
	if ((c = getchal()) == SI || c < 0)
		return;
	for (i = 0; tab[i] != 0; i++)
		if( c==tab[i] ){
			plot(trans[i]);
			goto loop;
		}
	putx(c);
	goto loop;
}

plot(s) char *s; {
	register int i,c;
	FLIPMODE;
	for( i=0; (c=s[i])!=0; i++ ) {
		if( c<0 )
			nplot(-c,s[++i]);
		else
			putx(c);
	}
	FLIPMODE;
	putx(' ');
}

nplot(n,c) int n,c; {
	while(n--)
		putx(c);
}

/*	putx: add ordinary (not PLOT or BEL) character to output, adding
	extra PLOT when needed to flip state. */
putx(c)
char c;
{
	if (intrmod != extrmod) {
		putc(PLOT,&fout);	/* modes mismatch; flip for real */
		extrmod = intrmod;
	}
	putc(c,&fout);
	return;
}
