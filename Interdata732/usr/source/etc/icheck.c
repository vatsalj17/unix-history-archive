#

	int buf[128], vbuf[128];	/***/
extern int errno;

char	*dargv[]
{
	"/dev/df0",
	"/dev/dr0",
	"/dev/df1",
	"/dev/dr1",
	0
};
#define	NINODE	8*4
#define	NB	10
#include "/usr/sys/ino.h"
#include "/usr/sys/filsys.h"

struct	inode	inode[NINODE];
struct	filsys	sblock;

int	sflg;

int	fi;
int	nifiles;
int	nfile;
int	nspcl;
int	nlarg;
int	nvlarg;
int	nindir;
int	nvindir;
int	ndir;
int	nused;
int	nfree;
int	ino;
int	ndup;
int	blist[10] { -1};
int	nerror;
#define	BMSIZE	8192
short	bmap[BMSIZE];				/***/

main(argc, argv)
char **argv;
{
	register char **p;
	register int n, *lp;

	if (argc == 1) {
		for (p = dargv; *p;)
			check(*p++);
		exit(nerror);	/***/
	}
	while (--argc) {
		argv++;
		if (**argv=='-') switch ((*argv)[1]) {
		case 's':
			sflg++;
			continue;

		case 'b':
			lp = blist;
			while (lp < &blist[NB-1] && (n = number(argv[1]))) {
				*lp++ = n;
				argv++;
				argc--;
			}
			*lp++ = -1;
			continue;

		default:
			printf("Bad flag\n");
		}
		check(*argv);
	}
	exit(nerror);		/***/
}

check(file)
char *file;
{
	register *ip, i, j;

	fi = open(file, sflg?2:0);
	if (fi < 0) {
		printf("cannot open %s\n", file);
		nerror =| 04;
		return;
	}
	printf("%s:\n", file);
	nfile = 0;
	nspcl = 0;
	nlarg = 0;
	nvlarg = 0;
	nindir = 0;
	nvindir = 0;
	ndir = 0;
	nused = 0;
	nfree = 0;
	ndup = 0;
	for (ip = bmap; ip < &bmap[BMSIZE];)			/***/
		*ip++ = 0;
	sync();
	bread(1, &sblock, 512);
	nifiles = sblock.s_isize*8;			/***/
	for(i=0; ino < nifiles; i =+ NINODE/8) {		/***/
		bread(i+2, inode, sizeof inode);
		for (j=0; j<NINODE && ino<nifiles; j++) {
			ino++;
			pass1(&inode[j]);
		}
	}
	ino = 0;
	sync();
	bread(1, &sblock, 512);
	if (sflg) {
		makefree();
		return;
	}
	while(i = alloc()) {
		if (chk(i, "free"))
			break;
		nfree++;
	}
	if (ndup) {
		printf("%d dups in free\n", ndup);
		nerror =| 02;
	}
	j = 0;
	for (ip = bmap; ip < &bmap[BMSIZE];) {			/***/
		i = *ip++;
		while (i) {
			if (i<0)
				j--;
			i =<< 1;
		}
	}
	j =+ sblock.s_fsize - sblock.s_isize - 2;
	if (j)
		printf("missing%5d\n", j);
	printf("spcl  %6d\n", nspcl);
	printf("files %6d\n", nfile);
	printf("large %6d\n", nlarg);
	if (nvlarg)
		printf("huge  %6d\n", nvlarg);
	printf("direc %6d\n", ndir);
	printf("indir %6d\n", nindir);
	if (nvindir)
		printf("indir2%6d\n", nvindir);
	printf("used  %6d\n", nused);
	printf("free  %6d\n", nfree);
	close(fi);
}

pass1(aip)
struct inode *aip;
{
/*	int buf[128], vbuf[128];	*/
	register i, j, *ip;

	ip = aip;
	if ((ip->i_mode&IALLOC) == 0)
		return;
	if ((ip->i_mode&IFCHR&IFBLK) != 0) {
		nspcl++;
		return;
	}
	if ((ip->i_mode&IFMT) == IFDIR)
		ndir++;
	else
		nfile++;
	if ((ip->i_mode&ILARG) != 0) {
		nlarg++;
		for(i=0; i<7; i++)
		if (ip->i_addr[i] != 0) {
			nindir++;
			if (chk(ip->i_addr[i], "indirect"))
				continue;
			bread(ip->i_addr[i], buf, 512);
			for(j=0; j<128; j++)		/***/
			if (buf[j] != 0)
				chk(buf[j], "data (large)");
		}
		if (ip->i_addr[7]) {
			nvlarg++;
			if (chk(ip->i_addr[7], "indirect"))
				return;
			bread(ip->i_addr[7], buf, 512);
			for(i=0; i<128; i++)		/***/
			if (buf[i] != 0) {
				nvindir++;
				if (chk(buf[i], "2nd indirect"))
					continue;
				bread(buf[i], vbuf, 512);
				for(j=0; j<128; j++)		/***/
				if (vbuf[j])
					chk(vbuf[j], "data (very large)");
			}
		}
		return;
	}
	for(i=0; i<8; i++) {
		if (ip->i_addr[i] != 0)
			chk(ip->i_addr[i], "data (small)");
	}
}

chk(ab, s)
char *ab;
{
	register char *b;
	register n, m;

	b = ab;
	if (ino)
		nused++;
	if (b<sblock.s_isize+2 || b>=sblock.s_fsize) {
		printf("%d bad; inode=%d, class=%s\n", b, ino, s);
		return(1);
	}
	m = 1 << (b&017);
	n = (b>>4) & (BMSIZE-1);
	if (bmap[n]&m) {
		printf("%d dup; inode=%d, class=%s\n", b, ino, s);
		ndup++;
	}
	bmap[n] =| m;
	for (n=0; blist[n] != -1; n++)
		if (b == blist[n])
			printf("%d arg; inode=%d, class=%s\n", b, ino, s);
	return(0);
}

alloc()
{
	register b, i;
/*	int buf[128];	*/

	i = --sblock.s_nfree;
	if (i<0 || i>=50) {			/***/
		printf("bad freeblock\n");
		return(0);
	}
	b = sblock.s_free[i];
	if (b == 0)
		return(0);
	if (sblock.s_nfree <= 0) {
		bread(b, buf, 512);
		sblock.s_nfree = buf[0];
		for(i=0; i<50; i++)		/***/
			sblock.s_free[i] = buf[i+1];
	}
	return(b);
}

bread(bno, buff, cnt)
int *buff;
{
	register *ip;
	register err;

	seek(fi, bno, 3);
	if ((err=read(fi, buff, cnt)) != cnt) {
		printf("read error %d\n", bno);
		printf("err %x count %d\n", errno, err);
		if (sflg) {
			printf("No update\n");
/*			sflg = 0;	*/
		}
		for (ip = buff; ip < &buff[128];)		/***/
			*ip++ = 0;
	}
}

free(in)
{
	register i;
/*	int buf[128];		*/

	if (sblock.s_nfree >= 50) {		/***/
		buf[0] = sblock.s_nfree;
		for(i=0; i<50; i++)		/***/
			buf[i+1] = sblock.s_free[i];
		sblock.s_nfree = 0;
		bwrite(in, buf);
	}
	sblock.s_free[sblock.s_nfree++] = in;
}

bwrite(bno, buff)
{
	register err;

	seek(fi, bno, 3);
	if ((err=write(fi, buff, 512)) != 512) {
		printf("write error %d\n", bno);
		printf("err %x count %d\n", errno, err);
	}
}

number(as)
char *as;
{
	register n, c;
	register char *s;

	s = as;
	n = 0;
	while ((c = *s++) >= '0' && c <= '9') {
		n = n*10+c-'0';
	}
	return(n);
}

makefree()
{
	register i;

	sblock.s_nfree = 0;
	sblock.s_ninode = 0;
	sblock.s_flock = 0;
	sblock.s_ilock = 0;
	sblock.s_fmod = 0;
	free(0);
	for(i=sblock.s_fsize-1; i>=sblock.s_isize+2; i--) {
		if ((bmap[(i>>4)&(BMSIZE-1)] & (1<<(i&017)))==0)
			free(i);
	}
	bwrite(1, &sblock);
	close(fi);
	sync();
	return;
}
