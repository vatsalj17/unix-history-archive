#

/*
 * Make Interdata UNIX dump tape from PDP-11 UNIX RK disc image
 *
 * rkdump fsb filesystem
 * f take output tape from arglist
 * s specify tape size in feet (feet = blocks/9)
 * b specify tape size in blocks
 */

char	*dargv[]
{
	0,
	"0",
	"/dev/dfix",
	0
};

#include "/usr/sys/ino.h"
#define	MAXSIZE	1000
#define XCBLKINT 128		/*******/
#define	XCBLKSH	256		/*******/

/*
 * Inode structure as it appears on
 * PDP-11 disk.
 */
struct	iinode
{
	short	ii_mode;
	char	ii_nlink;
	char	ii_uid;
	char	ii_gid;
	char	ii_size0;
	short	ii_size1;
	short	ii_addr[8];
	short	ii_atime[2];
	short	ii_mtime[2];
};

/* modes */
#define	IALLOC	0100000
#define	IFMT	060000
#define		IFDIR	040000
#define		IFCHR	020000
#define		IFBLK	060000
#define	ILARG	010000
#define	ISUID	04000
#define	ISGID	02000
#define ISVTX	01000
#define	IREAD	0400
#define	IWRITE	0200
#define	IEXEC	0100
struct {
	short	s_isize;
	short	s_fsize;
	short	s_junk[254];
} sblock;


char	*ofile;
int	*talist;
int	fi;
int	buf[XCBLKINT];
short	dbuf[XCBLKSH];
short	ibuf[XCBLKSH];
short	vbuf[XCBLKSH];
char	*date[2];
char	*ddate[2];
int	fo	-1;
int	pher;
int	dflg;
int	iflg;
int	cflg;
int	aflg;
char	*tsize	19000;
char	*taddr;

main(argc, argv)
char **argv;
{
	char *key;
	int s, i, nfil, nblk, f;
	register *tap;
	register struct iinode *ip;
	int ino;
	int isize;

	ofile = "/dev/mt0";
	time(date);
	if(argc == 1) {
		argv = dargv;
		for(argc = 1; dargv[argc]; argc++);
	}

	argc--;
	argv++;
	key = *argv;
	while(*key)
	switch(*key++) {

	default:
		printf("bad character in key\n");
		exit();

	case '-':
		continue;

	case 'c': /* increment file name */
		cflg++;
		continue;

	case 'f': /* file name from arg list */
		argc--;
		argv++;
		ofile = *argv;
		continue;

	case 's': /* tape size */
		tsize = number(argv[1]) * 9;
		argv++;
		argc--;
		continue;

	case 'b': /* tape size */
		tsize = number(argv[1]);
		argv++;
		argc--;
		continue;

	case '0': /* dump all */
		ddate[0] = ddate[1] = 0;
		continue;

	}
	if(argc <= 1) {
		printf("no file system specified\n");
		exit();
	}
	printf("%s:\n", argv[1]);
	fi = open(argv[1], 0);
	if(fi < 0) {
		printf("cannot open %s\n", argv[1]);
		exit();
	}
	printf("RK filesystem dump\n");
	sync();
	bread(1, &sblock);
	isize = swab(sblock.s_isize);
/***/	talist = sbrk(size(0, isize*(512*4/sizeof (struct iinode))*512));
	tap = talist;
	if(tap == -1) {
		printf("No memory\n");
		exit();
	}
	nfil = 0;
	nblk = size(0, isize*(512*4/sizeof (struct iinode)));	/***/
	ino = 0;
	for(i=0; i<isize; i++) {
		bread(i+2, buf);
		for(ip = &buf[0]; ip < &buf[XCBLKINT]; ip++) {
			ino++;
			if(ip->ii_mode == 0 || ip->ii_nlink == 0) {
				*tap++ = -1;
				continue;
			}
			s = size(ip->ii_size0&0377, swab(ip->ii_size1)) + 1;
			if (s>MAXSIZE && aflg==0 && iflg!=0) {
				printf("%l big; not dumped.\n", ino);
				goto no;
			}
			nfil++;
			nblk =+ s;
			*tap++ = s;
			continue;
		no:
			*tap++ = 0;
		}
	}
	printf("%l files\n%l blocks\n", nfil, nblk);
	i = ldiv(0, nblk, ldiv(0, tsize, 10));
	printf("%l.%l tapes\n", i/10, i%10);
	tap = buf;
	clrbuf(tap);
	*tap++ = isize*sizeof (struct inode)/sizeof (struct iinode);
	*tap++ = swab(sblock.s_fsize);
	*tap++ = date[0];
	*tap++ = date[1];
	*tap++ = ddate[0];
	*tap++ = ddate[1];
	*tap++ = tsize;
	swrite(buf);
	i = size(0, isize*(512*4/sizeof (struct iinode)));	/***/
	tap = talist;
	while(i--) {
		bwrite(tap);
		tap =+ XCBLKINT;
	}
	tap = talist;
	for(i=0; i<isize; i++) {
		bread(i+2, buf);
		for(ip = &buf[0]; ip < &buf[XCBLKINT]; ip++) {
			if(*tap && *tap != -1)
				dump(ip, *tap-1);
			tap++;
		}
	}
	printf("%l phase errors\n", pher);
	exit();
}

dump(ip, sz)
register struct iinode *ip;
{
	register short *p, *q, *r;
	register struct inode *np;
	register int *t;
	int dirflag;

	np = dbuf;
	clrbuf(np);
	np->i_mode = swab(ip->ii_mode);
	np->i_nlink = ip->ii_nlink;
	np->i_uid = ip->ii_uid;
	np->i_gid = ip->ii_gid;
	np->i_size0 = ip->ii_size0;
	np->i_size1 = swab(ip->ii_size1);
	p = &ip->ii_addr[0];
	t = &np->i_addr[0];
	while (p < &ip->ii_addr[8])
		*t++ = swab(*p++);
	np->i_atime[1] = (swab(ip->ii_atime[0])<<16) | swab(ip->ii_atime[1]);
	np->i_mtime[1] = (swab(ip->ii_mtime[0])<<16) | swab(ip->ii_mtime[1]);
	swrite(dbuf);
	if(swab(ip->ii_mode) & (IFBLK&IFCHR)) {
		if(sz != 0)
			printf("special\n");
		return;
	}
	dirflag = ((swab(ip->ii_mode)&IFMT) == IFDIR);
	for(p = &ip->ii_addr[0]; p < &ip->ii_addr[8]; p++)
	if(*p) {
		if(swab(ip->ii_mode)&ILARG) {
			bread(swab(*p), ibuf);
			for(q = &ibuf[0]; q < &ibuf[XCBLKSH]; q++)
			if(*q) {
				if(p == &ip->ii_addr[7]) {
					bread(swab(*q), vbuf);
					for(r = &vbuf[0]; r < &vbuf[XCBLKSH]; r++)
					if(*r) {
						if(--sz < 0)
							goto pe;
						bread(swab(*r), dbuf);
						if (dirflag)
							dirfix(dbuf);
						bwrite(dbuf);
					}
					continue;
				}
				if(--sz < 0)
					goto pe;
				bread(swab(*q), dbuf);
				if (dirflag)
					dirfix(dbuf);
				bwrite(dbuf);
			}
		} else {
			if(--sz < 0)
				goto pe;
			bread(swab(*p), dbuf);
			if (dirflag)
				dirfix(dbuf);
			bwrite(dbuf);
		}
	}
	if(sz)
		goto pe;
	return;

pe:
	clrbuf(dbuf);
	while(--sz >= 0)
		bwrite(dbuf);
	pher++;
}

bread(bno, b)
{

	seek(fi, bno, 3);
	if(read(fi, b, 512) != 512) {
		printf("read error %l\n", bno);
	}
}

clrbuf(b)
int *b;
{
	register i, *p;

	p = b;
	i = XCBLKINT;
	while(i--)
		*p++ = 0;
}

swrite(b)
int *b;
{
	register i, s, *p;

	i = XCBLKINT-2;
	s = taddr;
	p = b;
	while(i--)
		s =+ *p++;
	*p++ = taddr;
	*p = 031415 - s;
	bwrite(b);
}

bwrite(b)
{

	if(taddr == 0) {
		if(fo != -1) {
			printf("change tapes\n");
			close(fo);
			rdline();
		}
		otape();
	}
wloop:
	if(write(fo, b, 512) != 512) {
		printf("write error\n");
		rdline();
		seek(fo, taddr, 3);
		goto wloop;
	}
	taddr++;
	if(taddr >= tsize)
		taddr = 0;
}

rdline()
{
	char c;		/***/

loop:
	c = 0;
	read(0, &c, 1);
	if(c == 0)
		exit();
	if(c != '\n')
		goto loop;
}

number(s)
char *s;
{
	register n, c;

	n = 0;
	while(c = *s++) {
		if(c<'0' || c>'9')
			continue;
		n = n*10+c-'0';
	}
	return(n);
}

size(s0, s1)
{
	register s;
	extern ldivr;

	s = ldiv(s0&0377, s1, 512);
	if(ldivr)
		s++;
	return(s);
}

otape()
{
	register char *p;

	fo = creat(ofile, 0666);
	if(fo < 0) {
		printf("can not open %s\n", ofile);
		exit();
	}
	if(!cflg)
		return;
	p = ofile;
	while(*p++)
		;
	p[-2]++;
}

equal(a, b)
char *a, *b;
{

	while(*a++ == *b)
		if(*b++ == 0)
			return(1);
	return(0);
}

struct	{	char byte[4];	};

swab(pair)
{
	return ((pair.byte[3]<<8) | pair.byte[2]);
}

dirfix(dir)
register struct dir {
	short	dinode;
	char	dname[14];
} *dir;
{
	register struct dir *dp;

	for (dp = dir; dp < &dir[32]; dp++)
		dp->dinode = swab(dp->dinode);
}
