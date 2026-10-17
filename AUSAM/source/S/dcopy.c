#
#include <local-system>
/*
 *	disk copier and compacter
 *	J. N. Rottman
 *
 *	NOTE:
 *	1) Boostrap block is copied.
 *	2) Size of destination file system (in particular no inodes)
 *	   is used i.e it is not duplicated. An initial pass is thus
 *	   made to check if copied filesystem fits !!
 *	3) Disk errors are also copied, so dcheck and icheck aforehand.
 *	4) Will work for HP and MS disks only (agsm and eecf respectively).
 *
 *	C. H. Maltby esq.
 */

/*
 *	5) Will now work for any disk (e.g. RK).  The algorithm was
 *	   plagiarised from `icheck.c'.
 *	6) Added directory-compression feature.
 *
 *	K. F. Hill (failed)
 */

#include <param.h>
#include <filsys.h>
#include <ino.h>

#ifdef	EECF
#define	IN	10	/* magic interleave number */
#define NSEC	21	/* number of sectors per track */
#define NTRACK	19	/* number of tracks per cylinder */
#endif	EECF
#ifdef	AGSM
#define	IN	7	/* magic interleave number */
#define NSEC	22	/* number of sectors per track */
#define NTRACK	19	/* number of tracks per cylinder */
#endif	AGSM
#ifdef	EECF1140
#define	IN	3	/* magic interleave number */
#define	NSEC	12	/* number of sectors per track */
#define NTRACK	2	/* number of tracks per cylinder */
#endif	EECF1140

#ifdef	EECF & AGSM | EECF & EECF1140 | AGSM & EECF1140
more than one is defined .... this should not bee ;;
#endif

#define	NPOOL	50	/* size of output buffer pool - must be >= 8 */


struct	filsys	super0;
struct	filsys	super1;
struct	inode	node0;
struct	inode	node1;

struct	direct
{
	int	inum;
	char	name[14];
};

struct	inode	in_node[16];
struct	inode	out_node[16];

int	inum	16;
int	iblk	1;
int	onum	0;
int	oblk	2;
int	iunit;
int	*iindex;
int	fi;
int	fo;
int	lastblock;
int	inext	1;
int	in[256];
int	ind0[256];
int	ind1[256];
char	*blkno;


int dirent;		/* number of directory entries */
int diriaddr;		/* index into i_addr for new large directories */
int dirind1;		/* index into ind1 for new block numbers */
int *upto;		/* remembers where up to in new block */


struct
{
	int	bn;
	int	bb[256];
} bpl[NPOOL];

int	inpool;


char	adr[NSEC], flag[NSEC];


main(argc, argv)
char **argv;
{
	register i, *indx, icount;
	extern int *sbrk();

	if (argc != 3)
	{
		printf("Usage: dcopy fromfilesys tofilesys\n");
		return(1);
	}
	if ((fi = open(argv[1], 0)) < 0)
	{
		printf("Cannot open %s.\n", argv[1]);
		return(1);
	}
	if ((fo = open(argv[2], 2)) < 0)
	{
		printf("Cannot open %s.\n", argv[2]);
		return(1);
	}
	blkno = argv[0];
	indx = 0;
	if (gtty(0, in) == -1) indx++;		/* terminal input */
	sync();
	getblk(fi, &super0, 0);
	putblk(&super0, 0);		/* copy bootstrap */
	getblk(fi, &super0, 1);

	if (indx == 0)
	{
		printf("%s: size = %d blks (%d in use); i-list = %d blks (%d in use).\n",
			argv[1], super0.s_fsize, super0.s_fsize - (icount = countfree()), super0.s_isize, i = counti());
		for (;;)
		{
			printf("Enter size (blks) for %s file system ", argv[2]);
			if ((super1.s_fsize = getnum(super0.s_fsize)) < i)
			{
				printf("Too small\n");
				continue;
			}
			printf("Enter size (blks) of i-list (16 inodes per blk) ");
			if ((super1.s_isize = getnum(super0.s_isize)) < i || super1.s_isize >= super1.s_fsize)
			{
				printf("Illegal size\n");
				continue;
			}
			if ((super0.s_fsize - (super0.s_isize + icount)) <= (super1.s_fsize - super1.s_isize)) break;
			printf("Insufficient space for file system.\n");
		}

		if ((i = fork()) < 0)
			printf("Fork has failed!! Parent will carry on...\n");
		if (i > 0)
		{
			printf("Child continuing dcopy (pid %d)\n", i);
			return 0;
		}
		if (i == 0)
		{
			signal(2, 1);
			signal(3, 1);
		}
	}
	else
	{
		super1.s_fsize = super0.s_fsize;
		super1.s_isize = super0.s_isize;
	}

	super1.s_time = super0.s_time;
	icount = 0;
	for (i = 0; i < NSEC; i++)
	{
		while (flag[icount]) icount = (icount + 1) % NSEC;
		adr[i] = icount;
		flag[icount]++;
		icount = (icount + IN) % NSEC;
	}

	indx = i = sbrk(super0.s_isize * 2 * 16);
	if (i == -1)
	{
		printf("Dcopy: Not enough core!\n");
		return 1;
	}
	iindex = indx - 1;
	icount = 16*super0.s_isize;


	iunit = fi;

	i = icount;
	do
	{
		geti();
		if (node0.i_mode & IALLOC)
		{
			copyi();
			*indx++ = inext++;
		}
		else *indx++ = 0;
	} while (--i);

	ipurge();

	for (i=1; i < inext; i++)
	{
		geti();
		if ((node0.i_mode & IFMT) == IFDIR) reldir();
	}

	icount = 16 * super1.s_isize;
	i = inext;
	while (i <= icount && super1.s_ninode != 100)
		super1.s_inode[super1.s_ninode++] = i++;

	for (indx = &in[100]; indx != &in[256]; *indx++ = 0);
	freelist();

	putblk(&super1, 1);
	purgepool();
	sync();
	printf("\007Dcopy finished\n");
	return(0);
}

getnum(dflt)
{
	register unsigned n;
	register c;

	for (;;)
	{
		n = 0;
		if ((c = getchar()) == 0 || c == '\n') n = dflt;
		else while (c >= '0' && c <= '9')
		{
			n = n * 10 + c - '0';
			c = getchar();
		}
		if (c == 0 || c == '\n') return n;
		while ((c = getchar()) && c != '\n');
		printf("\007Illegal number - try again ");
	}
}

getblk(f, b, n)
{
	seek(f, n, 3);
	if (read(f, b, 512) != 512) printf("Input error: block %d\n", n);
}

putblk(b, n)
register int *b;
{
	register int *w;
	register int q;

	if (inpool == NPOOL) purgepool();
	bpl[inpool].bn = n;
	w = bpl[inpool++].bb;
	q = 256;
	do *w++ = *b++; while (--q);
}

grablock(n)	/* grabs the next free block from the pool */
{
	if (inpool == NPOOL) purgepool();
	bpl[inpool].bn = n;
	return(&bpl[inpool++].bb);
}

purgepool()
{
	register int i;

	for (i=0; i < inpool; i++)
	{
		seek(fo, bpl[i].bn, 3);
		if (write(fo, bpl[i].bb, 512) != 512)
			printf("Output error: block %d\n", bpl[i].bn);
	}
	inpool = 0;
}

geti()
{
	register *r, *w, q;

	if (inum == 16)
	{
		getblk(iunit, in_node, ++iblk);
		decnum(blkno, iblk);
		inum = 0;
	}
	w = &node0;
	r = &in_node[inum++];
	q = 16;
	do *w++ = *r++; while (--q);
}

decnum(s, n)
register char *s;
register int n;
{
	if (n/10 != 0) s = decnum(s, n/10);
	*s++ = n%10 + '0';
	*s = 0;
	return(s);
}

puti()
{
	register *r, *w, q;

	w = &out_node[onum];
	r = &node1;
	q = 16;
	do *w++ = *r++; while (--q);
	if (++onum == 16)
	{
		putblk(out_node, oblk++);
		onum = 0;
	}
}

ipurge()
{
	register *r, *w, q;

	if (onum)
	{
		r = &out_node[onum];
		q = 16*(16-onum);
		do *r++ = 0; while (--q);
		putblk(out_node, oblk++);
	}

	r = out_node;
	q = 256;
	do *r++ = 0; while (--q);

	while (oblk < 2 + super1.s_isize) putblk(out_node, oblk++);

	iblk = 1;
	inum = 16;
	iunit = fo;
	purgepool();
}

nxtblk()
{
	static int cyl, sector, track;
	register char *block;

	for (;;)
	{
		lastblock = block = (cyl * NTRACK + track) * NSEC + adr[sector];
		if (++sector >= NSEC)
		{
			sector = 0;
			if (++track >= NTRACK)
			{
				cyl++;
				track = 0;
			}
		}
		if (block >= 2 + super1.s_isize) return(block);
	}
}

freelist()
{
	register cyl, sector, track;
	extern int ldivr;
	char *block;

	cyl = ldiv(0, super1.s_fsize, NTRACK*NSEC);
	track = ldivr / NSEC;		/* first non-usable block */
	sector = ldivr % NSEC;
	bfree(0);			/* end-of-list sentinel */

	while (sector) bfree((cyl * NTRACK + track) * NSEC + --sector);
	for (;;)
	{
		if (sector-- == 0)
		{
			sector =+ NSEC;
			if (track-- == 0)
			{
				track =+ NTRACK;
				cyl--;
			}
		}
		block = (cyl * NTRACK + track) * NSEC + adr[sector];
		if (block == lastblock) return;
		if (block >= 2 + super1.s_isize) bfree(block);
	}
}

bfree(b)
{
	register *r, *q, w;
	if (super1.s_nfree == 100)
	{
		r = in;
		*r++ = super1.s_nfree;
		q = super1.s_free;
		w = 100;
		do *r++ = *q++; while (--w);
		putblk(in, b);
		super1.s_nfree = 0;
	}
	super1.s_free[super1.s_nfree++] = b;
}

copyi()
{
	register int w, ww;
	int *r, *q;

	r = &node0;
	q = &node1;
	w = 16;
	do *q++ = *r++; while (--w);

	if (w = node1.i_mode & IFMT)
	{
		if (w == IFDIR) copydir();
		puti();
		return;
	}

	if ((node1.i_mode&ILARG) == 0)
	{
		for (w = 0; w < 8; w++) 
			if (node1.i_addr[w])
			{
				getblk(fi, in, node1.i_addr[w]);
				node1.i_addr[w] = nxtblk();
				putblk(in, node1.i_addr[w]);
			}
		puti();
		return;
	}
	for (w = 0; w < 7; w++) 
		if (node1.i_addr[w])
		{
			getblk(fi, ind0, node1.i_addr[w]);
			node1.i_addr[w] = nxtblk();
			for (ww = 0; ww < 256; ww++)
				if (ind0[ww])
				{
					getblk(fi, in, ind0[ww]);
					ind0[ww] = nxtblk();
					putblk(in, ind0[ww]);
				}
			putblk(ind0, node1.i_addr[w]);
		}

	if (node1.i_addr[7])
	{
		getblk(fi, ind0, node1.i_addr[7]);
		node1.i_addr[7] = nxtblk();
		for (w = 0; w < 256; w++)
			if (ind0[w])
			{
				getblk(fi, ind1, ind0[w]);
				ind0[w] = nxtblk();
				for (ww = 0; ww < 256; ww++)
					if (ind1[ww])
					{
						getblk(fi, in, ind1[ww]);
						ind1[ww] = nxtblk();
						putblk(in, ind1[ww]);
					}
				putblk(ind1, ind0[w]);
			}
		putblk(ind0, node1.i_addr[7]);
	}
	puti();
}

putdir(blk)
{
	register *to, *from, cntr;
	int count;

	to = upto;
	getblk(fi, from = in, blk);
	for (count = 0; count < 32; count++)
	{
		if (! *from)		/* valid entry ? */
		{
			from =+ 8;
			continue;
		}
		if ((dirent & 037) == 0)
		{
			if (diriaddr == -1 && dirent == 256)	/* convert to large file */
			{
				node1.i_mode =| ILARG;
				node1.i_addr[diriaddr = 0] = ind1[0];
				for (cntr = 0; cntr < 7; cntr++)	/* at least 8 buffers have been guaranteed */
					bpl[inpool + cntr - 8].bn = ind1[cntr] = ind1[cntr + 1];
				bpl[inpool - 1].bn = ind1[7] = nxtblk();
			}
			else if (diriaddr != -1 && (dirent & 017777) == 0)
			{
				putblk(ind1, node1.i_addr[diriaddr]);
				node1.i_addr[++diriaddr] = nxtblk();
				dirind1 = 0;
			}
			to = grablock(ind1[dirind1++] = nxtblk());
		}
		for (cntr = 0; cntr < 8; cntr++) *to++ = *from++;	/* copy name */
		dirent++;
	}
	upto = to;
}

copydir()
{
	register w, ww;
	register *p;

	dirent = 0;
	diriaddr = -1;
	dirind1 = 0;
	if ((node1.i_mode & ILARG) == 0)
	{
		for (w = 0; w < 8; w++) if (node1.i_addr[w]) putdir(node1.i_addr[w]);
	}
	else
	{
		if (NPOOL - inpool < 8) purgepool();	/* guarantees 8 buffers - see `putdir' */
		node1.i_mode =& ~ILARG;
		for (w = 0; w < 7; w++)
			if (node1.i_addr[w])
			{
				getblk(fi, ind0, node1.i_addr[w]);
				for (ww = 0; ww < 256; ww++)
					if (ind0[ww]) putdir(ind0[ww]);
			}
		if (diriaddr != -1) while (dirind1 < 256) ind1[dirind1++] = 0;	/* clear rest of pointers */
	}
	node1.i_size0 = (dirent >> 12) & 017;
	node1.i_size1 = dirent << 4;
	if (diriaddr == -1)
	{
		w = -1;
		for (ww = 0; ww < dirind1; node1.i_addr[++w] = ind1[ww++]);
	}
	for (p = upto; p < &bpl[inpool]; *p++ = 0);	/* clear rest of directory */
	if (diriaddr != -1) putblk(ind1, node1.i_addr[w = diriaddr]);
	while (w < 7) node1.i_addr[++w] = 0;
}

reldir()
{
	register int i, j;

	if ((node0.i_mode & ILARG) == 0)
	{
		for (i=0; i < 8; i++) relx(node0.i_addr[i]);
		return;
	}
	for (i=0; i < 7; i++)
		if (node0.i_addr[i])
		{
			getblk(fo, ind0, node0.i_addr[i]);
			for (j = 0; j < 256; j++) relx(ind0[j]);
		}
}

relx(b)
{
	register struct  direct *p;
	register int i;

	if (b == 0) return;
	getblk(fo, in, b);
	p = in;
	i = 32;
	do
	{
		if (p->inum) p->inum = iindex[p->inum];
		p++;
	} while (--i);
	putblk(in, b);
}

counti()
{
	register i,j,n;

	n = 0;
	for (i = 0; i < super0.s_isize; i++)
	{
		getblk(fi, &in_node, 2+i);
		for (j = 0; j < 16; j++)
			if (in_node[j].i_mode & IALLOC) n++;
	}
	return((n + 15) / 16);
}

countfree()
{
	register n,m;

	n = super0.s_nfree;
	if (n)
	{
		m = super0.s_free[0];
		while (m)
		{
			getblk(fi, in, m);
			n =+ in[0];
			m = in[1];
		}
	}
	return(n);
}

printf(s, a)
register char *s;
{
	register c, *p;

	p = &a;
	while (c = *s++)
	{
		if (c == '%')
		{
			if ((c = *s++) == 'd')
			{
				printl(*p++);
				continue;
			}
			if (c == 's')
			{
				printf(*p++);	/* CARE !! */
				continue;
			}
			if (c == 0) return;
		}
		putchar(c);
	}
}

printl(n)
register unsigned n;
{
	register i;

	i = n % 10;
	if (n =/ 10) printl(n);
	putchar(i + '0');
}
