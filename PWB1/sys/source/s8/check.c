#

#include	"ino.h"
#include	"filsys.h"
#define	NINODE 16
#define SIZE 15
#define	NBLK 8
#define MAXERR 8
int	bufa;
int	rootflg;
int	badfree;
int	sflg;
int	lfdes;
int	lfptr;
int	ckfile;
struct inode inode[NINODE*NBLK];
struct filsys sblock;
char	bitcnt[]
{
	4,3,3,2,3,2,2,1,
	3,2,2,1,2,1,1,0,
};
int	ndlist;
int	dlist[100];
char	*great;
char	list[512];
char	*dargv[SIZE];
char	*lptr;
int	nblist;
int	blist[100];
char	*bmap -1;
int	bmapsize;
int	nifiles;
int	fi;
int	nfile;
int	nspcl;
int	nlarg;
int	nindir;
int	ndir;
int	nused;
int	hiwat;
int	nfree;
int	ino;
int	ndup;
int	nmiss;
int	nnf	100;
int	inoerrs;
int	errs;

struct {
	char	icnt[2];
} *icnt;
struct { char *ptr; };
char	*ucnt;

struct	fname
{
	int	inum;
	int	nptr;
	int	pnum;
} *dn, *edn;

int	ldivr;
int	fout;

main(argc, argv)
char **argv;
{
	char *arg;
	register *p;
	register int nread;
	int	buf[19];

	argv[argc] = 0;
	if(argc == 1) {
		if((ckfile = open("/etc/checklist",0)) <0) {
			fprintf("Cannot open checklist\n");
			exit(1);
		}
		nread = sizeof list;
		fstat(ckfile,&buf[0]);
		if(buf[5] > nread) {
			fprintf("ERROR- Checklist to large\n");
			exit(1);
		}
		if((nread = read(ckfile,&list[0],sizeof list)) < 0) {
			fprintf("read error on checklist\n");
			exit(1);
		}
		lptr = list;
		argv = dargv;
		*argv++ = 0;
		*argv = lptr;
		while(nread-- > 0) {
			if(*lptr == '\n') {
				*lptr = '\0';
				*++argv = ++lptr;
				if(argc == SIZE) {
					fprintf("ERROR- To many args in checklist\n");
					exit(1);
				}
				argc++;
				continue;
			}
			lptr++;
		}
		dargv[argc] = 0;
		argv = dargv;
		close(ckfile);
	}

	if(argc>1 && *argv[1] == '-') {
		argc--;
		argv++;
		arg = *argv;
		while(*++arg)
		switch(*arg) {

		case 's':
			sflg++;
			continue;

		case 'i':
			dlist[ndlist++] = number(argv[1]);
			argc--;
			argv++;
			continue;

		case 'b':
			blist[nblist++] = number(argv[1]);
			argc--;
			argv++;
			continue;

		case 'g':
			great = number(argv[1]);
			argc--;
			argv++;
			continue;


		default:
			fprintf("bad option %c\n",arg[-1]);
			continue;
		}
	}
	fout = dup(1);
	argv++;
	argc--;
	fprintf("%s:\n", argv[0]);
	if(stat("/",&buf[0]) || stat(argv[0],&buf[1])) {
		fprintf("stat error\n");
	}else {
		if(buf[6+1] == buf[0]) {
			rootflg++;
		}
		check(argv[0]);
	}
	flush();
	close(fout);
	fout = 1;
	if(argc > 1) {
		execv("/etc/check",argv);
	}
}

check(file)
char *file;
{
	struct inode buf[16];
	register i, j;
	struct fname *dnp;

	fi = open(file, 0);
	if(fi < 0) {
		fprintf("cannot open %s\n", file);
		return;
	}
	sync();
	bread(1, &sblock, 512);
	if((sblock.s_isize+2) > sblock.s_fsize.ptr ||
	sblock.s_isize <0 || sblock.s_isize >4096) {
		fprintf("size check: fsize %l, isize %l\n",
		sblock.s_fsize,sblock.s_isize);
		return;
	}
	bmapsize = ((sblock.s_fsize>>3)&017777)+1;
	nifiles = sblock.s_isize*16;
	if((bmap = sbrk(bmapsize)) <0 ||
	   (icnt = sbrk(nifiles)) <0) {
		fprintf("Not enough memory\n");
		return;
	}
	for(i=2;ino<nifiles;i=+NBLK) {
		bread(i, inode, sizeof inode);
		for(j=0; j<NINODE*NBLK && ino < nifiles; j++) {
			ino++;
			pass1(&inode[j]);
		}
	}
	ino = 0;
	sync();
	bread(1, &sblock, 512);
	if(sflg) {
		close(fi);
		fi = open(file, 1);
		if(fi < 0) {
			fprintf("cannot write %s\n", file);
			return;
		}
		sblock.s_nfree = 0;
		sblock.s_tfree = 0;
		sblock.s_ninode = 0;
		sblock.s_flock = 0;
		sblock.s_ilock = 0;
		sblock.s_fmod = 0;
		free(0);
		for(i=sblock.s_fsize-1; i>=sblock.s_isize+2; i--) {
			ndup = 0;
			chk(i, "URK", 0);
			if(ndup == 0)
				free(i);
		}
		bwrite(1, &sblock);
		close(fi);
		sync();
		return;
	}
	while(i = alloc()) {
		if(chk(i, "free", 0))
			break;
		nfree++;
	}
	if(ndup)
		printf("%l dups in free\n", ndup);
	nmiss = sblock.s_fsize - sblock.s_isize - 2;
	for(i=0; i<bmapsize; i++) {
		j = bmap[i];
		nmiss =+ bitcnt[j&017];
		nmiss =+ bitcnt[(j>>4)&017];
	}
	nmiss =+ (8192-bmapsize)*8;
	if(nmiss)
		printf("%l missing\n", nmiss);
	for(i=0; i<sblock.s_isize*16; i++) {
		j = icnt->icnt[i] & 0377;
		if(j!=0 && j!=0200)
			printf("%6l %3o\n", i+1, j);
	}
	printf("spcl  %6l\n", nspcl);
	printf("files %6l\n", nfile);
	printf("large %6l\n", nlarg);
	printf("direc %6l\n", ndir);
	printf("indir %6l\n", nindir);
	printf("used  %6l\n", nused);
	printf("last  %6l\n", hiwat);
	printf("free  %6l\n", nfree);
	close(fi);
}


pass1(ip)
struct inode *ip;
{
	int buf[256];
	register i, j, df;
	int k;

	if((ip->i_mode&IALLOC) == 0){
		if(ip->i_mode != 0) fprintf("INODE - BAD FORMAT : inode =%o\n",
			ino);
		return;
	}
	icnt->icnt[ino-1] =+ 0100;
	if(ip->i_nlink)
		icnt->icnt[ino-1] =+ 0100 + ip->i_nlink;
	if((ip->i_mode&IFCHR&IFBLK) != 0) {
		nspcl++;
		return;
	}
	inoerrs = 0;
	df = 0;
	if((ip->i_mode&IFMT) == IFDIR) {
		df = (ip->i_size1>>4)&017777;
		ndir++;
		if(ip->i_size0) {
			fprintf("directory bigger than 2^16: inode=%l\n",ino);
		}
	}
	nfile++;
	if((ip->i_mode&ILARG) != 0) {
		nlarg++;
		for(i=0; i<8; i++)
		if(ip->i_addr[i] != 0) {
			nindir++;
			if(chk(ip->i_addr[i], "indirect", 0))
				continue;
			bread(ip->i_addr[i], buf, 512);
			for(j=0; j<256; j++) {
				if(buf[j] != 0)
					chk(buf[j], "data (large)", df);
				if (df > 0) df =- 32;
			}
		}
	}else {
		for(i=0; i<8; i++) {
			if(ip->i_addr[i] != 0)
				chk(ip->i_addr[i], "data (small)", df);
			df =- 32;
		}
	}
	if(inoerrs > MAXERR)
		fprintf("excessive errors: inode= %l\n",ino);
}

chk(ii, s, df)
char *ii;
{
	register char *i;
	register n, j;
	int b;
	int buf[256];
	struct {
		int	ino;
		char	name[14];
	};

	i = ii;
	for(j=0; j<nblist; j++)
		if(i == blist[j])
			fprintf("%l blk: inode=%l(%s)\n", i, ino, s);
	if(great && i >= great && i)
		fprintf("%l geq; inode=%l(%s)\n", i, ino, s);
	if(ino) {
		nused++;
		if(i > hiwat)
			hiwat = i;
	}
	if(i<sblock.s_isize+2 || i>=sblock.s_fsize) {
		efprintf("%l bad; inode=%l(%s)\n", i, ino, s);
		return(1);
	}
	n = ldiv(0, i, 8);
	j = (1<<ldivr);
	if(bmap[n] & j) {
		if(ino == 0) {
			ndup++;
			return(0);
		}
		efprintf("%l dup; inode=%l(%s)\n", i, ino, s);
		return(1);
	}
	bmap[n] =| j;
	if(df>0) {
		bread(i, buf, 512);
		for(n=0; n<256; n=+8) {
			if(df <= 0)
				break;
			df--;
			if((b=buf[n]) == 0)
				continue;
			for(j=0; j<ndlist; j++)
				if(b == dlist[j])
				fprintf("%l ino: inode=%l(%s) \"%.16s\"\n",
				  b, ino, s, buf+n+1);
			if(b<1 || b> nifiles) {
				efprintf("%l din: inode=%l(%s)\n", i, ino, s);
				continue;
			}
			icnt->icnt[b-1]--;
		}
	}
	return(0);
}

alloc()
{
	register b, i;
	int buf[256];

	i = --sblock.s_nfree;
	if(i<0 || i>=100) {
		badfree++;
		fprintf("bad freeblock\n");
		return(0);
	}
	b = sblock.s_free[i];
	if(b == 0)
		return(0);
	if(sblock.s_nfree <= 0) {
		bread(b, buf, 512);
		sblock.s_nfree = buf[0];
		for(i=0; i<100; i++)
			sblock.s_free[i] = buf[i+1];
	}
	return(b);
}

bread(bno, buf, cnt)
int	*buf;
{

	seek(fi, bno, 3);
	if(read(fi, buf, cnt) != cnt) {
		fprintf("read error %d\n", bno);
		exit(1);
	}
}

free(in)
{
	int i;
	int buf[256];

	if(sblock.s_nfree >= 100) {
		buf[0] = sblock.s_nfree;
		for(i=0; i<100; i++)
			buf[i+1] = sblock.s_free[i];
		sblock.s_nfree = 0;
		bwrite(in, buf);
	}
	sblock.s_free[sblock.s_nfree++] = in;
	if(in) sblock.s_tfree++;
}

bwrite(bno, buf)
{

	seek(fi, bno, 3);
	if(write(fi, buf, 512) != 512) {
		fprintf("write error %d\n", bno);
		exit(1);
	}
}

number(s)
char *s;
{
	int n, c;

	n = 0;
	while(c = *s++) {
		if(c<'0' || c>'9')
			continue;
		n = n*10+c-'0';
	}
	return(n);
}

fprintf(a, b, c, d, e)
{
	printf(a, b, c, d, e);
	flush();
}

efprintf(a, b, c, d, e)
{
	errs++;
	if(inoerrs >= MAXERR)
		return;
	inoerrs++;
	printf(a, b, c, d, e);
	flush();
}
