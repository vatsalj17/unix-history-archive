#
/*
 * list file or directory
 */


#include	<local-system>
#ifdef	AUSAM
#include	<passwd.h>
#endif

struct {
	int	fdes;
	int	nleft;
	char	*nextc;
	char	buff[512];
} inf;

struct ibuf {
	int	idev;
	int	inum;
	int	iflags;
#ifdef	AUSAM
	unsigned iuid;
#endif
	char	inl;
#ifndef	AUSAM
	char	iuid;
	char	igid;
#endif
	char	isize0;
	int	isize;
	int	iaddr[8];
#ifndef VN6_ORIG
	long	iatime;
	long	imtime;
#endif
#ifdef VN6_ORIG
	char	*iatime[2];
	char	*imtime[2];
#endif
};

struct lbuf {
	char	lname[15];
	int	lnum;
	int	lflags;
#ifdef	AUSAM
	unsigned luid;
#endif
	char	lnl;
#ifndef	AUSAM
	char	luid;
	char	lgid;
#endif
	char	lsize0;
	int	lsize;
#ifndef VN6_ORIG
	long	lmtime;
#endif
#ifdef VN6_ORIG
	char	*lmtime[2];
#endif
};

struct lbufx {
	char	*namep;
};

int	aflg, dflg, lflg, sflg, tflg, uflg, iflg, fflg;
#ifndef	AUSAM
int	gflg;
#endif
#ifdef	AUSAM
int	nflg;
#endif
int	fout;
int	rflg	1;
#ifndef VN6_ORIG
long	year;
#endif
#ifdef VN6_ORIG
char	*year;
#endif
int	flags;
#ifndef	AUSAM
int	uidfil	-1;
#endif
int	lastuid	-1;
char	tbuf[16];
int	tblocks;
int	statreq;
#ifndef VN6_ORIG
extern struct lbuf end[];
struct	lbuf	*lastp	end;
struct	lbuf	*rlastp	end;
#endif
#ifdef VN6_ORIG
struct	lbuf	*lastp	&end;
struct	lbuf	*rlastp	&end;
#endif
char	*dotp	"";
#ifndef VN6_ORIG
long	time();
#endif

#define	IFMT	060000
#define	DIR	0100000
#define	CHR	020000
#define	BLK	040000
#define	ISARG	01000
#define	LARGE	010000
#define	STXT	010000
#define	SUID	04000
#ifndef	AUSAM
#define	SGID	02000
#endif
#ifdef	AUSAM
#define	SLOK	02000
#endif
#define	ROWN	0400
#define	WOWN	0200
#define	XOWN	0100
#ifndef	AUSAM
#define	RGRP	040
#define	WGRP	020
#define	XGRP	010
#else	AUSAM
#define	AUTOLOK	010
#endif	AUSAM
#define	ROTH	04
#define	WOTH	02
#define	XOTH	01
#define	RSTXT	01000

main(argc, argv)
char **argv;
{
	int i, j;
#ifdef VN6_ORIG
	extern struct lbuf end;
#endif
	register struct lbuf *ep, *ep1;
	register struct lbuf *slastp;
	struct lbuf lb;
	int t;
	int compar();

	fout = dup(1);
#ifndef VN6_ORIG
	lb.lmtime = time();
	year = lb.lmtime - 15552000L;
#endif
#ifdef VN6_ORIG
	time(lb.lmtime);
	year = lb.lmtime[0] - 245; /* 6 months ago */
#endif
	if (--argc > 0 && *argv[1] == '-') {
		argv++;
		while (*++*argv) switch (**argv) {
		case 'a':
			aflg++;
			continue;

		case 's':
			sflg++;
			statreq++;
			continue;

		case 'd':
			dflg++;
			continue;

#ifndef	AUSAM
		case 'g':
			gflg++;
			continue;
#endif

		case 'l':
			lflg++;
			statreq++;
			continue;

		case 'r':
			rflg = -1;
			continue;

		case 't':
			tflg++;
			statreq++;
			continue;

		case 'u':
			uflg++;
			continue;

		case 'i':
			iflg++;
			continue;

		case 'f':
			fflg++;
			continue;

#		ifdef	AUSAM
		case 'n':
			nflg++;
			continue;
#		endif

		default:
			continue;
		}
		argc--;
	}
	if (fflg) {
		aflg++;
		lflg = 0;
		sflg = 0;
		tflg = 0;
		statreq = 0;
	}
#ifndef	AUSAM
	if(lflg) {
		t = "/etc/passwd";
		if(gflg)
			t = "/etc/group";
		uidfil = open(t, 0);
	}
#endif
	if (argc==0) {
		argc++;
		argv = &dotp - 1;
	}
	for (i=0; i < argc; i++) {
		if ((ep = gstat(*++argv, 1))==0)
			continue;
		ep->namep = *argv;
		ep->lflags =| ISARG;
	}
	qsort(end, lastp - end, sizeof *lastp, compar);
	slastp = lastp;
	for (ep = end; ep<slastp; ep++) {
		if (ep->lflags&DIR && dflg==0 || fflg) {
			if (argc>1)
				printf("\n%s:\n", ep->namep);
			lastp = slastp;
			readdir(ep->namep);
			if (fflg==0)
				qsort(slastp,lastp - slastp,sizeof *lastp,compar);
			if (statreq)
				printf("total %u\n", tblocks);
			for (ep1=slastp; ep1<lastp; ep1++)
				pentry(ep1);
		} else 
			pentry(ep);
	}
	flush();
	exit(0);		/* added by KFH so can be tested */
}

pentry(ap)
struct lbuf *ap;
{
	struct { char dminor, dmajor;};
	register t;
	register struct lbuf *p;
	register char *cp;
#ifdef	AUSAM
	struct pwent pe;
	char sbuf[SSIZ];
	static unsigned lastuid 0XFFFF;
	static lastname[SSIZ];
	struct { char *chp; };
#endif

	p = ap;
	if (p->lnum == -1)
		return;
	if (iflg)
		printf("%5u ", p->lnum);
	if (lflg) {
		pmode(p->lflags);
		printf("%2u ", p->lnl);
#ifndef	AUSAM
		t = p->luid;
		if(gflg)
			t = p->lgid;
		t =& 0377;
		if (getname(t, tbuf)==0)
#ifndef VN6_ORIG
			printf("%-8.8s", tbuf);
#endif
#ifdef VN6_ORIG
			printf("%-6.6s", tbuf);
#endif
#endif
#ifdef	AUSAM
		if ( lastuid == p->luid )
		{
			pe.pw_strings[LNAME] = lastname;
			goto pname;
		}
		pe.pw_uid = p->luid;
		if ( !nflg && getpwlog( &pe, sbuf, SSIZ ) > 0 )
		{
			if ( lastuid == 0XFFFF )
			{
				lastuid = pe.pw_uid;
				for ( cp = lastname, t.chp = pe.pw_strings[LNAME]; *cp++ = *t.chp++ ; );
			}
pname:
			printf( "%-8.8s", pe.pw_strings[LNAME] );
		}
		else
			printf( "%-8d", pe.pw_uid );
#endif
#ifndef	AUSAM
		else
#ifndef VN6_ORIG
			printf("%-8d", t);
#endif
#ifdef VN6_ORIG
			printf("%-6d", t);
#endif
#endif
		if (p->lflags & (BLK|CHR))
			printf("%3u,%3u", p->lsize.dmajor&0377,
			    p->lsize.dminor&0377);
		else
#ifndef VN6_ORIG
			printf("%7s", locv(p->lsize0&0377, p->lsize));
#endif
#ifdef VN6_ORIG
			printf("%7s", locv(p->lsize0, p->lsize));
#endif
		cp = ctime(p->lmtime);
#ifndef VN6_ORIG
		if(p->lmtime < year)
#endif
#ifdef VN6_ORIG
		if(p->lmtime[0] < year)
#endif
			printf(" %-7.7s %-4.4s ", cp+4, cp+20); else
			printf(" %-12.12s ", cp+4);
	} else if (sflg)
#ifndef VN6_ORIG
		printf("%5u ", nblock(p->lsize0, p->lsize));
#endif
#ifdef VN6_ORIG
		printf("%4d ", nblock(p->lsize0, p->lsize));
#endif
	if (p->lflags&ISARG)
		printf("%s\n", p->namep);
	else
		printf("%.14s\n", p->lname);
}

#ifndef	AUSAM
getname(uid, buf)
int uid;
char buf[];
{
	int j, c, n, i;

	if (uid==lastuid)
		return(0);
	inf.fdes = uidfil;
	seek(inf.fdes, 0, 0);
	inf.nleft = 0;
	lastuid = -1;
	do {
		i = 0;
		j = 0;
		n = 0;
		while((c=getc(&inf)) != '\n') {
			if (c<0)
				return(-1);
			if (c==':') {
				j++;
				c = '0';
			}
			if (j==0)
				buf[i++] = c;
			if (j==2)
				n = n*10 + c - '0';
		}
	} while (n != uid);
	buf[i++] = '\0';
	lastuid = uid;
	return(0);
}
#endif

nblock(size0, size)
#ifdef	VN6_ORIG
char *size0, *size;
#endif
#ifndef	VN6_ORIG
  unsigned size0, size;
#endif
{
	register int n;

#ifndef VN6_ORIG
	n = ldiv(size0&0377, size, 512);
#endif
#ifdef VN6_ORIG
	n = ldiv(size0, size, 512);
#endif
	if (size&0777)
		n++;
	if (n>8)
		n =+ (n+255)/256;
	return(n);
}

int	m0[] { 3, DIR, 'd', BLK, 'b', CHR, 'c', '-'};
int	m1[] { 1, ROWN, 'r', '-' };
int	m2[] { 1, WOWN, 'w', '-' };
int	m3[] { 2, SUID, 's', XOWN, 'x', '-' };
#ifndef	AUSAM
int	m4[] { 1, RGRP, 'r', '-' };
int	m5[] { 1, WGRP, 'w', '-' };
int	m6[] { 2, SGID, 's', XGRP, 'x', '-' };
#endif
#ifdef	AUSAM
int	m4[] { 0, '-' };
#endif
int	m7[] { 1, ROTH, 'r', '-' };
int	m8[] { 1, WOTH, 'w', '-' };
int	m9[] { 1, XOTH, 'x', '-' };
#ifndef	AUSAM
int	m10[] { 1, STXT, 'T', ' ' };
#endif
#ifdef	AUSAM
int	m10[] { 2, STXT, 'T', SLOK, '\0', ' ' };
int	m11[] { 2, AUTOLOK, 'A', SLOK, 'L', '\0' };
#endif

#ifndef	AUSAM
int	*m[] { m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10};
#endif
#ifdef	AUSAM
int	*m[] { m0, m1, m2, m3, m4, m4, m4, m7, m8, m9, m10, m11};
#endif
#ifndef	VN6_ORIG
#define	MSIZ		((sizeof m)/(sizeof m[0]))
#endif


pmode(aflag)
{
	register int **mp;

	flags = aflag;
#	ifdef	AUSAM
	if ( !(flags & SLOK) )
		flags =& ~AUTOLOK;
#	endif	AUSAM
#ifdef	VN6_ORIG
	for (mp = &m[0]; mp < &m[11];)
#endif
#ifndef	VN6_ORIG
	for (mp = &m[0]; mp < &m[MSIZ];)
#endif
		select(*mp++);
}

select(pairp)
int *pairp;
{
	register int n, *ap;

	ap = pairp;
	n = *ap++;
	while (--n>=0 && (flags&*ap++)==0)
		ap++;
	putchar(*ap);
}

makename(dir, file)
char *dir, *file;
{
	static char dfile[100];
	register char *dp, *fp;
	register int i;

	dp = dfile;
	fp = dir;
	if(*fp)
	{
		do
			*dp++ = *fp++;
		while (*fp);
		*dp++ = '/';
	}
	fp = file;
	for (i=0; i<14; i++)
		*dp++ = *fp++;
	*dp = 0;
	return(dfile);
}

readdir(dir)
char *dir;
{
	static struct {
		int	dinode;
		char	dname[14];
	} dentry;
	register char *p;
	register int j;
	register struct lbuf *ep;

	if (fopen(dir, &inf) < 0) {
		printf("%s unreadable\n", dir);
		return;
	}
	tblocks = 0;
	for(;;) {
		p = &dentry;
		for (j=0; j<16; j++)
			*p++ = getc(&inf);
		if (dentry.dinode==0
		 || aflg==0 && dentry.dname[0]=='.')
			continue;
		if (dentry.dinode == -1)
			break;
		ep = gstat(makename(dir, dentry.dname), 0);
		if (ep->lnum != -1)
			ep->lnum = dentry.dinode;
		for (j=0; j<14; j++)
			ep->lname[j] = dentry.dname[j];
	}
	close(inf.fdes);
}

gstat(file, argfl)
char *file;
{
	struct ibuf statb;
	register struct lbuf *rep;

	if (lastp+1 >= rlastp) {
		sbrk(512);
		rlastp.idev =+ 512;
	}
	rep = lastp;
	lastp++;
	rep->lflags = 0;
	rep->lnum = 0;
	if (argfl || statreq) {
#ifndef	AUSAM
		if (stat(file, &statb)<0) {
#endif
#ifdef	AUSAM
		if (newstat(file, &statb)<0) {
#endif
			printf("%s not found\n", file);
			statb.inum = -1;
			statb.isize0 = 0;
			statb.isize = 0;
			statb.iflags = 0;
			if (argfl) {
				lastp--;
				return(0);
			}
		}
		rep->lnum = statb.inum;
		statb.iflags =& ~DIR;
		if ((statb.iflags&IFMT) == 060000) {
			statb.iflags =& ~020000;
		} else if ((statb.iflags&IFMT)==040000) {
			statb.iflags =& ~IFMT;
			statb.iflags =| DIR;
		}
		statb.iflags =& ~ LARGE;
		if (statb.iflags & RSTXT)
			statb.iflags =| STXT;
		statb.iflags =& ~ RSTXT;
		rep->lflags = statb.iflags;
		rep->luid = statb.iuid;
#ifndef	AUSAM
		rep->lgid = statb.igid;
#endif
		rep->lnl = statb.inl;
		rep->lsize0 = statb.isize0;
		rep->lsize = statb.isize;
		if (rep->lflags & (BLK|CHR) && lflg)
			rep->lsize = statb.iaddr[0];
#ifndef VN6_ORIG
		rep->lmtime = statb.imtime;
		if(uflg) 
			rep->lmtime = statb.iatime;
#endif
#ifdef VN6_ORIG
		rep->lmtime[0] = statb.imtime[0];
		rep->lmtime[1] = statb.imtime[1];
		if(uflg) {
			rep->lmtime[0] = statb.iatime[0];
			rep->lmtime[1] = statb.iatime[1];
		}
#endif
		tblocks =+ nblock(statb.isize0, statb.isize);
	}
	return(rep);
}

compar(ap1, ap2)
struct lbuf *ap1, *ap2;
{
	register struct lbuf *p1, *p2;
	register int i;
	int j;
	struct { char *charp;};

	p1 = ap1;
	p2 = ap2;
	if (dflg==0) {
		if ((p1->lflags&(DIR|ISARG)) == (DIR|ISARG)) {
			if ((p2->lflags&(DIR|ISARG)) != (DIR|ISARG))
				return(1);
		} else {
			if ((p2->lflags&(DIR|ISARG)) == (DIR|ISARG))
				return(-1);
		}
	}
	if (tflg) {
		i = 0;
#ifndef VN6_ORIG
		if(p2->lmtime > p1->lmtime)
#endif
#ifdef VN6_ORIG
		if (p2->lmtime[0] > p1->lmtime[0])
#endif
			i++;
#ifndef VN6_ORIG
		if(p2->lmtime < p1->lmtime)
#endif
#ifdef VN6_ORIG
		else if (p2->lmtime[0] < p1->lmtime[0])
#endif
			i--;
#ifdef VN6_ORIG
		else if (p2->lmtime[1] > p1->lmtime[1])
			i++;
		else if (p2->lmtime[1] < p1->lmtime[1])
			i--;
#endif
		return(i*rflg);
	}
	if (p1->lflags&ISARG)
		p1 = p1->namep;
	else
		p1 = p1->lname;
	if (p2->lflags&ISARG)
		p2 = p2->namep;
	else
		p2 = p2->lname;
	for (;;)
		if ((j = *p1.charp++ - *p2.charp++) || p1.charp[-1]==0)
			return(rflg*j);
	return(0);
}
