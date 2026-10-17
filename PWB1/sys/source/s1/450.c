#
/*	450 1.4 of 5/31/77 */
/*
 *	450	(for DASI 450/DIABLO 1620 terminals)
 *	derived from gsi 2.9 of 9/16/75
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

int	nlcnt,		/* accumulated newline count */
	frevcnt;	/* accumulated reverse line-feeds */
char 	*ttydev;	/* will pt to /dev/ttyx */
int	svflags;	/* for mesg restore */
int	restore();
int	svmode, mode[3];

struct buf {
	int fildes;	/* file descriptor */
	int nleft;	/* bytes left */
	char *nextp;	/* ptr to next char */
	char buffer[512];	/* i/o area */
} fin, fout;

char d450___[]	"@(#)450.c	1.4";
main(argc,argv) int argc; char **argv; {
	register int c;
	ttydev = "/dev/ttyx";	/* initialize tty name */
	fout.fildes = 1;
		/* catch interrupt, if not already ignored */
	if ((signal(2,1) & 01) == 0) signal(2,&restore);
	if (gtty(1,mode) == 0) fixtty();
	for(;;) {
		if (fin.nleft-- > 0)
			c = *fin.nextp++;
		else if ((c = getchal()) < 0 )
			break;
		if (c == '\n') {
			if (frevcnt) flushrv();
			nlcnt++;
			continue;
		}
		else if (nlcnt) flushnl();
		else if (frevcnt && c != ESC) flushrv();
		if(c == SO){
			special();
			continue;
		}
		if( c != ESC ){
			putc(c,&fout);	/* most frequent put */
			continue;
		}
		c = getchal();
		if (frevcnt && c != FREV) {
			flushrv();
		}
		if (c == HREV || c == HFWD)
			pout(c);
		else if( c == FREV ) frevcnt++;
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
	fstat(1, &ibufx);
	svflags = ibufx.iflags;
	ttydev[8] = ttyn(1);
	chmod(ttydev,0600);	/* mesg n */
	return;
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
char frvadj[] {UP,UP,UP,LI,LI,LI,0};	/* forms tractor fixup */
flushrv()
{
	register int numleft;
	while (frevcnt--) {
		putx(UP);
		nplot(5,'\0');		/* slow down somewhat */
	}
	putstr(frvadj);	/* up and down for forms tractor */
	frevcnt = 0;
	return;
}

/*	flushnl: flush accumulated newlines (count in nlcnt) */
flushnl()
{
	register int numleft;
	putx(CR);	/* must have 1 CR; only 1 needed */
	while (nlcnt--)
		putx(LI);	/* no plot mode needed for these */
	nlcnt = 0;
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

restore(){
	write(1, "\0333", 1);	/* assure text mode */
	if (ttydev[8] != 'x') {
		mode[2] = svmode;
		stty(1,mode);
		chmod(ttydev,svflags);
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
	pout(PLOT);
	for( i=0; (c=s[i])!=0; i++ ) {
		if( c<0 )
			nplot(-c,s[++i]);
		else
			putx(c);
	}
	pout(BEL);
	putx(' ');
}

nplot(n,c) int n,c; {
	while(n--)
		putx(c);
}

/*	pout: put out appropriate sequence for motions */
char hrvadj[]	{ESC,'D', ESC,'U',0};
pout(type)
int type;
{
	putx(ESC);
	switch(type) {
	case HFWD:
		putx('U'); break;
	case HREV:
		putx('D');
		putstr(hrvadj);		/* up & down for forms tractor */
		break;
	case PLOT:
		putx('3'); break;	/* into plot mode */
	case BEL:	/* out of plot mode */
		putx('4'); break;
	}
}

/*	putx: add character to output; convert UP to ESC LI */
putx(c)
char c;
{
	if (c == UP) {
		putc(ESC, &fout); putc(LI, &fout);
	} else
		putc(c,&fout);
	return;
}
