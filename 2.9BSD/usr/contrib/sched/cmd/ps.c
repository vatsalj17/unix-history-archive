/*
 *	ps - process status
 *	examine and print certain things about processes
 */

#define XSCHED	1	/* use .h defns for experimental scheduler version */

#include <stdio.h>
#include <a.out.h>
#include <core.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <termio.h>
#include <sys/tty.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <pwd.h>
#include <sys/file.h>
#include <sys/inode.h>
#include <sys/buf.h>

struct nlist nl[] = {
	{ "_buf" },	/* must be first pair -- see wsyminit() !!! */
	{ "_nbuf" },
	{ "_proc" },
#define	X_PROC	2
	{ "_endproc" },
#define X_EPROC	3
	{ "_file" },
	{ "_endfile" },
	{ "_inode" },
	{ "_endinod" },
	{ "_dztty" },
	{ "_enddztt" },
	{ "_dltty" },
	{ "_enddltt" },
	{ "_dhtty" },
	{ "_enddhtt" },
	{ "_qztty" },
	{ "_endqztt" },
#define WSARRAY	8
	/* end of WSARRAY, start of WSYMS */
	{ "_runin" },
	{ "_runout" },
	{ "_bfreeli" },
	{ "_swbuf1" },
	{ "_swbuf2" },
#define WSYMS	5
	/* end of special section of nlist */
	{ "_swapdev" },
#define X_SWAPDEV	(WSARRAY*2 + WSYMS)
	{ "_swplo" },
#define X_SWPLO		(X_SWAPDEV+1)
	{ "" },
};

dev_t	swapdev	= NODEV;
struct	proc mproc;

struct	user u;
int	chkpid;
int	retcode=1;
int	lflg;
int	xflg;
char	*tptr = 0;
char	*kname = 0;	/* name of "/dev/kmem" alternative */
char	*uname = 0;	/* name of "/unix" alternative */
long	lseek();
char	*gettty();
int nproc;
char	*getptr();
char	*strncmp();
int	aflg;
int	mem;
int	swmem;
int	swpflg = 1;	/* try to read swap file?? -- don't if not /dev/kmem */
int	swap;
daddr_t	swplo;

char outbuf[BUFSIZ];
int	ndev;
struct devl {
	char	dname[DIRSIZ];
	dev_t	dev;
} devl[256];


main(argc, argv)
int argc;
char *argv[];
{
	register int i;
	register char *ap;
	int uid, puid;
	extern char *_pgmname;
	int nother = 0;
	int nrun = 0;
	int nsleep = 0;
	int nwait = 0;

	_pgmname = "ps";
	setbuf(stdout, outbuf);
	uid = getuid();
	if (argc > 1) {
		/* process flags */
		argc--; argv++;
		for(ap = *argv++,argc--; *ap; ap++)
			switch(*ap){
			case '-':
				break;
			case 'a':
				aflg++;
				break;

			case 'x':
				xflg++;
				break;

			case 't':
				aflg++;
				if(argc > 0){
					argc--;
					tptr = *argv++;
					if(*tptr == '\0')
						tptr = 0;
				}
				break;
			case 'l':
				lflg++;
				break;

			case 'k':
				if(argc > 0){
					kname = *argv++;
					argc--;
					if(*kname == '\0' || uid != 0)
						kname = 0;
					if(kname != 0)
						swpflg = 0;
				}
				break;

			case 'p':
				if(argc > 0){
					argc--;
					chkpid = atoi(*argv++);
				}
				break;

			case 'u':
				if(argc > 0){
					argc--;
					uname = *argv++;
					if(*uname == '\0' || uid != 0)
						uname = 0;
				}
				break;

			default:
				panic("bad option character: '%c'",*ap);
		}
	}

	if(uname == 0)
		uname = "/unix";
	nlist(uname, nl);
	if (nl[X_PROC].n_type==0)
		panic("No namelist: '%s'",uname);
	if(kname == 0)
		kname = "/dev/mem";
	if ((mem = open(kname, 0)) < 0)
		panic("%#Can't read '%s'",kname);
	swmem = open(kname, 0);
	/*
	 * read mem to find swap dev.
	 */
	lseek(mem, (long)nl[X_SWAPDEV].n_value, 0);
	read(mem, (char *)&nl[X_SWAPDEV].n_value, sizeof(nl[X_SWAPDEV].n_value));
	swapdev = nl[X_SWAPDEV].n_value;
	/*
	 * Find base of swap
	 */
	lseek(mem, (long)nl[X_SWPLO].n_value, 0);
	read(mem, (char *)&swplo, sizeof(swplo));

	wsyminit();	/* set up wchan symbol table */
	/*
	 * Locate proc table
	 */
	lseek(mem,(long)nl[X_EPROC].n_value,0);
	read(mem,(char *) &nproc,sizeof(nproc));
	nproc -= nl[X_PROC].n_value;
	nproc /= sizeof(struct proc);
	lseek(mem, (long)nl[X_PROC].n_value, 0);
	getdev();
	if (lflg)
		printf("SIZE WAITING-FOR LEVEL USER    PPID  ");
	printf(" TIME  TTY PID  COMMAND\n");
	fflush(stdout);
	for (i=0; i<nproc; i++) {
		read(mem, (char *)&mproc, sizeof mproc);
		switch(mproc.p_stat){
		case SEMPTY:
			continue;
		case SRUN:
			nrun++;
			break;
		case SSLEEP:
			nsleep++;
			break;
		case SWAIT:
			nwait++;
			break;
		default:
			nother++;
		}
		if (mproc.p_pgrp==0 && xflg==0 && mproc.p_uid==0)
			continue;
		puid = mproc.p_uid;
		if ((uid != puid && aflg==0) ||
		    (chkpid!=0 && chkpid!=mproc.p_pid))
			continue;
		if(prcom(puid)) {
			printf("\n");
			retcode=0;
		}
	}
	if(lflg)
	printf("\n%d active: %d running, %d sleeping, %d waiting, %d other\n",
		nother+nrun+nsleep+nwait,nrun,nsleep,nwait,nother);
	exit(retcode);
}

char devdir[] = "/dev/";
getdev()
{
#include <sys/stat.h>
	register FILE *df;
	struct stat sbuf;
	struct direct dbuf;
	char swapfile[sizeof(dbuf.d_name)+sizeof(devdir)];
	char dnam[sizeof(dbuf.d_name)+sizeof(devdir)];

	swapfile[0] = '\0';
	if ((df = fopen("/dev", "r")) == NULL)
		panic("%#Can't open /dev");
	ndev = 0;
	while (fread((char *)&dbuf, sizeof(dbuf), 1, df) == 1) {
		if(dbuf.d_ino == 0)
			continue;
		strcpy(dnam,"/dev/");
		strncat(dnam,dbuf.d_name,sizeof(dbuf.d_name));
		dnam[sizeof(dnam)-1] = '\0';
		if(stat(dnam, &sbuf) < 0)
			continue;

		if(swpflg && (sbuf.st_mode&S_IFMT) == S_IFBLK)
			if(sbuf.st_rdev == swapdev){
				strcpy(swapfile,dnam);
			}
		if ((sbuf.st_mode&S_IFMT) != S_IFCHR)
			continue;
		strncpy(devl[ndev].dname, dbuf.d_name, sizeof(devl[0].dname));
		devl[ndev].dev = sbuf.st_rdev;
		ndev++;
	}
	fclose(df);
	if(swpflg){
		if(swapfile[0] == '\0')
			panic("can't find swapfile");
		if((swap = open(swapfile, 0)) < 0)
			panic("%#can't open '%s'",swapfile);
	}
}

long
round(a, b)
	long		a, b;
{
	long		w = ((a+b-1)/b)*b;

	return(w);
}

struct map {
	long	b1, e1; long f1;
	long	b2, e2; long f2;
};
struct map datmap;
int	ffile;
prcom(puid)
{
	char *user_name();
	char abuf[512];
	long addr;
	register int *ip;
	register char *cp, *cp1;
	long tm;
	int c, nbad;
	register char *tp;
	long txtsiz, datsiz, stksiz;
	int septxt;
	int lw=(lflg?25:50);
	char **ap;

	if(mproc.p_stat != SEMPTY && mproc.p_stat != SZOMB){
		if (mproc.p_flag&SLOAD) {
			addr = ctob((long)mproc.p_addr);
			ffile = swmem;
		} else{
			addr = (mproc.p_addr+swplo)<<9;
			ffile = swap;
		}
		if(ffile != swap || swpflg){
			lseek(ffile, addr, 0);
			if (read(ffile, (char *)&u, sizeof(u)) != sizeof(u))
				return(0);
	
			/* set up address maps for user pcs */
			txtsiz = ctob(u.u_tsize);
			datsiz = ctob(u.u_dsize);
			stksiz = ctob(u.u_ssize);
			septxt = u.u_sep;
			datmap.b1 = (septxt ? 0 : round(txtsiz,TXTRNDSIZ));
			datmap.e1 = datmap.b1+datsiz;
			datmap.f1 = ctob(USIZE)+addr;
			datmap.b2 = stackbas(stksiz);
			datmap.e2 = stacktop(stksiz);
			datmap.f2 = ctob(USIZE)+(datmap.e1-datmap.b1)+addr;
		
			tp = gettty();
			if (tptr && strncmp(tptr, tp, 2))
				return(0);
		}
		if (lflg) {
			printf("%3d %c",
				(mproc.p_size + 15)>>4,	/* K-bytes */
				(mproc.p_flag & SLOAD)? '*':' ');
			prwchan(mproc.p_wchan, mproc.p_stat);
			printf(" %2d %2d %-7.7s %5u ",
				mproc.p_level,
				mproc.p_baslev,
				user_name(puid),
				mproc.p_ppid);
		}
		if(ffile != swap || swpflg){
			tm = (u.u_utime + u.u_stime + 30)/60;
			printf("%3D:%02D %-2.2s %5u ",
				(tm / 60), (tm % 60),
				tp,mproc.p_pid);
		}
		else {
			printf("          %5u ",mproc.p_pid);
		}
	}
	else {	/* ZOMBIE or empty slot  */
		if (lflg) {
			printf("%3d %c",
				(mproc.p_size + 15)>>4,	/* K-bytes */
				(mproc.p_flag & SLOAD)? '*':' ');
			prwchan(mproc.p_wchan, mproc.p_stat);
			printf(" %2d %2d %-7.7s %5u ",
				mproc.p_level,
				mproc.p_baslev,
				user_name(puid),
				mproc.p_ppid);
		}
		printf("          %5u ",mproc.p_pid);
		if(mproc.p_stat == SZOMB)
			printf(" <PROCESS EXITING>");
		else
			printf(" <EMPTY>");
		return(1);
	}
	if (mproc.p_pid == 0) {
		printf(" swapper");
		return(1);
	}
	if(ffile == swap && swpflg == 0){
		printf(" <SWAPPED OUT>");
		return(1);
	}
	addr += ctob((long)mproc.p_size) - 512;

	/* look for command name and arguments and print them */

	lseek(ffile, addr+512-sizeof(char **), 0);
	if (read(ffile, (char *)&ap, sizeof(char *)) != sizeof(char *))
		return(1);
	if (ap) {
		char b[52];
		char *bp = b;
		while((cp=getptr(ap++)) && cp && (bp<b+lw) ) {
			nbad = 0;
			while((c=getbyte(cp++)) && (bp<b+lw)) {
				if (c<' ' || c>'~') {
					if (nbad++>3)
						break;
					continue;
				}
				*bp++ = c;
			}
			*bp++ = ' ';
		}
		*bp++ = 0;
		prcmd(b);
		return(1);
	}

	lseek(ffile, addr, 0);
	if (read(ffile, abuf, sizeof(abuf)) != sizeof(abuf))
		return(1);
	for (ip = (int *)&abuf[512]-2; ip > (int *)abuf; ) {
		if (*--ip == -1 || *ip==0) {
			cp = (char *)(ip+1);
			if (*cp==0)
				cp++;
			nbad = 0;
			for (cp1 = cp; cp1 < &abuf[512]; cp1++) {
				c = *cp1&0177;
				if (c==0)
					*cp1 = ' ';
				else if (c < ' ' || c > 0176) {
					if (++nbad >= 5) {
						*cp1++ = ' ';
						break;
					}
					*cp1 = '?';
				} else if (c=='=') {
					*cp1 = 0;
					while (cp1>cp && *--cp1!=' ')
						*cp1 = 0;
					break;
				}
			}
			while (*--cp1==' ')
				*cp1 = 0;
			prcmd(cp);
			return(1);
		}
	}
	printf(" (%.*s)", DIRSIZ, u.u_comm);
	return(1);
}

prcmd(s)
char *s;
{
	register char *p;
	int f = (lflg ? 25 : 50);	/* field width */
	int bad = 0;
	int good = 0;

	while (*s == ' ' || *s == '\t')
		s++;

	for (p = s; *p && *p != ' ' && *p != '\t'; p++) {
		if (*p < ' ')	/* non-printable */
			bad++;
		if ((*p >= 'a' && *p <= 'z') || (*p >= '0' && *p <= '9')
		  || (*p >= 'A' && *p <= 'Z'))
			good++;
		if (p >= (s + f))
			bad++;
	}

	if (bad || (!good))
		printf(" (%.*s)", DIRSIZ, u.u_comm);
	else
		printf(" %.*s", f, s);
}

char *
gettty()
{
	register i;
	register char *p;

	if (u.u_ttyp==0)
		return("?");
	for (i=0; i<ndev; i++) {
		if (devl[i].dev == u.u_ttyd) {
			p = devl[i].dname;
			if (p[0]=='t' && p[1]=='t' && p[2]=='y')
				p += 3;
			return(p);
		}
	}
	return("?");
}

char *
getptr(adr)
char **adr;
{
	char *ptr;
	register char *p, *pa;
	register i;

	ptr = 0;
	pa = (char *)adr;
	p = (char *)&ptr;
	for (i=0; i<sizeof(ptr); i++)
		*p++ = getbyte(pa++);
	return(ptr);
}

getbyte(adr)
char *adr;
{
	register struct map *amap = &datmap;
	char b;
	long saddr;

	if(!within(adr, amap->b1, amap->e1)) {
		if(within(adr, amap->b2, amap->e2)) {
			saddr = (unsigned)adr + amap->f2 - amap->b2;
		} else
			return(0);
	} else
		saddr = (unsigned)adr + amap->f1 - amap->b1;
	if(lseek(ffile, saddr, 0)==-1
		   || read(ffile, &b, 1)<1) {
		return(0);
	}
	return((unsigned)b);
}


within(adr,lbd,ubd)
char *adr;
long lbd, ubd;
{
	return((unsigned)adr>=lbd && (unsigned)adr<ubd);
}

/*
 * Maintain list of user names, and return them on demand.
 */

#define MAXUTAB	50

/******************************
char *
center(s)
register char *s;
{
	register int n;
	register char *p;
	static char b[8];

	n = 7 - strlen(s);
	n /= 2;

	for(p = &b[0]; n-- > 0;)
		*p++ = ' ';

	while(p < &b[7]){
		if(*s != '\0')
			*p++ = *s++;
		else
			*p++ = ' ';
	}
	*p = '\0';
	return(&b[0]);
}
********************************/


char *
user_name(u)
int u;
{
	static char nb[8];
	register struct passwd *p;
	struct utab {
		short	x_uid;		/* user id number */
		char	x_name[8];	/* user name */
	};
	static struct utab utab[MAXUTAB];
	register struct utab *up;
	register char *s;

	for(up = &utab[0]; up < &utab[MAXUTAB]; up++){
		s = up->x_name;
		if(*s == '\0' || up->x_uid == u)
			break;
	}
	if(up >= &utab[MAXUTAB] || *s == '\0'){
		p = getpwuid(u);
		if(p != NULL && p->pw_name != NULL){
			if(up < &utab[MAXUTAB]){
				strncpy(s,p->pw_name,8);
				s[7] = '\0';
				up->x_uid = u;
			}
			s = p->pw_name;
		}
		else {
			sprintf(nb,"%d",u);
			s = nb;
		}
	}
	return(s);
}

/*
 * Print wchan value, symbolically interpreted.
 */

struct wsym {
	char *w_string;		/* label or name of the wchan */
	unsigned w_size;	/* number of bytes */
	unsigned w_base;	/* starting address */
	unsigned w_end;		/* first address AFTER it */
};

struct wsym wsyms[] = {
	{ "runin",	1 },
	{ "runout",	1 },
	{ "bfreelist",	sizeof(struct buf) },
	{ "swbuf1",	sizeof(struct buf) },
	{ "swbuf2",	sizeof(struct buf) },
	{ "pause",	1, 0140000, 0140001 },
	{0}
};

struct wsym warray[] = {
	{ "buf",	sizeof(struct buf) },
	{ "proc",	sizeof(struct proc) },
	{ "file",	sizeof(struct file) },
	{ "inode",	sizeof(struct inode) },
	{ "dz",		sizeof(struct tty) },
	{ "dl",		sizeof(struct tty) },
	{ "dh",		sizeof(struct tty) },
	{ "qz",		sizeof(struct tty) },
	{ 0 }
};

prwchan(w, s)
register unsigned w;	/* wchan address */
char s;			/* process state */
{
	register struct wsym *wp;
	char b[20];
	register char *x;

	switch(s){
	case SRUN:
		x = "ready";
		break;
	case SSTOP:
		x = "stopped";
		break;
	case SIDL:
		x = "newborn";
		break;
	case SZOMB:
		x = "zombie";
		break;
	case SEMPTY:
		x = "empty";
		break;
	case SWAIT:
	case SSLEEP:
		x = 0;
		break;
	default:
		x = "garbage";
		break;
	}
	if (x) {
		sprintf(b, "<%.8s>", x);
		goto prt_it;
	}

	/* SSLEEP and SWAIT cases */
	/* first check scalars */

	for (wp = &wsyms[0]; wp->w_string; wp++) {
		if ((w >= wp->w_base) && (w < wp->w_end)) {
			sprintf(b, "%.10s", wp->w_string);
			goto prt_it;
		}
	}

	/* then check arrays */

	for (wp = &warray[0]; wp->w_string; wp++) {
		if ((w >= wp->w_base) && (w < wp->w_end)) {
			w = (w - wp->w_base) / wp->w_size;
			if(w > 999)
				sprintf(b, "%.5s[???]", wp->w_string);
			else
				sprintf(b, "%.5s[%d]", wp->w_string, w);
			goto prt_it;
		}
	}

	/* sigh.  No symbolic match */

	printf(" %06o    ", w);
	return;
prt_it:
	if (s == SSLEEP) {
		for (x = &b[0]; *x; x++) {
			if(*x >= 'a' && *x <= 'z')
				*x += 'A' - 'a';
		}
	}

	printf(" %-10.10s", b);
}

wsyminit()
{
	register struct wsym *wp;
	register struct nlist *np;
	register int i;
	long lseek();
	int *e;		/* read end address of table */

	for (i = WSARRAY, wp = &warray[0], np = &nl[0]; i > 0; i--, wp++) {
		if (np[0].n_type > 0 && np[1].n_type > 0) {
			wp->w_base = np[0].n_value;
			if (lseek(mem,(long)(np[1].n_value),0) != -1L &&
			  read(mem, &e, sizeof(e)) == sizeof(e))
				wp->w_end = (unsigned) e;
			else
				wp->w_end = wp->w_base;	/* invalid */
		} else
			wp->w_end = wp->w_base = 0;
		np += 2;
	}
	/* unfortunately, buf[] is not delimited the same way, need to hack */
	if(warray[0].w_base != warray[0].w_end)	/* valid initialization */
		warray[0].w_end = warray[0].w_end * warray[0].w_size +
				warray[0].w_base;

	for (i = WSYMS, wp = &wsyms[0]; i > 0; i--, wp++) {
		if(np->n_type > 0) {
			wp->w_base = np->n_value;
			wp->w_end = wp->w_base + wp->w_size;
		} else
			wp->w_base = wp->w_end = 0;
		np++;
	}
}
