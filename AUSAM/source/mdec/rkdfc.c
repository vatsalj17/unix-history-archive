#

/*	This program created by Ian J.	July '76
 *
 *	Performs certain necessary i/o functions for rk05 disks:
 *
 *	  h for help
 *	  c for copy function
 *	  f for format function
 *	  k for compare function
 *	  r for read function
 */

/*	Modified by Kevin Hill		Sep-Oct 78
 *
 *	1.	Corrected a bug which caused `rkdf' to lie about block nrs.
 *	2.	Extra commands added:
 *
 *		    `a' for assisting with drive alignment;
 *		    `b' for reboot (asks for a drive number);
 *		    `d' for the J. Rottman `dcopy' program (as modified
 *			by C. Maltby / K. Hill).
 *
 *	3.	Altered code slightly so that `help' is printed out at start.
 *	4.	Rewrote `gdrivnum' and `rkio' to avoid use of goto's.
 *	5.	Included a smaller `printf' - only `%d', `%o', and `%s' supported.
 *	6.	Added DELETE facility.
 *	7.	Added checks to detect:
 *
 *		    if selected drive is active;
 *		    if drive selected to be written on is write-protected;
 *		    if same drive numbers for compare (==> exit) or copy
 *			(==> copy-in-situ on a cylinder-by-cylinder basis).
 */

#define	DELETE	0177

#define	NRETRYS	10	/* number of attempts at disk i/o before collapsing in a heap */

#define	SSR0	0177572
#define	KISA0	0172340
#define	KISA6	0172354
#define	KISD0	0172300
#define	SWREG	0177570
struct { int i; };	/* structure to access integers */


#define	TT	0177560
#define	DONE	0200

struct
{
	int	csr;
	int	csb;
	int	tsr;
	int	tsb;
};


#define	RK	0177400

#define	NCYL		203
#define	BLKPERTRK	12
#define	NTRACK		2
#define	BLKPERCYL	NTRACK * BLKPERTRK
#define	WDSPERCYL	256 * BLKPERCYL
#define	BLKPERDSK	NCYL * BLKPERCYL	/* 4872 */

struct
{
	int	rkds;
	int	rker;
	int	rkcs;
	int	rkwc;
	int	rkba;
	int	rkda;
};

/* rk i/o requests */

#define	CNTRLR		   01		/* control reset */
#define	WRITE		   03	
#define	READ		   05
#define	WRITECHK	   07
#define	SEEK		  011
#define	READCHK		  013
#define	WRITEPROT	  017
#define	SSE		 0400		/* stop on soft error */
#define	FMTW		02003		/* format write */
#define	IBA		04000		/* inhibit incrementing RKBA */

#define	WTPROT		 040		/* write protect bit */
#define	DRY		0200		/* drive ready bit */


char *bigbuf;		/* pointer to buffer area */
int	iob;		/* number of blocks per big i/o */
int	nio;		/* number of i/o for these blocks */
int    szio;		/* number of words for each of above i/os */
int	iov;		/* number of blocks left after the big ones */
int    szov;		/* number of words for this last i/o */

int in, out;		/* drives for current input/output operations */
int inform, outform;	/* optimised-format flags - for dcopy */
int base;		/* base of number to be printed out */
int kludge;		/* prints out help first time only */



main()
{
	register i;

	setexit();
	if (kludge++ == 0) help();
	for (;;)
	{
		printf("\007\nWhich function is desired? \007");
		i = getchar();
		putchar('\n');
		switch(i)
		{
	    case 'a':	in = gdrivnum("Drive# ", 0);
			wrlock(in);
			align();	/* no return */

	    case 'b':	printf("Switches = %o - ", SWREG->i);
			base = gdrivnum("Boot from drive# ", 0) << 13;
			putchar(0); putchar(0);		/* flush */
			reboot();		/* no return */

	    case 'c':	in = gdrivnum("Read from drive# ", 0);
			out = gdrivnum(" Write to drive# ", 1);
			if (in == out)
			{
				if (iob >= BLKPERCYL)
				{
					reed(in);
					cpyinsitu();
					break;
				}
				printf("\007Not enough core to copy-in-situ.\n");
				break;
			}
			wrlock(in);
			reed(in);
			format();
			copy();
			check();
			break;

	    case 'd':	in = gdrivnum("Read from drive# ", 0);
			inform = formtype();
			out = gdrivnum("Write to drive# ", 1);
			if (in == out)
			{
				printf("Same drives (?)\n");
				break;
			}
			outform = formtype();
			wrlock(in);
			dcopy();		/* does the reed and format */
			break;

	    case 'f':	out = gdrivnum("Format drive# ", 1);
			format();
			reed(out);
			break;

	    case 'k':	in = gdrivnum("   Read from drive# ", 0);
			out = gdrivnum("Compare with drive# ", 0);
			if (in == out) printf("\007Same drives.\n");
			else check();
			break;

	    case 'r':	in = gdrivnum("Read from drive# ", 0);
			wrlock(in);
			reed(in);
			break;

	    default:	help();
		}

        }
}

help()
{
	printf("\nYou are reqd. to enter a char\n");
	printf("to indicate the function you want\n");
	printf("        a -- align\n");
	printf("        b -- reboot\n");
	printf("        c -- copy\n");
	printf("        d -- dcopy\n");
	printf("        f -- format\n");
	printf("        h -- help\n");
	printf("        k -- compare\n");
	printf("        r -- read\n");
}

wrlock(unit)
register unit;
{
	rkio(unit, 0, 0, WRITEPROT, 0);
	printf("Drive %d write protected.\n", unit);
}

format()
{
	register i, unit;

	bigbuf[0] = 0;
	printf("Begin formatting drive %d.\n", unit = out);
	for(i = 0; i < nio; i++) rkio(unit, bigbuf, iob * i, FMTW | IBA, szio);
	rkio(unit, bigbuf, iob * i, FMTW | IBA, szov);
	printf("Formatting complete.\n");
}

cpyinsitu()
{
	register i, unit;

	printf("Begin copying drive %d in situ.\n", unit = in);
	for (i = 0; i < NCYL; i++)
	{
		rkio(unit, bigbuf, i * BLKPERCYL, READ | SSE, WDSPERCYL);
		rkio(unit, bigbuf, i * BLKPERCYL, FMTW, WDSPERCYL);
		rkio(unit, bigbuf, i * BLKPERCYL, WRITECHK | SSE, WDSPERCYL);
	}
	wrlock(unit);
	printf("Copying complete.\n");
}

copy()
{
	register i, iunit, ounit;

	printf("Begin copying drive %d to drive %d.\n", iunit = in, ounit = out);
	for (i = 0; i < nio; i++)
	{
		rkio(iunit, bigbuf, i * iob, READ | SSE, szio);
		rkio(ounit, bigbuf, i * iob, WRITE | SSE, szio);
		rkio(ounit, bigbuf, i * iob, WRITECHK | SSE, szio);
	}
	rkio(iunit, bigbuf, i * iob, READ | SSE, szov);
	rkio(ounit, bigbuf, i * iob, WRITE | SSE, szov);
	rkio(ounit, bigbuf, i * iob, WRITECHK | SSE, szov);
	printf("Copy complete.\n");
}

check()
{
	register i, iunit, ounit;

	wrlock(iunit = in);
	wrlock(ounit = out);
	printf("Begin comparing drive %d to drive %d.\n", iunit, ounit);
	for (i = 0; i < nio; i++)
	{
		rkio(iunit, bigbuf, i * iob, READ | SSE, szio);
		rkio(ounit, bigbuf, i * iob, WRITECHK | SSE, szio);
	}
	rkio(iunit, bigbuf, i * iob, READ | SSE, szov);
	rkio(ounit, bigbuf, i * iob, WRITECHK | SSE, szov);
	printf("Compare complete.\n");
}

reed(unit)
register unit;
{
	register i;

	printf("Begin reading drive %d.\n", unit);
	for (i = 0; i < nio; i++) rkio(unit, bigbuf, i * iob, READCHK | SSE, szio);
	rkio(unit, bigbuf, i * iob, READCHK | SSE, szov);
	printf("Read complete.\n");
}

align()
{
	register block, unit;
	register char c;

	unit = in;
	for (;;)
	{
		printf("Cylinder# ");
		block = getnum(0, NCYL, 0) * BLKPERCYL;
		printf("0 = top head; 1 = bottom head; 'c' for new cyl; DEL to exit.\n");
		for (;;)
		{
			printf("Head# ");
			c = getchar();
			putchar('\n');
			if (c == 'c') break;
			if (c != '0' && c != '1')
			{
				printf("\007Illegal head.\n");
				continue;
			}
			rkio(unit, 0, 0, WRITEPROT, 0);		/* safety first! */
			rkio(unit, 0, c == '1' ? block + BLKPERTRK : block, SEEK | SSE, 0);
		}
	}
}

rkio(unit, buffer, block, cmd, wc)
register unit, block, wc;
{
	int nretry;

	for (nretry = 0; nretry < NRETRYS; nretry++)
	{
		while ((RK->rkcs & DONE) == 0);
		RK->rkda = ((block / 12) << 4) | (block % 12) | (unit << 13);
		RK->rkba = buffer;
		RK->rkwc = -wc;
		RK->rkcs = cmd;
		while ((RK->rkcs & DONE) == 0);
		if (RK->rkcs >= 0) return;
		printf("\007Error on drive %d block=%d. -- ds=%o da=%o cs=%o er=%o\n",
			(RK->rkda >> 13) & 07,
			12 * ((RK->rkda & 017760) >> 4) + (RK->rkda & 017),
			RK->rkds, RK->rkda, RK->rkcs, RK->rker);
		RK->rkcs = CNTRLR;
	}
	printf("\nRecovery impossible.\n");
	putchar(0); putchar(0);		/* flush */
	restart();		/* no return */
}

/********************************************************/


getnum(lower, upper, dflt)		/* returns lower <= num < upper */
register unsigned upper;
{
	register unsigned n;
	register c;

	for (;;)
	{
		n = 0;
		if ((c = getchar()) == '\r')
		{
			if (dflt < upper) n = dflt;
		}
		else while (c >= '0' && c <= '9' && (n = n * 10 + c - '0') < upper) c = getchar();
		putchar('\n');
		if (c == '\r' && n >= lower) return(n);
		printf("\007Illegal entry - try again ");
	}
}

formtype()
{
	register c;

	printf("Optimised format? (y-n) ");
	c = (getchar() == 'y');
	putchar('\n');
	return(c);
}

gdrivnum(s, chkwrprt)
register char *s;
register chkwrprt;
{
	register c;

	for (;;)
	{
		printf(s);
		c = getchar() - '0';
		putchar('\n');
		if (c < 0 || c > 7)
		{
			printf("\007Illegal drive.\n");
			continue;
		}
		RK->rkda = c << 13;
		if ((RK->rkds & DRY) == 0)
		{
			printf("\007Drive not active.\n");
			continue;
		}
		if (chkwrprt && (RK->rkds & WTPROT))
		{
			printf("\007Drive write protected.\n");
			continue;
		}
		return(c);
	}
}

printf(s, a)
register char *s;
{
	register c, *p;

	p = &a;
	while (c = *s++)
	{
		if (c == '%')
			switch (c = *s++)
			{
		    case 's':	printf(*p++);	/* CARE! */
				continue;

		    case 'o':	base = 8;
				printl(*p++);
				continue;

		    case 'd':	base = 10;
				printl(*p++);
				continue;

		    case 0:	return;
			}
		putchar(c);
	}
}

printl(n)
register unsigned n;
{
	register i;

	i = n % base;
	if (n =/ base) printl(n);
	putchar(i + '0');
}

putchar(c)
register char c;
{
	while ((TT->tsr & DONE) == 0);
	TT->tsb = c;
	if (c == '\n') putchar('\r');
}

getchar()
{
	register c;

	while ((TT->csr & DONE) == 0);
	if ((c = TT->csb & 0177) == DELETE)
	{
		putchar('\n');
		reset();
	}
	putchar(c);
	if (c >= 'A' && c <= 'Z') c =| 040;
	return(c);
}

/********************************************************/


#include	<local-system>
#include	<defines.h>
#include	<param.h>
#include	<filsys.h>
#include	<ino.h>


#define	INTL		3	/* magic interleaving number */

#define	IN_NODE		sizeof *superblk
#define	OUT_NODE	IN_NODE + 16 * sizeof *in_node
#define	GENERAL		OUT_NODE + 16 * sizeof *out_node
#define	IND0		GENERAL + 512
#define	IND1		IND0 + 512
#define	INDX		IND1 + 512

struct buffer
{
	int bn;		/* block number */
	int bb[256];	/* block */
} *startbuff, *currbuff;

struct filsys *superblk;	/* target super block */
struct inode *in_node;		/* inode blocks from source */
struct inode *out_node;		/* inode blocks to target */
int *general;			/* general usage */
int *ind0;			/* first indirect block */
int *ind1;			/* second indirect block */
int *indx;			/* start of dynamic array area */
int *iindex;			/* used with indx */
char adr[BLKPERTRK];		/* interleaving array */
int	nbuf;			/* number of buffers in the pool */
int	inpool;			/* number of used buffers */
struct	inode	*node;		/* the current inode being considered */
int	isize_sav;		/* i-list size from the source super block */
long	time_sav;		/* time from the source super block */
int	form;			/* format of current input unit */
int	upto;			/* where up to when compacting directories */
unsigned dirent;		/* counts the new directory entries */
int	*diraddr;		/* where up to in the current inode */
int	*dirind;		/* where up to in indirect block */
int	inext;
int	inum, iblk, onum, oblk;
int	lastblock, cyl, sector, track;

dcopy()
{
	register char *ptr1;
	register *ptr2;
	register icount;
	int isize1;

	superblk = bigbuf;
	in_node	= &bigbuf[IN_NODE];
	out_node = &bigbuf[OUT_NODE];
	general	= &bigbuf[GENERAL];
	ind0	= &bigbuf[IND0];
	ind1	= &bigbuf[IND1];
	indx	= &bigbuf[INDX];
	iindex = indx - 1;
	inum = 16;
	iblk = 1;
	onum = 0;
	oblk = 2;
	inpool = 0;
	cyl = sector = track = 0;

	form = inform;
	getblk(in, superblk, 1);		/* source super block */
	isize_sav = superblk->s_isize;
	time_sav = superblk->s_time;


/*	Test to see if there is room for the program plus reserved buffers
 *	plus at least 8 buffers in the pool, in the program's address space,
 *	and in the machine's core.
 */

	ptr1 = &bigbuf[INDX] + superblk->s_isize * 2 * 16;
	inext = 8 * sizeof *startbuff;
	if (ptr1 + inext < &bigbuf[INDX] || ((ptr1 + inext + 077) >> 6) > (icount = KISA6->i))
	{
		printf("Not enough core.\n");
		return;
	}
	inext = 1;
	if (icount > 01600) icount = 01600;		/* i/o page */
	icount = (icount << 6) - ptr1;
	currbuff = startbuff = ptr1;
	nbuf = ldiv(0, icount, sizeof *startbuff);
	printf("%d buffers in pool.\n\n", nbuf);

	printf("Source: size = %d blks (%d in use); i-list = %d blks (%d in use).\n",
		superblk->s_fsize, superblk->s_fsize - (ptr1 = countfree()), superblk->s_isize, ptr2 = counti());
	for (;;)
	{
		printf("Enter size (blocks) of target volume (%d less swap space) ", BLKPERDSK);
		icount = getnum(ptr2, BLKPERDSK + 1, superblk->s_fsize);
		printf("Enter size (blocks) of target i-list (16 inodes per block) ");
		isize1 = getnum(ptr2, icount, superblk->s_isize);
	
		if ((superblk->s_fsize - (superblk->s_isize + ptr1)) <= (icount - isize1)) break;
		printf("Insufficient file space.\n");
	}

	reed(in);
	format();		/* secondary effect is to clear the inodes */

	for (ptr2 = superblk; ptr2 < &superblk[1]; *ptr2++ = 0);	/* clear target super block */
	superblk->s_fsize = icount;
	superblk->s_isize = isize1;
	superblk->s_time = time_sav;

	ptr1 = "optimised ";
	printf("Begin dcopy from %sdrive %d to %sdrive %d.\n",
		inform ? ptr1 : "", in, outform ? ptr1 : "", out);
	getblk(in, grablock(0), 0);		/* bootstrap block */

	for (ptr2 = general; ptr2 < &general[BLKPERTRK]; *ptr2++ = 0);
	icount = 0;
	for (ptr1 = adr; ptr1 < &adr[BLKPERTRK]; )	/* set up interleaving template */
	{
		while (general[icount]) icount = (icount + 1) % BLKPERTRK;
		*ptr1++ = icount;
		general[icount]++;
		icount = (icount + INTL) % BLKPERTRK;
	}

	printf("Copy files.\n");
	icount = 16 * isize_sav;
	do
	{
		geti();
		if (node->i_mode & IALLOC)
		{
			copyi();
			*indx++ = inext++;
		}
		else *indx++ = 0;
	} while (--icount);

	printf("Complete i-list.\n");
	if (onum)
	{
		ptr1 = &out_node[onum];
		icount = 32 * (16 - onum);
		do *ptr1++ = 0; while (--icount);
		putblk(out_node, oblk);
	}
	purgepool();		/* remaining inodes zero due to the format */

	printf("Correct directory entries.\n");
	iblk = 1;
	inum = 16;
	in = out;		/* so `geti' gets from target */
	form = outform;
	for (icount = 0; icount < inext; icount++)
	{
		geti();
		if ((node->i_mode & IFMT) == IFDIR) reldir();
	}

	printf("Construct the free list.\n");
	ptr1 = 16 * isize1;
	icount = inext;
	while (icount <= ptr1 && superblk->s_ninode != 100)
		superblk->s_inode[superblk->s_ninode++] = icount++;
	ptr2 = &general[100];
	for (icount = 100; icount < 256; icount++) *ptr2++ = 0;
	freelist();
	putblk(superblk, 1);
	purgepool();
	wrlock(out);
	printf("Dcopy complete.\n");
}

getblk(unit, buff, block)
register unit, buff, block;
{
	rkio(unit, buff, map(form, block), READ | SSE, 256);
}

grablock(n)		/* grabs the next free block from the pool */
register n;
{
	if (inpool == nbuf) purgepool();
	inpool++;
	currbuff->bn = map(outform, n);
	return((currbuff++)->bb);
}

putblk(buff, block)
register int *buff;
{
	register int *w, q;

	w = grablock(block);
	q = 256;
	do *w++ = *buff++; while (--q);
}

purgepool()
{
	register int i;

	currbuff = startbuff;
	for (i = 0; i < inpool; i++)
	{
		rkio(out, currbuff->bb, currbuff->bn, WRITE | SSE, 256);
		currbuff++;
	}
	currbuff = startbuff;
	inpool = 0;
}

map(fmt, block)		/* translates optimised to equivalent unoptimised */
register fmt, block;
{
	if (! fmt || ! block) return(block);
	return(block > 2436 ? block - 2436 : block + 2435);
}

geti()
{
	if (inum++ < 16) node++;
	else
	{
		getblk(in, node = in_node, ++iblk);
		printf("%d\r", iblk);		/* keeps the user awake */
		inum = 1;
	}
}

puti()
{
	register *r, *w, q;

	w = &out_node[onum];
	r = node;
	q = 16;
	do *w++ = *r++; while (--q);
	if (++onum == 16)
	{
		putblk(out_node, oblk++);
		onum = 0;
	}
}

nxtblk()
{
	register block;

	do
	{
		block = (cyl * NTRACK + track) * BLKPERTRK + adr[sector];
		if (++sector >= BLKPERTRK)
		{
			sector = 0;
			if (++track >= NTRACK)
			{
				cyl++;
				track = 0;
			}
		}
	} while (block < 2 + superblk->s_isize);
	return(lastblock = block);
}

freelist()
{
	register rcyl, rsector, rtrack;
	extern int ldivr;
	unsigned block;

	rcyl = ldiv(0, superblk->s_fsize, NTRACK * BLKPERTRK);
	rtrack = ldivr / BLKPERTRK;
	rsector = ldivr % BLKPERTRK;	/* first unusable block */

	bfree(0);			/* end-of-list sentinel */

	while (rsector) bfree ((rcyl * NTRACK + rtrack) * BLKPERTRK + --rsector);
	for (;;)
	{
		if (rsector-- == 0)
		{
			rsector =+ BLKPERTRK;
			if (rtrack-- == 0)
			{
				rtrack =+ NTRACK;
				rcyl--;
			}
		}
		block = (rcyl * NTRACK + rtrack) * BLKPERTRK + adr[rsector];
		if (block == lastblock) return;
		if (block >= 2 + superblk->s_isize) bfree(block);
	}
}

bfree(b)
{
	register *r, *q, w;

	if (superblk->s_nfree == 100)
	{
		r = general;
		*r++ = superblk->s_nfree;
		q = superblk->s_free;
		w = 100;
		do *r++ = *q++; while (--w);
		putblk(general, b);
		superblk->s_nfree = 0;
	}
	superblk->s_free[superblk->s_nfree++] = b;
}

copyi()
{
	register *p, *pp, md;

	if (md = node->i_mode & IFMT)
	{
		if (md == IFDIR) copydir();
		puti();
		return;
	}

	if ((node->i_mode & ILARG) == 0)
	{
		for (p = node->i_addr; p < &node->i_addr[8]; p++)
			if (*p) getblk(in, grablock(*p = nxtblk()), *p);	/* stacking-dependent */
		puti();
		return;
	}

	for (p = node->i_addr; p < &node->i_addr[7]; p++)
		if (*p)
		{
			getblk(in, ind0, *p);
			*p = nxtblk();
			for (pp = ind0; pp < &ind0[256]; pp++)
				if (*pp) getblk(in, grablock(*pp = nxtblk()), *pp);
			putblk(ind0, *p);
		}
	if (node->i_addr[7])
	{
		getblk(in, ind0, node->i_addr[7]);
		node->i_addr[7] = nxtblk();
		for (p = ind0; p < &ind0[256]; p++)
			if (*p)
			{
				getblk(in, ind1, *p);
				*p = nxtblk();
				for (pp = ind1; pp < &ind1[256]; pp++)
					if (*pp) getblk(in, grablock(*pp = nxtblk()), *pp);
				putblk(ind1, *p);
			}
		putblk(ind0, node->i_addr[7]);
	}
	puti();
}

putdir(blk)
{
	register *to, *from, cntr;
	int count;

	to = upto;
	getblk(in, from = general, blk);
	for (count = 0; count < 32; count++)
	{
		if (! *from)
		{
			from =+ 8;
			continue;
		}
		if ((dirent & 037) == 0)
		{
			if (diraddr == 0 && dirent == 256)	/* convert to large file */
			{
				node->i_mode =| ILARG;
				*(diraddr = node->i_addr) = *ind1;
				for (cntr = 0; cntr < 7; cntr++)	/* at least 8 buffers have been guaranteed */
				{
					ind1[cntr] = ind1[cntr + 1];
					currbuff[cntr - 8].bn = currbuff[cntr - 7].bn;
				}
				currbuff[-1].bn = map(outform, ind1[7] = nxtblk());
			}
			else if (diraddr && (dirent & 017777) == 0)
			{
				putblk(ind1, *diraddr);
				*++diraddr = nxtblk();
				dirind = ind1;
			}
			to = grablock(*dirind++ = nxtblk());
		}
		for (cntr = 0; cntr < 8; cntr++) *to++ = *from++;
		dirent++;
	}
	upto = to;
}

copydir()
{
	register *p, *pp;

	dirent = diraddr = 0;
	dirind = ind1;
	if ((node->i_mode & ILARG) == 0)
	{
		for (p = node->i_addr; p < &node->i_addr[8]; p++)
			if (*p) putdir(*p);
	}
	else
	{
		if (nbuf - inpool < 8) purgepool();	/* guarantees 8 buffers - see `putdir' */
		node->i_mode =& ~ILARG;
		for (p = node->i_addr; p < &node->i_addr[7]; p++)
			if (*p)
			{
				getblk(in, ind0, *p);
				for (pp = ind0; pp < &ind0[256]; pp++)
					if (*pp) putdir(*pp);
			}
		if (diraddr) while (dirind < &ind1[256]) *dirind++ = 0;		/* clear rest of pointers */
	}
	node->i_size0 = (dirent >> 12) & 017;
	node->i_size1 = dirent << 4;
	if (diraddr == 0)
	{
		p = &node->i_addr[-1];
		for (pp = ind1; pp < dirind; *++p = *pp++);
	}
	for (pp = upto; pp < currbuff; *pp++ = 0);	/* clear rest of directory */
	if (diraddr) putblk(ind1, *(p = diraddr));
	while (p < &node->i_addr[7]) *++p = 0;
}

reldir()
{
	register i, j;

	if ((node->i_mode & ILARG) == 0)
	{
		for (i = 0; i < 8; i++) relx(node->i_addr[i]);
		return;
	}
	for (i = 0; i < 7; i++)
		if (node->i_addr[i])
		{
			getblk(out, ind0, node->i_addr[i]);
			for (j = 0; j < 256; j++) relx(ind0[j]);
		}
}

relx(b)
register b;
{
	register *p;
	register i;

	if (b == 0) return;
	getblk(out, p = grablock(b), b);
	i = 32;
	do
	{
		if (*p) *p = iindex[*p];
		p =+ 8;			/* 8 words per directory entry */
	} while (--i);
}

counti()
{
	register i, j, n;

	n = 0;
	for (i = 0; i < superblk->s_isize; i++)
	{
		getblk(in, in_node, 2+i);
		for (j = 0; j < 16; j++)
			if (in_node[j].i_mode & IALLOC) n++;
	}
	return((n + 15) / 16);
}

countfree()
{
	register n, m;

	n = superblk->s_nfree;
	if (n)
	{
		m = superblk->s_free[0];
		while (m)
		{
			getblk(in, general, m);
			n =+ general[0];
			m = general[1];
		}
	}
	return(n);
}

/********************************************************/


struct { unsigned u; };

initial()
{
	register j, *a, *d;
	extern char end;	/* identify the end of the bss  */

	bigbuf = (&end + 077) & 0177700;	/* start on 32w boundary */
	a = KISA0;
	d = KISD0;
	for (j = 0; j < 01600; j =+ 0200)
	{
		*a++ = j;
		*d++ = 077406;		/* 4kw r/w  */
	}
	*d = 077406;
	*a-- = 07600;		/* i/o page */
	SSR0->i++;		/* start mem-mngmnt */
	for (*a = (bigbuf >> 6); *a < 07600; (*a)++)
		for (d = 0140000; d.u < 0140100; *d++ = 0);	/* 32 w */
}

initiam()
{
	int register j;

	SSR0->i = 0;		/* stop mem-mngmnt */
	printf("\n\nMem =  %dkw\n", (j = KISA6->i) >> 5);
	iob = ((--j) - (bigbuf >> 6)) >> 3;
	if (iob > 256) iob = 256;	/* Max. disk i/o size */
	nio = BLKPERDSK / iob;
	szio = iob * 256;
	iov = BLKPERDSK - (nio * iob);
	szov = iov * 256;
	printf("%d big i/o's (each %d blocks)", nio, iob);
	if (iov) printf(" + one at %d blocks", iov);
	printf("\n\n");
}
