#
#define	SIGHUP	1
#define	SIGINTR	2
#define	SIGQUIT	3
#define SIGPIPE 13
#define	FNSIZE	64
#define	LBSIZE	512
#define	ESIZE	128
#define	GBSIZE	256
#define	NBRA	5
#define	EOF	-1

#define	CBRA	1
#define	CCHR	20
#define	CDOT	4
#define	CCL	8
#define	NCCL	12
#define	CDOL	16
#define	CEOF	17
#define	CKET	18

#define	STAR	01
#define PLUS	02
#define RNGE	03
#define MINUS	16

#define	error	errfunc()
#define	READ	0
#define	WRITE	1

/*------------------------------------------------------------
	Variables to change capacity:

	Param		Def			128k	256k	512k

	bndry	Byte multiple			4	8	16
	shft	Shift count to convert to	1	2	3
		byte address
	offmsk	Offset mask			0377	0177	077
	blkmsk	Block number mask (after	0377	0777	01777
		right shifting by OFFBTS)
	offbts	Bits in offset			8	7	6
	incrmt	Ptr increment to get to		0400	0200	0100
		next block
	lbtmsk	Low bit mask			0774	0770	0760
	nmblks	Max number of blocks		255	511	1023

	Following variables are set up for 128k:
*/
int bndry	4;
int shft	1;
int offmsk	0377;
int blkmsk	0377;
int offbts	8;
int incrmt	0400;
int lbtmsk	0774;
int nmblks	255;
int bed[8][2] {
8,16,
2,3,
0177,077,
0777,01777,
7,6,
0200,0100,
0770,0760,
511,1023
};
/*------------------------------------------------------------*/
struct Fspec {
	char Ftabs[22];
	char Fdel;
	char Flim;
	char Fmov;
	char Ffill;
};
struct Fspec fss;
struct filsys {
	unsigned int s_tfree;      /* free blocks  */
	unsigned int s_tinode;     /* free inodes  */
	char s_fname[6];  /* filsys name  */
	char s_fpack[6];  /* pack name  */
} ubuf;

/*
 * Editor
 * Copyright 1974, Bell Telephone Laboratories, Incorporated
 */

int	peekc;
char	*sep;
int	lcnt,dcnt,cflg;
int	lflg;
int	sflg;
int	rflg;
int	stbuf[18];
int	ccount;
int	lastc;
char	WRERR[] "WRITE ERROR";
char	savedfile[FNSIZE];
char	file[FNSIZE];
char	funny[LBSIZE];
char	linebuf[LBSIZE];
char	rhsbuf[LBSIZE/2];
char	expbuf[ESIZE+4];
int	circfl;
int	*zero;
int	*dot;
int	*dol;
int	*endcore;
int	*fendcore;
int	*addr1;
int	*addr2;
char	genbuf[LBSIZE];
int	count[2];
char	*nextip;
char	*linebp;
int	ninbuf;
int	io;
int	pflag;
int	onhup;
int	onquit;
int	onpipe;
int	vflag	1;
int	listf;
int	col;
char	*globp;
int	tfile	-1;
int	tline;
char	*tfname;
char	*loc1;
char	*loc2;
char	*locs;
char	ibuff[512];
int	iblock	-1;
char	obuff[512];
int	oblock	-1;
int	ichanged;
int	nleft;
int	errfunc();
char	TMPERR[] "TMP";
int	names[26];
int	anymarks;
char	*braslist[NBRA];
char	*braelist[NBRA];
char	hfile[] "/tmp/etm";
int	shflg;
int	wflg;
int	aflg 1;
int	fflg;
int	subnewa;
int	subolda;
char	prompt	'*';
int errno;

main(argc, argv)
char **argv;
{
	register char *p1, *p2;
	register int type;
	extern int onintr();

	onquit = signal(SIGQUIT, 1);
	onhup = signal(SIGHUP, 1);
	onpipe = signal(SIGPIPE, 1);
	if (**argv == 'r') rflg++;
	argv++;
	if (argc > 1 && **argv=='-') {
		vflag = aflg = 0;
		/* allow debugging quits? */
		if ((*argv)[1]=='q') {
			signal(SIGQUIT, 0);
			vflag++;
		}
		if ((*argv)[1]=='b') {
			switch((*argv)[2]) {
			case '1':
			case '2':
				type = ((*argv)[2] - '0') - 1;
				bndry = bed[0][type];
				shft = bed[1][type];
				offmsk = bed[2][type];
				blkmsk = bed[3][type];
				offbts = bed[4][type];
				incrmt = bed[5][type];
				lbtmsk = bed[6][type];
				nmblks = bed[7][type];
				break;
			case '0':
				break;
			default:
				puts("invalid b option : 0 assumed");
			}
		aflg = vflag = 1;
		}
		argv++;
		argc--;
	}
	if (argc > 1 && **argv == '+') {
		aflg = 0;
		argv++;
		argc--;
	}
	if (argc>1) {
		p1 = *argv;
		p2 = savedfile;
		while (*p2++ = *p1++);
		--p2;
		while (*--p2 == '/');
		*++p2 = '\0';
		globp = "r";
		fflg++;
	}
	fendcore = sbrk(0);
	init();
	if ((signal(SIGINTR, 1) & 01) == 0)
		signal(SIGINTR, onintr);
	setexit();
	commands();
	unlink(tfname);
}

commands()
{
	int getfile(), gettty();
	register *a1, c;
	register char *p;
	int r;

	for (;;) {
	if (pflag) {
		pflag = 0;
		addr1 = addr2 = dot;
		goto print;
	}
	if(shflg && globp==0)
		write(1, &prompt, 1);
	addr1 = 0;
	addr2 = 0;
	do {
		addr1 = addr2;
		if ((a1 = address())==0) {
			c = getchar();
			break;
		}
		addr2 = a1;
		if ((c=getchar()) == ';') {
			c = ',';
			dot = a1;
		}
	} while (c==',');
	if (addr1==0)
		addr1 = addr2;
	switch(c) {

	case 'a':
		setdot();
		newline();
		append(gettty, addr2);
		if (dot != addr2) wflg++;
		continue;

	case 'c':
		delete();
		append(gettty, addr1-1);
		wflg++;
		continue;

	case 'd':
		delete();
		wflg++;
		continue;

	case 'e':
		setnoaddr();
		filename(c);
		if (wflg && aflg) ask("e?\n");
		init();
		addr2 = zero;
		fflg = 1;
		goto caseread;

	case 'f':
		setnoaddr();
		filename(c);
		puts(savedfile);
		continue;

	case 'G':
		globaln(1);
		continue;

	case 'g':
		global(1);
		continue;

	case 'h':
		setdot();
		newline();
		date();
		if((io = open(hfile,0))<0)
			error;
		ninbuf = 0;
		append(getfile, addr2);
		unlink(hfile);
		close(io);
		io = -1;
		wflg++;
		continue;

	case 'i':
		setdot();
		nonzero();
		newline();
		append(gettty, addr2-1);
		if (dot != (addr2-1)) wflg++;
		continue;

	case 'j':
		if (addr2==0) {
			addr1 = dot;
			addr2 = dot + 1;
		}
		setdot();
		newline();
		nonzero();
		join();
		continue;

	case 'k':
		if ((c = getchar()) < 'a' || c > 'z')
			error;
		newline();
		setdot();
		nonzero();
		names[c-'a'] = *addr2 & ~01;
		anymarks =| 01;
		continue;

	case 'm':
		move(0);
		wflg++;
		continue;

	case '\n':
		if (addr2==0)
			addr2 = dot+1;
		addr1 = addr2;
		goto print;

	case 'l':
		listf++;
	case 'p':
		newline();
	print:
		setdot();
		nonzero();
		a1 = addr1;
		do
			puts(getline(*a1++));
		while (a1 <= addr2);
		dot = addr2;
		listf = 0;
		continue;

	case 'Q':
		wflg = 0;
	case 'q':
		setnoaddr();
		newline();
		if (wflg && aflg) ask("q?\n");
		unlink(tfname);
		exit(0);

	case 'r':
		filename(c);
		if (!fflg) wflg++;
	caseread:
		if ((io = open(file, 0)) < 0) {
			lastc = '\n';
			error;
		}
		if (fflg) {
			fflg--;
			read(io,funny,LBSIZE);
			if(fspec(funny,&fss,0)<0) {
							fss.Ffill = 0;
							puts("bad fmt spec");
			}
			seek(io,0,0);
		}
		setall();
		ninbuf = 0;
		append(getfile, addr2);
		exfile();
		continue;

	case 's':
		setdot();
		nonzero();
		substitute(globp);
		wflg++;
		continue;

	case 't':
		move(1);
		wflg++;
		continue;

	case 'u':
		setdot();
		nonzero();
		newline();
		if ((*addr2&~01) != subnewa)
			error;
		*addr2 = subolda;
		wflg--;
		dot = addr2;
		continue;

	case 'V':
		globaln(0);
		continue;

	case 'v':
		global(0);
		continue;

	case 'w':
	case 'z':
		setall();
		nonzero();
		filename(c);
		if (rflg) {
			p = file;
			while (*p)
				if (*p++ == '/') error;
		}
		if (stat(file,stbuf) >= 0) {
			if ((stbuf[3] & 0377) == 1) {
				if ((close(open(file,1)))<0) error;
				mkfunny();
				if ((io=creat(funny,stbuf[2]&0777))<0)
					goto noperm;
				lflg++;
				chown(funny,stbuf[4]<<8 | ((stbuf[3]>>8)&0377));
				putfile();
				exfile();
				unlink(file);
				if (link(funny,file)) {
						puts("can't link: e4");
						error;
				}
				unlink(funny);
				sflg = lflg = wflg = 0;
				continue;
			}
			else {
				if (ustat(stbuf[0],&ubuf) == 0) {
					if (ubuf.s_tfree <= 200) {
						if (!sflg) {
							puts("space?: e2");
							putds(ubuf.s_tfree);
							puts(" blks left");
							sflg++;
							continue;
						}
					}
				}
			}
		}
	noperm:
		sflg = 0;
		if ((io = creat(file, 0644)) < 0)
			error;
		putfile();
		exfile();
		wflg = 0;
		continue;

	case '=':
		setall();
		newline();
		count[0] = 0;
		count[1] = (addr2-zero)&077777;
		putd();
		putchar('\n');
		continue;

	case '!':
		p = funny;
		while((*p++ = getchar()) != '\n');
		*p = 0;
		unix();
		continue;

	case EOF:
		return;

	case 'P':
		setnoaddr();
		newline();
		if (shflg) shflg = 0;
		else shflg++;
		continue;

	}
	error;
	}
}

address()
{
	register *a1, minus, c;
	int n, relerr;

	minus = 0;
	a1 = 0;
	for (;;) {
		c = getchar();
		if ('0'<=c && c<='9') {
			n = 0;
			do {
				n =* 10;
				n =+ c - '0';
			} while ((c = getchar())>='0' && c<='9');
			peekc = c;
			if (a1==0)
				a1 = zero;
			if (minus<0)
				n = -n;
			a1 =+ n;
			minus = 0;
			continue;
		}
		relerr = 0;
		if (a1 || minus)
			relerr++;
		switch(c) {
		case ' ':
		case '\t':
			continue;
	
		case '+':
			minus++;
			if (a1==0)
				a1 = dot;
			continue;

		case '-':
		case '^':
			minus--;
			if (a1==0)
				a1 = dot;
			continue;
	
		case '?':
		case '/':
			compile(c);
			a1 = dot;
			for (;;) {
				if (c=='/') {
					a1++;
					if (a1 > dol)
						a1 = zero;
				} else {
					a1--;
					if (a1 < zero)
						a1 = dol;
				}
				if (execute(0, a1))
					break;
				if (a1==dot)
					error;
			}
			break;
	
		case '$':
			a1 = dol;
			break;
	
		case '.':
			a1 = dot;
			break;

		case '\'':
			if ((c = getchar()) < 'a' || c > 'z')
				error;
			for (a1=zero; a1<=dol; a1++)
				if (names[c-'a'] == (*a1 & ~01))
					break;
			break;
	
		default:
			peekc = c;
			if (a1==0)
				return(0);
			a1 =+ minus;
			if (a1<zero || a1>dol)
				error;
			return(a1);
		}
		if (relerr)
			error;
	}
}

setdot()
{
	if (addr2 == 0)
		addr1 = addr2 = dot;
	if (addr1 > addr2)
		error;
}

setall()
{
	if (addr2==0) {
		addr1 = zero+1;
		addr2 = dol;
		if (dol==zero)
			addr1 = zero;
	}
	setdot();
}

setnoaddr()
{
	if (addr2)
		error;
}

nonzero()
{
	if (addr1<=zero || addr2>dol)
		error;
}

newline()
{
	register c;

	if ((c = getchar()) == '\n')
		return;
	if (c=='p' || c=='l') {
		pflag++;
		if (c=='l')
			listf++;
		if (getchar() == '\n')
			return;
	}
	error;
}

filename(comm)
{
	register char *p1, *p2;
	register c;

	count[1] = 0;
	c = getchar();
	if (c=='\n' || c==EOF) {
		p1 = savedfile;
		if (*p1==0 && comm!='f')
			error;
		p2 = file;
		while (*p2++ = *p1++);
		return;
	}
	if (c!=' ')
		error;
	while ((c = getchar()) == ' ');
	if (c=='\n')
		error;
	p1 = file;
	do {
		*p1++ = c;
		if (c==' ' || c==EOF)
			error;
	} while ((c = getchar()) != '\n');
	while (*--p1 == '/') ;
	*++p1 = 0;
	if (comm=='f' || ((!wflg || !aflg) && comm=='e') ||
	   (comm!='e' && savedfile[0]==0)) {
		p1 = savedfile;
		p2 = file;
		while (*p1++ = *p2++);
	}
}

exfile()
{
	close(io);
	io = -1;
	if (vflag) {
		putd();
		putchar('\n');
	}
}

onintr()
{
	signal(SIGINTR, onintr);
	putchar('\n');
	lastc = '\n';
	error;
}

errfunc()
{
	register c;

	listf = 0;
	puts("?");
	count[0] = 0;
	seek(0, 0, 2);
	pflag = 0;
	if (globp)
		lastc = '\n';
	globp = 0;
	peekc = lastc;
	while ((c = getchar()) != '\n' && c != EOF);
	if (io > 0) {
		close(io);
		io = -1;
	}
	reset();
}

getchar()
{
	if (lastc=peekc) {
		peekc = 0;
		return(lastc);
	}
	if (globp) {
		if ((lastc = *globp++) != 0)
			return(lastc);
		globp = 0;
		return(EOF);
	}
	if (read(0, &lastc, 1) <= 0)
		return(lastc = EOF);
	lastc =& 0177;
	return(lastc);
}

gettty()
{
	register c, gf;
	register char *p;

	p = linebuf;
	gf = globp;
	while ((c = getchar()) != '\n') {
		if (c==EOF) {
			if (gf)
				peekc = c;
			return(c);
		}
		if ((c =& 0177) == 0)
			continue;
		*p++ = c;
		if (p >= &linebuf[LBSIZE-2])
			error;
	}
	*p++ = 0;
	if (linebuf[0]=='.' && linebuf[1]==0)
		return(EOF);
	return(0);
}

getfile()
{
	register c;
	register char *lp, *fp;

	lp = linebuf;
	fp = nextip;
	do {
		if (--ninbuf < 0) {
			if ((ninbuf = read(io, genbuf, LBSIZE)-1) < 0)
				return(EOF);
			fp = genbuf;
		}
		if (lp >= &linebuf[LBSIZE])
			error;
		if ((*lp++ = c = *fp++ & 0177) == 0) {
			lp--;
			continue;
		}
		if (++count[1] == 0)
			++count[0];
	} while (c != '\n');
	*--lp = 0;
	nextip = fp;
	if (fss.Ffill && fss.Flim  && lenchk(linebuf,&fss)<0) {
		write(1,"line too long: lno = ",21);
		ccount = (++dot-zero)&077777;
		dot--;
		putds(ccount);
		putchar('\n');
	}
	return(0);
}

putfile()
{
	int *a1;
	register char *fp, *lp;
	register nib;

	nib = 512;
	fp = genbuf;
	a1 = addr1;
	do {
		lp = getline(*a1++);
		if (fss.Ffill && fss.Flim  && lenchk(lp,&fss)<0) {
			write(1,"line too long: lno = ",21);
			ccount = (--a1-zero)&077777;
			a1++;
			putds(ccount);
			putchar('\n');
		}
		for (;;) {
			if (--nib < 0) {
				fwrite(io, genbuf, fp-genbuf);
				nib = 511;
				fp = genbuf;
			}
			if (++count[1] == 0)
				++count[0];
			if ((*fp++ = *lp++) == 0) {
				fp[-1] = '\n';
				break;
			}
		}
	} while (a1 <= addr2);
	fwrite(io, genbuf, fp-genbuf);
}

fwrite(a,b,c)
{

	if(write(a,b,c) != c) {
		puts("no space: e1");
		if (lflg) {
			close(io); io = -1;
			lflg = 0;
			unlink(funny);
		}
		else
			exfile();
		error;
	}
}

append(f, a)
int (*f)();
{
	register *a1, *a2, *rdot;
	int nline, tl;
	struct { int integer; };

	nline = 0;
	dot = a;
	while ((*f)() == 0) {
		if (dol >= endcore) {
			if (sbrk(1024) == -1)
				error;
			endcore.integer =+ 1024;
		}
		tl = putline();
		nline++;
		a1 = ++dol;
		a2 = a1+1;
		rdot = ++dot;
		while (a1 > rdot)
			*--a2 = *--a1;
		*rdot = tl;
	}
	return(nline);
}

unix()
{
	register savint, pid, rpid;
	int retcode;

	setnoaddr();
	if ((pid = fork()) == 0) {
		signal(SIGHUP, onhup);
		signal(SIGQUIT, onquit);
		signal(SIGPIPE, onpipe);
		if (rflg) execl("/bin/rsh","rsh","-c",funny,0);
		else execl("/bin/sh", "sh", "-c", funny,0);
		exit(0);
	}
	savint = signal(SIGINTR, 1);
	while ((rpid = wait(&retcode)) != pid && rpid != -1);
	signal(SIGINTR, savint);
	if (aflg) puts("!");
}

rdelete(ad1, ad2)
int *ad1, *ad2;
{
	register *a1, *a2, *a3;

	a1 = ad1;
	a2 = ad2+1;
	a3 = dol;
	dol =- a2 - a1;
	do
		*a1++ = *a2++;
	while (a2 <= a3);
	a1 = ad1;
	if (a1 > dol)
		a1 = dol;
	dot = a1;
	wflg++;
}

join()
{
	register char *gp, *lp;
	register *a1;

	gp = genbuf;
	for (a1=addr1; a1<=addr2; a1++) {
		lp = getline(*a1);
		while (*gp = *lp++)
			if (gp++ >= &genbuf[LBSIZE-2])
				error;
	}
	lp = linebuf;
	gp = genbuf;
	while (*lp++ = *gp++);
	*addr1 = putline();
	if (addr1<addr2)
		rdelete(addr1+1, addr2);
	dot = addr1;
}

delete()
{

	setdot();
	newline();
	nonzero();
	rdelete(addr1,addr2);
}

getline(tl)
{
	register char *bp, *lp;
	register nl;

	lp = linebuf;
	bp = getblock(tl, READ);
	nl = nleft;
	tl =& ~offmsk;
	while (*lp++ = *bp++)
		if (--nl == 0) {
			bp = getblock(tl=+incrmt, READ);
			nl = nleft;
		}
	return(linebuf);
}

putline()
{
	register char *bp, *lp;
	register nl;
	int tl;

	lp = linebuf;
	tl = tline;
	bp = getblock(tl, WRITE);
	nl = nleft;
	tl =& ~offmsk;
	while (*bp = *lp++) {
		if (*bp++ == '\n') {
			*--bp = 0;
			linebp = lp;
			break;
		}
		if (--nl == 0) {
			bp = getblock(tl=+incrmt, WRITE);
			nl = nleft;
		}
	}
	nl = tline;
	tline =+ (((lp-linebuf)+bndry-1)>>shft)&077776;
	return(nl);
}

getblock(atl, iof)
{
	extern read(), write();
	register bno, off;
	
	bno = (atl>>offbts)&blkmsk;
	off = (atl<<shft)&lbtmsk;
	if (bno >= nmblks) {
		puts(TMPERR);
		error;
	}
	nleft = 512 - off;
	if (bno==iblock) {
		ichanged =| iof;
		return(ibuff+off);
	}
	if (bno==oblock)
		return(obuff+off);
	if (iof==READ) {
		if (ichanged)
			blkio(iblock, ibuff, write);
		ichanged = 0;
		iblock = bno;
		blkio(bno, ibuff, read);
		return(ibuff+off);
	}
	if (oblock>=0)
		blkio(oblock, obuff, write);
	oblock = bno;
	return(obuff+off);
}

blkio(b, buf, iofcn)
int (*iofcn)();
{
	seek(tfile, b, 3);
	if ((*iofcn)(tfile, buf, 512) != 512) {
		puts(TMPERR);
		error;
	}
}

init()
{
	register char *p;
	register pid, *markp;

	close(tfile);
	tline = 2;
	for (markp = names; markp < &names[26]; )
		*markp++ = 0;
	anymarks = 0;
	iblock = -1;
	oblock = -1;
	tfname = "/tmp/exxxxx";
	ichanged = 0;
	pid = getpid();
	for (p = &tfname[11]; p > &tfname[6];) {
		*--p = (pid&07) + '0';
		pid =>> 3;
	}
	close(creat(tfname, 0600));
	tfile = open(tfname, 2);
	brk(fendcore);
	dot = zero = dol = fendcore;
	endcore = fendcore - 2;
}

global(k)
{
	register char *gp;
	register c;
	register int *a1;
	char globuf[GBSIZE];

	if (globp)
		error;
	setall();
	nonzero();
	if ((c=getchar())=='\n')
		error;
	compile(c);
	gp = globuf;
	while ((c = getchar()) != '\n') {
		if (c==EOF)
			error;
		if (c=='\\') {
			c = getchar();
			if (c!='\n')
				*gp++ = '\\';
		}
		*gp++ = c;
		if (gp >= &globuf[GBSIZE-2])
			error;
	}
	*gp++ = '\n';
	*gp++ = 0;
	for (a1=zero; a1<=dol; a1++) {
		*a1 =& ~01;
		if (a1>=addr1 && a1<=addr2 && execute(0, a1)==k)
			*a1 =| 01;
	}
	for (a1=zero; a1<=dol; a1++) {
		if (*a1 & 01) {
			*a1 =& ~01;
			dot = a1;
			globp = globuf;
			commands();
			a1 = zero;
		}
	}
}

substitute(inglob)
{
	register gsubf, *a1, nl;
	int *markp;
	int getsub();

	gsubf = compsub();
	for (a1 = addr1; a1 <= addr2; a1++) {
		if (execute(0, a1)==0)
			continue;
		inglob =| 01;
		dosub();
		if (gsubf) {
			while (*loc2) {
				if (execute(1, 0)==0)
					break;
				dosub();
			}
		}
		subnewa = putline();
		*a1 =& ~01;
		if (anymarks) {
			*a1 =& ~01;
			for (markp = names; markp < &names[26]; markp++)
				if (*markp == *a1)
					*markp = subnewa;
		}
		subolda = *a1;
		*a1 = subnewa;
		nl = append(getsub, a1);
		a1 =+ nl;
		addr2 =+ nl;
	}
	if (inglob==0)
		error;
}

compsub()
{
	register seof, c;
	register char *p;
	int gsubf;

	if ((seof = getchar()) == '\n' || seof == ' ')
		error;
	compile(seof);
	p = rhsbuf;
	for (;;) {
		c = getchar();
		if (c=='\\')
			c = getchar() | 0200;
		if (c=='\n')
			error;
		if (c==seof)
			break;
		*p++ = c;
		if (p >= &rhsbuf[LBSIZE/2])
			error;
	}
	*p++ = 0;
	if ((peekc = getchar()) == 'g') {
		peekc = 0;
		newline();
		return(1);
	}
	newline();
	return(0);
}

getsub()
{
	register char *p1, *p2;

	p1 = linebuf;
	if ((p2 = linebp) == 0)
		return(EOF);
	while (*p1++ = *p2++);
	linebp = 0;
	return(0);
}

dosub()
{
	register char *lp, *sp, *rp;
	int c;

	lp = linebuf;
	sp = genbuf;
	rp = rhsbuf;
	while (lp < loc1)
		*sp++ = *lp++;
	while (c = *rp++) {
		if (c=='&') {
			sp = place(sp, loc1, loc2);
			continue;
		} else if (c<0 && (c =& 0177) >='1' && c < NBRA+'1') {
			sp = place(sp, braslist[c-'1'], braelist[c-'1']);
			continue;
		}
		*sp++ = c&0177;
		if (sp >= &genbuf[LBSIZE])
			error;
	}
	lp = loc2;
	loc2 = sp - genbuf + linebuf;
	while (*sp++ = *lp++)
		if (sp >= &genbuf[LBSIZE])
			error;
	lp = linebuf;
	sp = genbuf;
	while (*lp++ = *sp++);
}

place(asp, al1, al2)
{
	register char *sp, *l1, *l2;

	sp = asp;
	l1 = al1;
	l2 = al2;
	while (l1 < l2) {
		*sp++ = *l1++;
		if (sp >= &genbuf[LBSIZE])
			error;
	}
	return(sp);
}

move(cflag)
{
	register int *adt, *ad1, *ad2;
	int getcopy();

	setdot();
	nonzero();
	if ((adt = address())==0)
		error;
	newline();
	if (cflag) {
		ad1 = dol;
		append(getcopy, ad1++);
		ad2 = dol;
	} else {
		ad2 = addr2;
		for (ad1 = addr1; ad1 <= ad2;)
			*ad1++ =& ~01;
		ad1 = addr1;
	}
	ad2++;
	if (adt<ad1) {
		dot = adt + (ad2-ad1);
		if ((++adt)==ad1)
			return;
		reverse(adt, ad1);
		reverse(ad1, ad2);
		reverse(adt, ad2);
	} else if (adt >= ad2) {
		dot = adt++;
		reverse(ad1, ad2);
		reverse(ad2, adt);
		reverse(ad1, adt);
	} else
		error;
}

reverse(aa1, aa2)
{
	register int *a1, *a2, t;

	a1 = aa1;
	a2 = aa2;
	for (;;) {
		t = *--a2;
		if (a2 <= a1)
			return;
		*a2 = *a1;
		*a1++ = t;
	}
}

getcopy()
{
	if (addr1 > addr2)
		return(EOF);
	getline(*addr1++);
	return(0);
}

compile(aeof)
{
	register eof, c;
	register char *ep;
	char *lastep;
	char bracket[NBRA], *bracketp;
	int nbra,i;
	int cclcnt;

	sep = ep = expbuf;
	eof = aeof;
	bracketp = bracket;
	nbra = 0;
	if ((c = getchar()) == eof) {
		if (*ep==0)
			error;
		return;
	}
	circfl = 0;
	if (c=='^') {
		c = getchar();
		circfl++;
	}
	if ((c=='*') || (c=='+'))
		goto cerror;
	peekc = c;
	for (;;) {
		if (ep >= &expbuf[ESIZE])
			goto cerror;
		c = getchar();
		if (c==eof) {
			*ep++ = CEOF;
			return;
		}
		if ((c!='*') && (c!='+') && ((c!='\\') ||
		   ((peekc=getchar())!='{')))
			lastep = ep;
		switch (c) {

	case '\\':
			if ((c = getchar())=='(') {
				if (nbra >= NBRA)
					goto cerror;
				*bracketp++ = nbra;
				*ep++ = CBRA;
				*ep++ = nbra++;
				continue;
			}
			if (c == ')') {
				if (bracketp <= bracket)
					goto cerror;
				*ep++ = CKET;
				*ep++ = *--bracketp;
				continue;
			}
			if (c=='{') {
				if (sep==ep) goto cerror;
				*lastep =| RNGE;
				cflg = 0;
			nlim:
				c = getchar();
				i = 0;
				do {
					if ('0' <= c && c <= '9')
						i = (i*10+(c-'0'));
					else goto cerror;
				} while (((c=getchar()) != '\\') && (c != ','));
				if (i>255)  goto cerror;
				*ep++ = i;
				if (c==',') {
					if (cflg++) goto cerror;
					if((c=getchar())=='\\')
						*ep++ = -1;
					else {
						peekc = c;
						goto nlim;
					}
				}
				if(getchar() != '}') goto cerror;
				if (!cflg) *ep++ = i;
				else if ((ep[-1]&0377)<(ep[-2]&0377)) goto cerror;
				continue;
			}
			*ep++ = CCHR;
			if (c=='\n')
				goto cerror;
			*ep++ = c;
			continue;

		case '.':
			*ep++ = CDOT;
			continue;

		case '\n':
			goto cerror;

		case '+':
			if (*lastep==CBRA || *lastep==CKET)
				error;
			*lastep =| PLUS;
			continue;

		case '*':
			if (*lastep==CBRA || *lastep==CKET)
				error;
			*lastep =| STAR;
			continue;

		case '$':
			if ((peekc=getchar()) != eof)
				goto defchar;
			*ep++ = CDOL;
			continue;

		case '[':
			*ep++ = CCL;
			*ep++ = 0;
			cclcnt = 1;
			if ((c=getchar()) == '^') {
				c = getchar();
				ep[-2] = NCCL;
			}
			do {
				if (c=='\n')
					goto cerror;
				if ((c=='-') && (cclcnt>1) && ((peekc=getchar())!=']')){
					*ep++ = ep[-1];
					ep[-2] = MINUS;
					cclcnt++;
					continue;
				}
				*ep++ = c;
				cclcnt++;
				if (ep >= &expbuf[ESIZE])
					goto cerror;
			} while ((c = getchar()) != ']');
			lastep[1] = cclcnt;
			continue;

		defchar:
		default:
			*ep++ = CCHR;
			*ep++ = c;
		}
	}
   cerror:
	expbuf[0] = 0;
	error;
}

execute(gf, addr)
int *addr;
{
	register char *p1, *p2, c;

	if (gf) {
		if (circfl)
			return(0);
		p1 = linebuf;
		p2 = genbuf;
		while (*p1++ = *p2++);
		locs = p1 = loc2;
	} else {
		if (addr==zero)
			return(0);
		p1 = getline(*addr);
		locs = 0;
	}
	p2 = expbuf;
	if (circfl) {
		loc1 = p1;
		return(advance(p1, p2));
	}
	/* fast check for first character */
	if (*p2==CCHR) {
		c = p2[1];
		do {
			if (*p1!=c)
				continue;
			if (advance(p1, p2)) {
				loc1 = p1;
				return(1);
			}
		} while (*p1++);
		return(0);
	}
	/* regular algorithm */
	do {
		if (advance(p1, p2)) {
			loc1 = p1;
			return(1);
		}
	} while (*p1++);
	return(0);
}

advance(alp, aep)
{
	register char *lp, *ep, *curlp;
	char *nextep;

	lp = alp;
	ep = aep;
	for (;;) switch (*ep++) {

	case CCHR:
		if (*ep++ == *lp++)
			continue;
		return(0);

	case CDOT:
		if (*lp++)
			continue;
		return(0);

	case CDOL:
		if (*lp==0)
			continue;
		return(0);

	case CEOF:
		loc2 = lp;
		return(1);

	case CCL:
		if (cclass(ep, *lp++, 1)) {
			ep =+ *ep;
			continue;
		}
		return(0);

	case NCCL:
		if (cclass(ep, *lp++, 0)) {
			ep =+ *ep;
			continue;
		}
		return(0);

	case CBRA:
		braslist[*ep++] = lp;
		continue;

	case CKET:
		braelist[*ep++] = lp;
		continue;

	case CCHR|RNGE:
		sep = ep++;
		getrnge(&lcnt,&dcnt,ep);
		while(lcnt--)
			if (*lp++!=*sep) return(0);
		curlp = lp;
		while(dcnt--)
			if(*lp++!=*sep) break;
		if (dcnt < 0) lp++;
		ep =+ 2;
		goto star;

	case CDOT|RNGE:
		getrnge(&lcnt,&dcnt,ep);
		while(lcnt--)
			if(*lp++ == '\0') return(0);
		curlp = lp;
		while(dcnt--)
			if (*lp++ == '\0') break;
		if (dcnt < 0) lp++;
		ep =+ 2;
		goto star;
	case CCL|RNGE:
	case NCCL|RNGE:
		getrnge(&lcnt,&dcnt,(ep + (*ep&0377)));
		while(lcnt--)
			if(!cclass(ep,*lp++,ep[-1]==(CCL|RNGE))) return(0);
		curlp = lp;
		while(dcnt--)
			if(!cclass(ep,*lp++,ep[-1]==(CCL|RNGE))) break;
		if (dcnt < 0) lp++;
		ep =+ (*ep + 2);
		goto star;
	case CDOT|PLUS:
		if (*lp++ == '\0') return(0);


	case CDOT|STAR:
		curlp = lp;
		while (*lp++);
		goto star;

	case CCHR|PLUS:
		if (*lp++ != *ep) return(0);

	case CCHR|STAR:
		curlp = lp;
		while (*lp++ == *ep);
		ep++;
		goto star;
	case CCL|PLUS:
	case NCCL|PLUS:
		if (!cclass(ep,*lp++,(ep[-1]==(CCL|PLUS)))) return(0);

	case CCL|STAR:
	case NCCL|STAR:
		curlp = lp;
		while (cclass(ep, *lp++, ((ep[-1]==(CCL|STAR)) || (ep[-1]==(CCL|PLUS)))));
		ep =+ *ep;
		goto star;

	star:
		do {
			lp--;
			if (lp==locs)
				break;
			if (advance(lp, ep))
				return(1);
		} while (lp > curlp);
		return(0);

	default:
		error;
	}
}

cclass(aset, ac, af)
{
	register char *set, c;
	register n;

	set = aset;
	if ((c = ac) == 0)
		return(0);
	n = *set++;
	while (--n) {
		if (*set==MINUS) {
			if ((set[2] - set[1]) < 0) return(0);
			if (*++set <= c) {
				if (c <= *++set)
					return(af);
			}
			else ++set;
			++set;
			n =- 2;
			continue;
		}
		if (*set++ == c)
			return(af);
	}
	return(!af);
}

putd()
{
	register char *sp;

	sp = locv(count[0],count[1]);
	col = 0;
	while(*sp)
		putchar(*sp++);
	count[0] = count[1] = 0;
}
putds(dcount) 
{
	register r, icount;
	icount = dcount / 10;
	r = dcount - (10 * icount);
	if (icount) putds(icount);
	putchar(r + '0');
}
puts(as)
{
	register char *sp;
	int sz,i;
	if (fss.Ffill &&  !listf) {
		if ((i = expnd(as,funny,&sz,&fss)) == -1)  {
			write(1,funny,fss.Flim); putchar('\n');
			write(1,"too long",8);
		}
		else
			write(1,funny,sz);
		putchar('\n');
		if (i == -2) write(1,"tab count\n",10);
		return(0);
	}

	sp = as;
	col = 0;
	while (*sp)
		putchar(*sp++);
	putchar('\n');
}

char	Line[70];
char	*linp	Line;

putchar(ac)
{
	register char *lp;
	register c;

	lp = linp;
	c = ac;
	if (listf) {
		col++;
		if (col >= 72) {
			col = 0;
			*lp++ = '\\';
			*lp++ = '\n';
		}
		if (c=='\t') {
			c = '>';
			goto esc;
		}
		if (c=='\b') {
			c = '<';
		esc:
			*lp++ = '-';
			*lp++ = '\b';
			*lp++ = c;
			goto out;
		}
		if (c<' ' && c!= '\n') {
			*lp++ = '\\';
			*lp++ = (c>>3)+'0';
			*lp++ = (c&07)+'0';
			col =+ 2;
			goto out;
		}
	}
	*lp++ = c;
out:
	if(c == '\n' || lp >= &Line[64]) {
		linp = Line;
		if (write(1, Line, lp-Line) < 0) {
			unlink(tfname);
			exit(1);
		}
		return;
	}
	linp = lp;
}

date()
{
	register pid, rpid;
	int retcode;

	if ((pid = fork()) == 0) {
		close(1);
		creat(hfile,0600);
		execl("/bin/date","date",0);
		exit(0);
	}
	while ((rpid = wait(&retcode)) != pid && rpid != -1);
}
ask(qe) {
	register char *askq;
	register c;

	listf = 0;
	askq = qe;
	write(1,askq,3);
	wflg = 0;
	count[0] = 0;
	seek(0, 0, 2);
	pflag = 0;
	if (globp)
		lastc = '\n';
	globp = 0;
	peekc = lastc;
	while ((c = getchar()) != '\n' && c != EOF);
	reset();
}
mkfunny()
{
	register char *p, *p1, *p2;

	p2 = p1 = funny;
	p = file;
	while (*p1++ = *p)
		if (*p++ == '/') p2 = p1;
	p1 = &tfname[6];
	while (p1 <= &tfname[11])
		*++p2 = *p1++;
}


char *fsp, fsprtn;


fspec(line,f,up)
char line[];
struct Fspec *f;
int up;
{
	int arg[3];
	register int havespec, n;

	if(!up) clear(f);

	havespec = fsprtn = 0;
	for(fsp=line; *fsp && *fsp != '\n'; fsp++)
		switch(*fsp) {

			case '<':	if(havespec) return(-1);
					if(*(fsp+1) == ':') {
						havespec = 1;
						clear(f);
						if(!gtty(1,arg) && arg[2]&02)
						  f->Ffill = 1;
						fsp++;
						continue;
					}

			case ' ':	continue;

			case 's':	if(havespec && (n=numb()) >= 0)
						f->Flim = n;
					continue;

			case 't':	if(havespec) targ(f);
					continue;

			case 'd':	continue;

			case 'm':	if(havespec)  n = numb();
					continue;

			case 'e':	continue;
			case ':':	if(!havespec) continue;
					if(*(fsp+1) != '>') fsprtn = -1;
					return(fsprtn);

			default:	if(!havespec) continue;
					return(-1);
		}
	return(1);
}


numb()
{
	register int n;

	n = 0;
	while(*++fsp >= '0' && *fsp <= '9')
		n = 10*n + *fsp-'0';
	fsp--;
	return(n);
}


targ(f)
struct Fspec *f;
{
	register int n;

	if(*++fsp == '-') {
		if(*(fsp+1) >= '0' && *(fsp+1) <= '9') tincr(numb(),f);
		else tstd(f);
		return;
	}
	if(*fsp >= '0' && *fsp <= '9') {
		tlist(f);
		return;
	}
	fsprtn = -1;
	fsp--;
	return;
}


tincr(n,f)
int n;
struct Fspec *f;
{
	register int l, i;

	l = 1;
	for(i=0; i<20; i++)
		f->Ftabs[i] = l =+ n;
	f->Ftabs[i] = 0;
}


tstd(f)
struct Fspec *f;
{
	char std[3];

	std[0] = *++fsp;
	if (*(fsp+1) >= '0' && *(fsp+1) <= '9')  {
						std[1] = *++fsp;
						std[2] = '\0';
	}
	else std[1] = '\0';
	fsprtn = stdtab(std,f->Ftabs);
	return;
}


tlist(f)
struct Fspec *f;
{
	register int n, last, i;

	fsp--;
	last = i = 0;

	do {
		if((n=numb()) <= last || i >= 20) {
			fsprtn = -1;
			return;
		}
		f->Ftabs[i++] = last = n;
	} while(*++fsp == ',');

	f->Ftabs[i] = 0;
	fsp--;
}


expnd(line,buf,sz,f)
char line[], buf[];
int *sz;
struct Fspec *f;
{
	register char *l, *t;
	register int b;

	l = line - 1;
	b = 1;
	t = f->Ftabs;
	fsprtn = 0;

	while(*++l && *l != '\n' && b < 511) {
		if(*l == '\t') {
			while(*t && b >= *t) t++;
			if (*t == 0) fsprtn = -2;
			do buf[b-1] = ' '; while(++b < *t);
		}
		else buf[b++ - 1] = *l;
	}

	buf[b] = '\0';
	*sz = b;
	if(*l != '\0' && *l != '\n') {
		buf[b-1] = '\n';
		return(-1);
	}
	buf[b-1] = *l;
	if(f->Flim && b-1 > f->Flim) return(-1);
	return(fsprtn);
}


clear(f)
struct Fspec *f;
{
	f->Ftabs[0] = f->Fdel = f->Flim = f->Fmov = f->Ffill = 0;
}
lenchk(line,f)
char line[];
struct Fspec *f;
{
	register char *l, *t;
	register int b;

	l = line - 1;
	b = 1;
	t = f->Ftabs;

	while(*++l && *l != '\n' && b < 511) {
		if(*l == '\t') {
			while(*t && b >= *t) t++;
			while(++b < *t);
		}
		else b++;
	}

	if((*l!='\0'&&*l!='\n') || (f->Flim&&b-1>f->Flim))
		return(-1);
	return(0);
}
#define NTABS 21

/*	stdtabs: standard tabs table
	format: option code letter(s), null, tabs, null */
char stdtabs[] {
'a',	0,1,10,16,36,72,0,			/* IBM 370 Assembler */
'a','2',0,1,10,16,40,72,0,			/* IBM Assembler alternative*/
'c',	0,1,8,12,16,20,55,0,			/* COBOL, normal */
'c','2',0,1,6,10,14,49,0,			/* COBOL, crunched*/
'c','3',0,1,6,10,14,18,22,26,30,34,38,42,46,50,54,58,62,67,0,
'f',	0,1,7,11,15,19,23,0,			/* FORTRAN */
'p',	0,1,5,9,13,17,21,25,29,33,37,41,45,49,53,57,61,0, /* PL/I */
's',	0,1,10,55,0,				/* SNOBOL */
'u',	0,1,12,20,44,0,				/* UNIVAC ASM */
0};
/*	stdtab: return tab list for any "canned" tab option.
	entry: option points to null-terminated option string
		tabvect points to vector to be filled in
	exit: return(0) if legal, tabvect filled, ending with zero
		return(-1) if unknown option
*/

stdtab(option,tabvect)
char option[], tabvect[NTABS];
{
	char *scan;
	tabvect[0] = 0;
	scan = stdtabs;
	while (*scan)
		{
		if (strequal(&scan,option))
			{strcopy(scan,tabvect);break;}
		else while(*scan++);	/* skip over tab specs */
		}
/*	later: look up code in /etc/something */
	return(tabvect[0]?0:-1);
}

/*	strequal: checks strings for equality
	entry: scan1 points to scan pointer, str points to string
	exit: return(1) if equal, return(0) if not
		*scan1 is advanced to next nonzero byte after null
*/
strequal(scan1,str)
char **scan1, *str;
{
	char c, *scan;
	scan = *scan1;
	while ((c = *scan++) == *str && c) str++;
	*scan1 = scan;
	if (c == 0 && *str == 0) return(1);
	if (c) while(*scan++);
	*scan1 = scan;
	return(0);
}

/*	strcopy: copy source to destination */

strcopy(source,dest)
char *source, *dest;
{
	while (*dest++ = *source++);
	return;
}
getrnge(i,j,k) int *i,*j; char *k;
{
	*i = (*k++&0377);
	if (*k==-1) *j = 20000;
	else *j = ((*k&0377) - *i);
	return(1);
}
globaln(k)
{
	register char *gp;
	register c;
	register int *a1;
	int  nfirst;
	char globuf[GBSIZE];

	if (globp)
		error;
	setall();
	nonzero();
	if ((c=getchar())=='\n')
		error;
	compile(c);
	for (a1=zero; a1<=dol; a1++) {
		*a1 =& ~01;
		if (a1>=addr1 && a1<=addr2 && execute(0, a1)==k)
			*a1 =| 01;
	}
	nfirst = 0;
	if ((c = getchar()) != '\n')
		error;
	for (a1=zero; a1<=dol; a1++) {
		if (*a1 & 01) {
			*a1 =& ~01;
			dot = a1;
			puts(getline(*a1));
			if ((c=getchar()) == EOF)
				error;
			c =& 0177;
			if (c == '\n') {
				a1 = zero;
				continue;
			}
			if (c != '&') {
				gp = globuf;
				*gp++ = c;
				while ((c = getchar()) != '\n') {
					if (c=='\\') {
						c = getchar();
						if (c!='\n')
							*gp++ = '\\';
					}
					*gp++ = c;
					if (gp >= &globuf[GBSIZE-2])
						error;
				}
				*gp++ = '\n';
				*gp++ = 0;
				nfirst = 1;
			}
			else
				if ((c=getchar()) != '\n')
					error;
			globp = globuf;
			if (nfirst) commands();
			else error;
			globp = 0;
			a1 = zero;
		}
	}
}
