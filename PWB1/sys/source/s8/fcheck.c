char	*dargv[] {
	"/dev/rrp0",
	0,
};

char	**defarg	{ dargv };

#define	CLEAR	021
#define	DSTATE	020
#define	FSTATE	001

#define	NINOB	320
#define	ALLINUM	inum=1;inum<=imax;inum++

#define	ALLOC	(inode->flags&0100000)
#define	LARGE	(inode->flags&010000)
#define	DIR	((inode->flags&060000)==040000)
#define	NOTSPL	((inode->flags&020000)==0)

#define	MAXDUP	100

struct SB {
	int	isize;
	int	fsize;
	int	nfree;
	int	free[100];
	int	ninode;
	int	finode[100];
	char	flock;
	char	ilock;
	char	fmod;
	int	time[2];
	int	fill[50];
} sb;

struct INODE {
	int	flags;
	char	nlinks;
	char	uid;
	char	gid;
	char	size0;
	int	size1;
	int	addr[8];
	int	actime[2];
	int	modtime[2];
} *inode;

#define	NIPB	(512/sizeof(*inode))

struct DE {
	int	dnum;
	char	dname[14];
};

char
	*bbit, 
	*abbit,
	*state, 
	*lc, 
	pathname[200], 
	*pp, 
	*name, 
	sflag, 
	nflag, 
	yflag, 
;

unsigned
	dsize,
	fmin,
	fmax
;

int
	firsti,
	lasti,
	n_free, 
	n_blks, 
	n_files, 
	inum, 
	diskr, 
	diskw, 
	cc, 
	dups[MAXDUP], 
	*dc, 
	*el, 
	imax, 
	mod, 
	buf[256], 
	lbuf[256],
	nbad,
;
int	pass1(), pass2(), pass3(), pass4();
struct INODE ibuf[NINOB];

main(argc, argv)
int	argc;
char	*argv[];
{
	register c;

	while (argc>1 && *argv[1]=='-') {
		while(c = *++argv[1])
		switch(c) {
		case 's':
		case 'S':
			sflag = 1;
			continue;
		case 'n':
		case 'N':
			nflag = 1;
			continue;
		case 'y':
		case 'Y':
			yflag = 1;
			continue;
		default:
			printf("%c option?\n", c);
			goto fin;
		}
		argv++;
		argc--;
	}
	if (argc>1)
		while (argc>1) {
			check(argv[1]);
			argc--;
			argv++;
		}
	else while (*defarg)
		check(*defarg++);
fin:
	return(0);
}

check(dev)
char *dev;
{
	register int *ap, *bp;
	unsigned blk;
	int a, b;

/*	Initialization		*/

	if ((diskr = open(dev, 0)) == -1) {
		printf("CAN NOT OPEN %s\n", dev);
		return;
	}
	printf("%s", dev);
	if ((diskw = open(dev, 1)) == -1) {
		nflag = 1;
		printf(" (NO WRITE)");
	}
	printf(":\n");
	sync();
	bread(&sb, 1, 512);
	imax = 16 * sb.isize;
	fmin = 2+sb.isize;
	fmax = sb.fsize;
	bbit = getcore((fmax>>3)+2);
	state = getcore(imax/4+4);
	lc = getcore(imax+1);
	firsti = lasti = -1;
	el = dc = dups;
	n_files = n_blks = n_free = *dc = 0;
	pp = pathname;
	pathname[0] = '/';
	pathname[1] = 0;

	printf("Phase 1 - Check Blocks\n");
	for(ALLINUM) {
		stat(NINOB);
		if (ALLOC) {
			nbad = 0;
			n_files++;
			set(DIR? DSTATE:FSTATE);
			if ((lc[inum]=inode->nlinks)==0)
				set(CLEAR);
			forallblocks(pass1);
			if (nbad>5)
				printf("Total of %u bad/dup in I=%u\n", nbad, inum);
		}
	}

	setexit();
	if (dc==dups)
		goto phase3;
	printf("Phase 2 - Rescan for more DUPS\n");
	for(ALLINUM)
	if (get()) {
		stat(NINOB);
		forallblocks(pass2);
	}

phase3:
	printf("Phase 3 - Check Pathnames\n");
	inum = 1;
	lc[1]++;
	descend();

	printf("Phase 4 - Check Reference Counts\n");
	for(ALLINUM)
	switch(get()) {
	case FSTATE:
		if (lc[inum])
			adj();
		continue;
	case DSTATE:
	case CLEAR:
		clri();
	}

	printf("Phase 5 - Check Free List\n");
	inum = 1;
	getblk(1, buf);
	free(lc);
	free(state);
	abbit = getcore(b = (fmax>>3)+2);
	for (a=0; a<b; a++)
		abbit[a] = bbit[a];
	if (sflag)
		goto salvage;
	if (sb.nfree<=0 || sb.nfree>100) {
		sflag = 1;
		goto salvage;
	}
	while(blk = sb.free[--sb.nfree]) {
		if (sb.nfree==0)
			bread(&sb.nfree, blk, sizeof(sb.free)+sizeof(sb.nfree));
		if (sb.nfree<=0 || sb.nfree>100
		 || blk<fmin || blk>=fmax
		 || (abbit[a=blk>>3]&(b=1<<(blk&07)))) {
			printf("BAD FREE LIST-- SALVAGE?");
			sflag = reply();
			goto salvage;
		}
		abbit[a] =| b;
		n_free++;
	}
	if ((n_blks+n_free)!=(fmax-fmin)) {
		printf("%u MISSING-- SALVAGE?", fmax-fmin-n_blks-n_free);
		sflag = reply();
	}

salvage:
	if (sflag==0)
		goto statistic;
	if (nflag) {
		printf("Needs salvage, but can't write.\n");
		goto statistic;
	}
	printf("Phase 6 - Salvage Free List\n");
	n_free = sb.ninode = 0;
	sb.nfree = 1;
	for (a=0; a<100; a++)
		sb.free[a] = 0;
	for(blk=fmax-1;blk>=fmin;--blk)
	if ((bbit[blk>>3]&(1<<(blk&07)))==0) {
		if (sb.nfree==100) {
			bwrite(&sb.nfree, blk, sizeof(sb.free)+sizeof(sb.nfree));
			sb.nfree = 0;
		}
		sb.free[sb.nfree++] = blk;
		n_free++;
	}
	bwrite(&sb, 1, 512);
	mod = 1;

statistic:
	printf("%5l files    %5l blocks     %5l free\n", 
	n_files, n_blks, n_free);
	close(diskr);
	close(diskw);
	free(bbit);
	free(abbit);
}

forallblocks(f)
int (*f)();
{
	register unsigned *ap, *ip, *iip;

	if (NOTSPL) for (ap = inode->addr; ap < &inode->addr[8]; ap++) {
		if (*ap==0)
			continue;
		(*f)(*ap);
		if (LARGE) {
			getblk(*ap, buf);
			for (ip=buf; ip<&buf[256]; ip++)
				if (*ip) {
					(*f)(*ip);
					if (ap == &inode->addr[7]) {
						getblk(*ip, lbuf);
						for (iip=lbuf; iip<&lbuf[256]; iip++)
							if (*iip)
								(*f)(*iip);
					}
				}
		}
	}
}

pass1(blk)
unsigned blk;
{
	register int a, b, *ip;

	if (blk<fmin || blk>=fmax) {
		if (++nbad > 5)
			return(0);
		blkerr("BAD", blk);
		return(0);
	}
	if (bbit[a=blk>>3]&(b=1<<(blk&07))) {
		if (++nbad > 5)
			return(0);
		blkerr("DUP", blk);
		if (el > &dups[MAXDUP]) {
			printf("\tEXCESSIVE DUPS   EXIT?");
			if (reply())
				exit();
			else
				return(1);
		}
		ip = dups;
		while(ip<dc)
		if (*ip++ == blk) {
			*el++ = blk;
			goto ret;
		}
		*el++ = *dc;
		*dc++ = blk;
	} else {
		bbit[a] =| b;
		n_blks++;
	}
ret:
	return(1);
}

pass2(blk)
unsigned blk;
{
	register int *ip;

	if (blk<fmin || blk>=fmax) return(0);
	ip = dups;
	while(ip<dc)
	if (*ip++ == blk) {
		blkerr("DUP", blk);
		*--ip = *--dc;
		*dc = blk;
		if (dc==dups) reset(); else break;
	}
	return(1);
}

stat(ntoget)
{

	if (inum<firsti || inum>=lasti) {
		firsti = ((inum-1)/NIPB)*NIPB + 1;
		bread(ibuf, (firsti+31)/NIPB, ntoget*sizeof(ibuf[0]));
		lasti = firsti+ntoget;
	}
	return(inode = &ibuf[inum-firsti]);
}

iblock(blk, func)
unsigned blk;
int (*func)();
{
	register int *ap;

	ap = buf;
	getblk(blk, buf);
	do {
		if (*ap)
			(*func)(*ap);
	} while(++ap<&buf[256]);
}

blkerr(s, blk)
unsigned blk;
char	*s;
{
 	printf("%15l %-15s  I = %l\n", blk, s, inum);
	set(CLEAR);
}

clri()
{
	register int *ap;

	stat(NIPB);
	printf("%15s %-15sI = %5l\tCLEAR?", 
	((inode->nlinks==0)||(get()!=CLEAR))?"UNREFERENCED":"BAD/DUP", 
	DIR?"DIRECTORY":"FILE", inum);
	if (reply()) {
		n_files--;
		forallblocks(pass4);
		for (ap = inode; ap <&inode[1];)
			*ap++ = 0;
		iwrite();
	}
}

pass4(blk)
unsigned blk;
{
	register int a, b, *ip;

	if (blk<fmin || blk>=fmax)
		return;
	if (bbit[a=blk>>3]&(b=1<<(blk&07))) {
		ip = dups;
		while(ip<el)
		if (*ip++ == blk) {
			*--ip = *--el;
			return;
		}
		bbit[a] =& ~ b;
		n_blks--;
	}
}

adj()
{
	stat(NIPB);
	if (inode->nlinks==lc[inum])
		clri();
	else {
		printf("%15s %-15sI = %5l\tADJUST?", 
		"LINK COUNT", DIR?"DIRECTORY":"FILE", inum);
		if (reply()) {
			inode->nlinks =- lc[inum];
			iwrite();
		}
	}
}

descend()
{
	register int *ip, *ap;
	int a[8];
	extern int pass3();
	char *lname;
	int g;
	unsigned sdsize;

	if (inum>imax)
		return(direrr("I OUT OF RANGE"));
again:
	g = get();
	switch(g) {
	case DSTATE:
		set(FSTATE);
		lc[inum]--;
		ip = &stat(NIPB)->addr[0];
		sdsize = dsize;
		dsize = inode->size1;
		for(ap=a;ap<&a[8];)
			*ap++ = *ip++;
		*pp++ = '/';
		lname = name;
		name = pp;
		if (LARGE) {
			for(ap=a;ap<&a[8];ap++)
				if (*ap)
					iblock(*ap, &pass3);
		} else {
			for(ap=a;ap<&a[8];ap++)
				if (*ap)
					pass3(*ap);
		}
		dsize = sdsize;
		name = lname;
		*--pp = 0;
		return(0);
	case FSTATE:
		lc[inum]--;
		return(0);
	case 0:
		return(direrr("UNALLOCATED"));
	case CLEAR:
		if (direrr("DUP/BAD"))
			return(1);
		stat(NIPB);
		set(DIR? DSTATE: FSTATE);
		goto again;
	}
}

pass3(blk)
unsigned blk;
{
	register struct DE *dp;
	register char *c;
	int p3buf[256];

	dp = p3buf;
	getblk(blk, p3buf);
	do {
		if (dsize==0)
			return;
		dsize =- sizeof(*dp);
		if (inum = dp->dnum) {
			c = &dp->dname[0];
			while((*pp = *c++) && pp++ && c<&dp->dname[14]);
			*pp = 0;
			if (descend()) {
				dp->dnum = 0;
				if (nflag==0) {
					bwrite(p3buf, blk, 512);
				}
			}
			pp = name;
		}
	} while(++dp<&p3buf[256]);
}

direrr(s)
char	*s;
{
	printf("%15s   I = %-5l%s\tREMOVE?", s, inum, pathname);
	return(reply());
}

reply()
{
	register c, d;

	if (nflag) {
		printf(" no\n");
		return(0);
	}
	if (yflag) {
		printf(" yes\n");
		return(1);
	}
	do
		c = getchar();
	while(c == ' ' || c == '\t');
	d = c;
	while (d!='\n' && d>0)
		d = getchar();
	if (c == 'y')
		return(1);
	else
		return(0);
}

putchar(c)
char	c;
{
	if (c) write(1, &c, 1);
}

getchar()
{
	char c;

	if (read(0, &c, 1) <= 0)
		c = -1;
	return(c);
}

rwerr(s, b)
char	*s, *b;
{
	printf("\nCAN NOT %s: \tBLOCK %5l\tEXIT?", s, b);
	if (nflag || reply()) {
		printf("\n\n");
		exit();
	}
}

getblk(blk, bf)
char *blk, *bf;
{
	static char *cb, *cbf;

	if (blk==cb && cbf==bf)
		return;
	bread(cbf=bf, cb=blk, 512);
}

char	m[]	{ CLEAR, CLEAR<<1, CLEAR<<2 , CLEAR<<3};

set(s)
{
	register char *sp;
	register mi;

	mi = inum;
	sp = &state[mi>>2];
	mi =& 03;
	*sp =& ~m[mi];
	*sp =| s<<mi;
}

get()
{
	register mi;
	register char *sp;

	mi = inum;
	sp = &state[mi>>2];
	mi =& 03;
	return((*sp >> mi) & CLEAR);
}

bread(buf, blk, count)
char *buf;
{
	if (seek(diskr, blk, 3)<0)
		rwerr("SEEK", blk);
	if (read(diskr, buf, count) != count)
		rwerr("READ", blk);
}

bwrite(buf, blk, count)
{
	if (seek(diskw, blk, 3) < 0)
		rwerr("SEEK", blk);
	if (write(diskw, buf, count) != count)
		rwerr("WRITE", blk);
	mod = 1;
}

iwrite()
{
	register n;
	n = (lasti-firsti)*sizeof(*inode);
	bwrite(ibuf, (firsti+31)/NIPB, n);
	mod = 1;
}

getcore(n)
register unsigned n;
{
	register char *p, *p1;

	p = p1 = alloc(n);
	if (p == -1) {
		printf("Can't get enough core\n");
		exit(1);
	}
	do {
		*p++ = 0;
	} while (--n);
	return(p1);
}
