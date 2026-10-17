/*
 *	chk: fast combination of icheck+dcheck
 *
 *	Original from Vrije Universiteit, Amsterdam.
 *	Modified by Kevin Hill, University of NSW.
 */ 

#include <local-system>
#include <ino.h>
#include <filsys.h>

/*
 *	`prefix' is added to the given arguments - if this cannot be opened,
 *		the argument itself is tried.
 *	OFFSET is the char offset to the X's
 *	IN is the magic interleaving number
 *	NSEC is the number of sectors per track
 *		(must be <= 256 as `indbuf' is flogged in `makefree')
 */

#ifdef	EECF
char	prefix[]	"/dev/rmsXX";
#define	OFFSET	8
#define	IN	10
#define	NSEC	21
#endif

#ifdef	AGSM
char	prefix[]	"/dev/rhpXX";
#define	OFFSET	8
#define	IN	7
#define	NSEC	22
#endif

#ifdef	EECF1140
char	prefix[]	"/dev/rrkX";
#define	OFFSET	8
#define	IN	3
#define	NSEC	12
#endif

char *dargv[]
{
#ifdef EECF
	"02", "11", "00", "01", "03", "10",
#endif
#ifdef	AGSM
	"00", "01", "02", "03", "10", "11", "12", "13",
#endif
#ifdef	EECF1140
	"0", "1",
#endif
	0
};



/*
 *	TUNABLE CONSTANTS
 */

#define	IBLKS	16		/* this many blocks of inodes read each time */
#define	MAXSPECIAL	32	/* 2nd indir blks + directory indir. blks */

#define	BISZ	88		/* size of blist and ilist arrays */
#define	NPASS	20		/* max nr of passes in orphans() */
#define	LISTSZ	20		/* nr of levels looked at per pass in orphans() */
#define	BLKS	fsize / 24	/* sorts and reads this many directory blocks per read in orphans() */
#define	MAXDEPTH	20	/* max nr dir names printed out for inums - MUST BE < MAXSPECIAL */
#define	AVLEN	7		/* average length of a directory name, plus null */
#define	IINCR	10		/* allocate this many file structures each time in process */
#define	NINCR	256		/* allocate this many characters each time in addname - MUST BE EVEN */


/*
 *	exit status bits
 */

#define	COREERR	   1
#define	OPENERR	   2
#define	NONFATAL   4
#define	FATAL	0200

/*
 *	global variables
 */

struct inode inode[IBLKS * 16];	/* buffer to read inodes */ 
struct filsys sblock;		/* superblock of filesystem */ 
int  fildes;			/* file descriptor of filesystem */ 
char *ecount;			/* used in checking link counts */
int  *parent;			/* base of parent array in orphans() */
unsigned have;			/* largest chunk of core until now */
int  ninodes;			/* number of inodes on current device */ 
unsigned fsize;			/* size of current device */
unsigned isize;			/* size of i-list on device */
unsigned ino;			/* current inumber */ 
unsigned pino;			/* curr inum when processing in-core lists */
unsigned blknr;			/* current block */
char exitcode;			/* exitstatus is collected here */ 
int  nifree;			/* counts free inodes */
int  nspcl;			/* counts special files */
int  ndir;			/* nr. of directories */
int  nfile;			/* nr. of ordinary files */
int  nlarg;			/* number of large files */
int  nvlarg;			/* number of huge files */
int  nindir;			/* number of indirect blocks */
int  nvindir;			/* number of 2nd indirect blks */
int  high;			/* high water mark */
int  nused;			/* blocks used on this device */ 
char *usedbl;			/* bitmap for used blocks */ 
char *indbl;			/* bitmap for indirect blocks */ 
char *dirbl;			/* bitmap for directory blocks */ 
char *np;			/* current position in name array */
char *namend;			/* end of name array */
int  indbuf[256];		/* buffer to read indirect blocks */ 
unsigned  blist[BISZ];		/* list for funny blocks */
unsigned  ilist[BISZ];		/* list for funny inodes */

/*
 *	flags
 *	(the c-compiler will still use ints)
 */

char headpr;			/* flag for nice output */ 
char verbose;			/* verbose output */
char dotstoo;			/* print out . and .. entries for -i option */
char freelist;			/* recreate the free list */
char quiet;			/* cut out the horsing around */
char specfind;			/* look up special files */
char blook;			/* currently looking up blk nrs */
char ilook;			/* currently looking up inums */
char checked;			/* filsystem has been checked */
char chkdot;			/* currently checking . and .. entries */
char ctrlchrs;			/* print out control-chars as `^x' pair */

/*
 *	misc
 */

char data_small[] "data (small)";
char data_lge[] "data (large)";
char indirect[] "indirect";
char indr_huge[] "indirect (huge)";
char indr2[] "2nd indirect";

char dir_inum[] "dir inum ";
char not_in[] ".' not in first 32 bytes\n";


struct specialbl
{
	int blno;		/* block number */ 
	int which;		/* 2nd indir. or direc. indir. or inum */

#define SP2IND 0
#define SPDIR  1

} specialbl[MAXSPECIAL], *nspecial, *dirblks, *ndirbp;

struct dirent
{
	int inum, pinum;
	char *namep;
} *dstart, *ilstrt, *ilp, *ilend;


main(argc, argv)
register argc;
register char **argv;
{
	register n;
	char c;

	ecount = sbrk(0) - 1;		/* points to last-used byte of core */ 
	parent = ecount - 1;		/* points to last-used word of core */

	while (**++argv == '-')		/* check for flags */
	{
		argc--;
		switch (c = argv[0][1])
		{
	    case 'a':	dotstoo++;
			continue;

	    case 'b':
	    case 'i':	while (n = number(argv[1]))
			{
				addlist(n, c == 'b' ? blist : ilist);
				argv++;
				argc--;
			}
			if (headpr) headpr = 0;
			else { argv++; argc--; }
			continue;

	    case 'q':	quiet++;
			continue;

	    case 's':	freelist++;
			continue;

	    case 'v':	verbose++;
			continue;

	    case 'x':	specfind++;
			continue;

	    default:	printf("Usage: chk [-a][-b list][-i list][-q][-s][-v][-x] [specials]\n");
			return FATAL;
		}
	}

	if (argc == 1)			 /* check default devices */
	{
		argv = dargv;
		argc = sizeof dargv / 2;
	}

	while (--argc) check(*argv++);	/* check devices given as arguments */
	return exitcode;
}

check(file)
register char *file;
{
	register i, *p;
	register char *fp;
	unsigned want;
	int initpass();
	int checklinks();
	int findbad();
	int findblks();
	int markdirs();
	int countdir();
	int srchdirs();

	fp = file;
	i = OFFSET;
	while ((prefix[i++] = *fp++) && i < sizeof prefix);
	prefix[i - 1] = 0;
	if ((fildes = open(fp = prefix, freelist ? 2 : 0)) < 0 &&
		 (fildes = open(fp = file, freelist ? 2 : 0)) < 0)
	{
		printf("Cannot open %s\n", fp);
		exitcode =| OPENERR | FATAL;
		return;
	}
	nspecial = specialbl;
	nused = 0;
	nifree = 0;
	nspcl = 0;
	ndir = 0;
	nfile = 0;
	nlarg = 0;
	nvlarg = 0;
	nindir = 0;
	nvindir = 0;
	high = 0;
	checked = 0;
	headpr = 0;
	printf("%s:\n", fp);
	sync();				/* write out in core blocks */ 
	fsize = 2;			/* bread insists on blocks in range */ 
	bread(1, &sblock, 512);		/* read super block */ 
	fsize = sblock.s_fsize;
	ninodes = (isize = sblock.s_isize) * 16;
	if (*blist == 0 && *ilist == 0)
	{
		if (verbose) printf("Fsize\t%5u\nIsize\t%5u\n", fsize, isize);
		i = freelist ? 2 : 3;
		want = i * (fsize / 8) + ninodes + i;
		if (chkcore(want)) return;	/* not enough core */
		i = want >> 1;			/* number of words to clear */ 
		p = 1 + ecount;			/* address to start */ 
		do
			*p++ = 0;
		while (--i);
		ecount[want] = 0;
		usedbl = &ecount[ninodes + 1];
		indbl = &usedbl[fsize / 8 + 1];
		dirbl = &indbl[fsize / 8 + 1];
		scaninodes(initpass);
		ino = 0;		/* inum not known on error now */
		dospecs();
		finioff();
		if (freelist)
		{
			makefree();
			return;
		}
		checkfree();
		scaninodes(checklinks);
		if ((exitcode & FATAL) == 0) orphans();
		checked++;
	}
	if (*blist)
	{
		for (p = blist; *p; p++)
			if (*p > 1 && *p < isize + 2) printf("blk %u in i-list\n", *p);
		nspecial = specialbl;
		ndirbp = dirblks = &parent[1];			/* first usable word */
		if (chkcore(BLKS * 4)) return;
		blook++;
		scaninodes(checked ? findbad : findblks);
		chkindir();
		readblks();
		for (p = blist; *p; *p++ = 0);		/* clear out blist */
		blook = 0;
	}
	if (*ilist)
	{
		if (checked)
		{
			printf("\n looking for inums");
			for (p = ilist; *p; printf(" %u", *p++));
			putchar('\n');
		}
		else scaninodes(countdir);
		for (p = ilist, nspcl = 0; *p++; nspcl++);
		nfile = ndir;
		nlarg = (nspcl + nfile) * AVLEN;
		ndir =+ (ndir + 3) / 4;			/* to improve the hashing */
		want = (ndir + nspcl) * sizeof *dstart + BLKS * sizeof *nspecial + nlarg;
		if (chkcore(want)) return;
		ilook++;
		p = dstart = ecount + 1;
		i = (ndir * sizeof *dstart) >> 1;
		do					/* clean it up */
			*p++ = 0;
		while (--i);
		nspecial = specialbl;
		ndirbp = dirblks = &ecount[ndir * sizeof *dstart + 1];
		np = &ndirbp[BLKS];
		ilstrt = ilp = namend = &np[(nlarg + 1) & ~01];
		ilend = &ilstrt[nspcl];
		scaninodes(markdirs);
		scaninodes(srchdirs);
		getdirblks();
		readblks();
		printout();
		for (p = ilist; *p; *p++ = 0);		/* clear i-list */
		ilook = 0;
	}
	close(fildes);
}


/*
 *	General-purpose inode scanning procedure
 */

scaninodes(f)
register (*f)();
{
	register i, j;

	for (i = 0, ino = 0; ino < ninodes; i =+ IBLKS)
	{
		bread(i+2, inode, sizeof inode);	/* read large chunk */ 
		for (j = 0; j < IBLKS * 16 && ino < ninodes; j++)
		{
			ino++;
			(*f)(&inode[j]);
		}
	}
}


/*
 *	Read inodes; for each active inode update all bitmaps.
 *	Also fill usedbl[] with all double indirect blocks and
 *	all indirect directory blocks.
 *	There won't be a lot of those but if there are some 
 *	they have to be read first
 */ 

initpass(ip)
register struct inode *ip;
{
	register mode;
	register i;
	int n, dir;

	if (((mode = ip->i_mode) & IALLOC) == 0)	/* unused inode */ 
	{
		nifree++;
		return;
	}
	if ((mode & (IFCHR & IFBLK)))			/* special file */ 
	{
		nspcl++;
		if (specfind)
		{
			printf("Spec file at %u\n", ino);
			addlist(ino, ilist);
		}
		return;
	}
	dir = 0;
	if ((mode & IFMT) == IFDIR)
	{
		dir++;
		ndir++;
		if (ip->i_size1 & 017 || ip->i_size0 || ip->i_size1 < 32)
		{
			printf("Illegal dir size; inum %u\n", ino);
			addlist(ino, ilist);
			exitcode =| FATAL;
		}
	}
	else nfile++;
	if ((mode & ILARG) == 0)
	{
		for (i = 0; i < 8; i++)
		{
			if ((n = ip->i_addr[i]) == 0) continue;
			setbit(n, usedbl, data_small);
			if (dir && freelist == 0) setbit(n, dirbl);
		}
	}
	else
	{
		nlarg++;
		for (i = 0; i < 7; i++)
		{
			if ((n = ip->i_addr[i]) == 0) continue;
			nindir++;
			setbit(n, usedbl, indirect);
			setbit(n, indbl);
			if (dir && freelist == 0) inspecial(n, SPDIR);
		}
		if ((n = ip->i_addr[7]))
		{
			nvlarg++;
			setbit(n, usedbl, indr_huge);
			if (dir)
			{
				printf("Huge directory; inum %u\n", ino);
				addlist(ino, ilist);
				exitcode =| FATAL;
				return;
			}
			inspecial(n, SP2IND);
			if (verbose)
			{
				printf("Huge file; inum %u\n", ino);
				addlist(ino, ilist);
			}
		}
		else if (dir && verbose)
		{
			printf("Large directory; inum %u\n", ino);
			addlist(ino, ilist);
		}
	}
}


/*
 *	Read all double indirect blocks and all indirect directory blocks
 *	and fill in bitmaps.
 */ 

compar(p1, p2)
struct specialbl *p1, *p2;		/* NOT registers - shorter code */
{
	return p1->blno - p2->blno;
}

dospecs()
{
	register struct specialbl *p;
	register i, n;

	if (nspecial > specialbl + 1)
		qsort(specialbl, nspecial - specialbl, sizeof specialbl[0], compar);
	for (p = specialbl; p < nspecial; p++)
	{
		bread(blknr = p->blno, indbuf, 512);
		for (i = 0; i < 256; i++)
		{
			if ((n = indbuf[i]) == 0) continue;
			if (p->which == SP2IND)
			{
				nvindir++;
				setbit(n, usedbl, indr2);
				setbit(n, indbl);
			}
			else setbit(n, dirbl);
		}
	}
}


/*
 *	Proceed through the bitmaps indbl and dirbl, letting the
 *	arm of the disk move from low to high once.
 */ 

finioff()
{
	register i, k, b;
	int ind, dir, bit, j;
	char *n;

	for (i = 0; i < (fsize / 8 + 1); i++)
	{
		ind = indbl[i];
		dir = freelist ? 0 : dirbl[i];
		if (ind || dir)
		{
			b = i * 8;
			bit = 1;
			for (j = 0; j < 8; j++)
			{
				blknr = b;
				if (ind & bit)		/* indirect block */
				{
					bread(b, indbuf, 512);
					for (k = 0; k < 256; k++)
						if (indbuf[k])
							setbit(indbuf[k], usedbl, data_lge);
				}
				else if (dir & bit)	/* directory block */
				{
					bread(b, indbuf, 512);
					for (k = 0; k < 256; k =+ 8)
					{
						if ((n = indbuf[k]) == 0) continue;
						if (n <= ninodes) ++ecount[n];
						else
						{
							if (addlist(b, blist)) printf("Bad dir inum (%u) (blk %u)\n", n, b);
							exitcode =| FATAL;
						}
					}
				}
				b++;
				bit =<< 1;
			}
		}
	}
}


/*
 *	Read free list and final icheck
 */ 

checkfree()
{
	register b, n;

	n = 0;
	if (verbose) printf("Used\t%5u\n", nused);
	while ((b = nextfree()))
	{
		setbit(b, usedbl, "free");
		n++;
	}
	if (b = fsize - isize - 2 - nused)
	{
		printf("Missing %u blocks\n", b);
		exitcode =| NONFATAL;
	}
	if (verbose)
	{
		printf("Spcl\t%5u\n", nspcl);
		printf("Files\t%5u\n", nfile);
		printf("Large\t%5u\n", nlarg);
		if (nvlarg) printf("Huge\t%5u\n", nvlarg);
		printf("Direc\t%5u\n", ndir);
		printf("Indir\t%5u\n", nindir);
		if (nvindir) printf("Indir2\t%5u\n", nvindir);
		printf("High\t%5u\n", high);
	}
	if (quiet == 0) printf("Freei\t%5u\nFree\t%5u\n", nifree, n);
}


/*
 *	Read inodes, for each active inode check link and count
 */ 

checklinks(ip)
register struct inode *ip;
{
	register i;

	i = ino;
	if ((ip->i_mode & IALLOC) == 0 && ecount[i] == 0) return;
	if (ip->i_nlink == ecount[i] && ip->i_nlink) return;
	if (headpr++ == 0) printf("\nInum\tDir-ent\tLnk-cnt\n");
	printf("%u\t%u\t%u\n", ino, ecount[i] & 0377, ip->i_nlink & 0377);
	if (ecount[i] == 0) exitcode =| NONFATAL;
	else
	{
		exitcode =| FATAL;
		addlist(i, ilist);
	}
}


/*
 *	Look for disconnected branches
 */

orphans()
{
	register j;
	register unsigned k, *lp;
	unsigned *lpp;
	int	i, notyet;
	int list[LISTSZ];
	int readir();
	int fstblk();

	k = (j = ninodes + BLKS * 2) * 2;
	if (chkcore(k)) return;
	lp = 1 + ecount;		/* clear it out */
	do
		*lp++ = 0;
	while (--j);
	ndirbp = dirblks = &parent[ninodes + 1];
	nspecial = specialbl;

	/*
	 * Now rescan inodes looking for all directories
	 * These must be read, and their child inodes marked (in parent[])
	 * as children of the directory (inum).
	 * Also, check all . and .. entries.
	 */

	parent[1] = 1;
	scaninodes(readir);
	getdirblks();
	readblks();
	ndirbp = dirblks;
	nspecial = specialbl;
	chkdot++;
	scaninodes(fstblk);
	getdirblks();
	readblks();
	chkdot = 0;

	/*
	 * Now scan through parent[] - for each slot, work back through
	 * parents until reach inode number 1 (you hope!).
	 */

	i = 0;
	do
	{
		notyet = 1;
		for (j = 2; j <= ninodes; j++)
		{
			if (parent[k = j] <= 1) continue;
			for (lp = list; lp < &list[LISTSZ]; )
				if ((*lp++ = k = parent[k]) <= 1) break;
			if (k > 1) notyet = 0;
			if (k == 0) { lp--; k = lp[-1]; }
			lpp = lp - 2;
			for (lp = list; lp < lpp; lp++) parent[*lp] = k;
			parent[j] = k;
		}
	} while (i++ < NPASS && notyet == 0);

	for (j = 2; j < ninodes; j++)
		if ((i = parent[j]) != 1)
		{
			printf("possible orphan; inum %u", j);
			if (i)
			{
				printf(" (from %u)", i);
				addlist(i, ilist);
			}
			putchar('\n');
			exitcode =| NONFATAL;
		}
}

readir(ip)
struct inode *ip;
{
	register i, n, mode;
	int readblks();

	if (((mode = ip->i_mode) & IALLOC) == 0 || (mode & IFMT) != IFDIR)
	{
		parent[ino] = 1;
		return;
	}
	if ((mode & ILARG) == 0)
	{
		for (i = 0; i < 8; i++)
			if (n = ip->i_addr[i]) store(n, ino);
	}
	else for (i = 0; i < 7; i++)
		if (n = ip->i_addr[i]) storespec(n);
}

store(bno, inum)
unsigned bno;
{
	if (bno < isize + 2 || bno >= fsize) return;
	if (ndirbp == &dirblks[BLKS])
	{
		readblks();
		ndirbp = dirblks;
	}
	ndirbp->blno = bno;
	ndirbp->which = inum;
	ndirbp++;
}

storespec(bno)
{
	if (nspecial == &specialbl[MAXSPECIAL])
	{
		getdirblks();
		nspecial = specialbl;
	}
	nspecial->blno = bno;
	nspecial->which = ino;
	nspecial++;
}

getdirblks()
{
	register struct specialbl *p;
	register i, n;

	if (nspecial > specialbl + 1)
		qsort(specialbl, nspecial - specialbl, sizeof specialbl[0], compar);
	for (p = specialbl; p < nspecial; p++)
	{
		bread(p->blno, indbuf, 512);
		if (chkdot) store(indbuf[0], p->which);
		else for (i = 0; i < 256; i++)
			if (n = indbuf[i]) store(n, p->which);
	}
}

readblks()
{
	register struct specialbl *p;
	register k, n;

	if (ndirbp > dirblks + 1)
		qsort(dirblks, ndirbp - dirblks, sizeof dirblks[0], compar);
	for (p = dirblks; p < ndirbp; p++)
	{
		bread(p->blno, indbuf, chkdot ? 32 : 512);
		pino = p->which;			/* do NOT use ino */
		if (chkdot) { checkdots(); continue; }
		for (k = 0; k < 256; k =+ (blook ? 1 : 8))
		{
			if ((n = indbuf[k]) == 0) continue;
			if (blook)
			{
				chklist(n, pino, data_lge);
				continue;
			}
			if (ilook)
			{
				if (n >= 1 && n <= ninodes) process(&indbuf[k]);
				continue;
			}
			if ((parent[n] == 0) && ! dotname(&indbuf[k + 1])) parent[n] = pino;
		}
	}
}

dotname(s)
register *s;
{
	return (*s == '.\0' || *s == '..' && s[1] == 0);
}

bread(bno, buf, cnt)
register unsigned bno;
{
	register char *p;
	register i;

	if (bno >= fsize)
	{
		if (checked) abort();	/* impossible! */
		exitcode =| FATAL;	/* Error message will be given in setbit() */
		p = buf;
		i = cnt;
		do
			*p++ = 0;
		while (--i);
		return;
	}
	seek(fildes, bno, 3);
	if (read(fildes, buf, cnt) != cnt)
	{
		printf("read error %u\n", bno);
		exitcode =| FATAL;
		exit(exitcode);
	}
}


/*
 *	Set the bit in the given bit-map, checking for bad and dup blocks.
 *	`string' is only valid when map == usedbl.  `string' only starts
 *	with `f' when checking blocks in the free list.
 */

setbit(bno, map, string)
char *map, *string;
register unsigned bno;
{
	register char *b;
	register bit;

	if (bno < isize + 2 || bno >= fsize)
	{
		if (map == usedbl) prerr("%u bad;", bno, string);
		exitcode =| FATAL;
		return;
	}
	b = &map[bno >> 3];		/* unsigned; don't have to mask */
	bit = 1 << (bno & 07);
	if (map == usedbl)
	{
		if ((*b) & bit)
		{
			prerr("%u dup;", bno, string);
			exitcode =| FATAL;
			return;
		}
		nused++;
		if (string[0] != 'f' && bno > high) high = bno;
	}
	*b =| bit;
}


prerr(errstrng, bno, string)
register char *string;
register unsigned bno;
char *errstrng;
{
	register printed 0;

	if (ino)
	{
		if (addlist(ino, ilist))
		{
			printf(errstrng, bno);
			printf(" inum %u;", ino);
			printed++;
		}
	}
	else
	{
		if (addlist(blknr ? blknr : bno, blist))
		{
			printf(errstrng, bno);
			if (blknr) printf(" in ind-blk %u;", bno = blknr);
			printed++;
		}
	}
	if (printed) printf(" class=%s\n", string);
}


inspecial(bno, wflag)
unsigned bno;
{
	if (nspecial == &specialbl[MAXSPECIAL])
	{
		/*
		 * There are more than MAXSPECIAL strange blocks.
		 * Now we are forced to make an extra pass.
		 */ 

		printf("More than %u special blocks (recompile?)\n", MAXSPECIAL);
		dospecs();
		nspecial = specialbl;
	}
	nspecial->blno = bno;
	nspecial->which = wflag;
	nspecial++;
}

nextfree()
{
	register unsigned b, i;

	i = --sblock.s_nfree;
	if (i >= 100)		/* gets < 0 too! */
	{
		printf("illegal nfree (%u)\n", i + 1);
		exitcode =| FATAL;
		return(0);
	}
	b = sblock.s_free[i];
	if (b == 0) return(0);
	if (sblock.s_nfree <= 0)
	{
		bread(b, indbuf, 512);
		sblock.s_nfree = indbuf[0];
		for (i = 0; i < 100; i++) sblock.s_free[i] = indbuf[i+1];
	}
	return(b);
}

makefree()
{
	register unsigned i, j, low;
	int *flag, adr[NSEC];

	flag = &indbuf[0];
	for (i = 0; i < NSEC; flag[i++] = 0);
	for (i = 0, j = 0; i < NSEC; i++)
	{
		while (flag[j]) j = (j + 1) % NSEC;
		adr[i] = j;
		flag[j]++;
		j = (j + IN) % NSEC;
	}
	sblock.s_nfree = sblock.s_ninode = 0;
	sblock.s_flock = sblock.s_ilock = sblock.s_fmod = 0;
	for (i = 101; i < 256; indbuf[i++] = 0);
	low = isize + 2;		/* lowest potentially usable blk */
	free(0);				/* end-of-list sentinel */
	j = lrem(0, i = sblock.s_fsize, NSEC);
	while (j-- && --i >= low) freebl(i);
	i--;
	while (i >= low + NSEC - 1)
	{
		for (j = 0; j < NSEC; j++) freebl(i - adr[j]);
		i =- NSEC;
	}
	while (i >= low) freebl(i--);
	bwrite(1, &sblock);			/* do NOT sync (may be root device) */
	close(fildes);
}

free(bno)
{
	register i;

	if (sblock.s_nfree >= 100)
	{
		indbuf[0] = sblock.s_nfree;
		for (i = 0; i < 100; i++) indbuf[i + 1] = sblock.s_free[i];
		sblock.s_nfree = 0;
		bwrite(bno, indbuf);
	}
	sblock.s_free[sblock.s_nfree++] = bno;
}

freebl(bno)
{
	if ((usedbl[(bno >> 3) & 017777] & (1 << (bno & 07))) == 0) free(bno);
}

bwrite(bno, buf)
register unsigned bno;
{
	seek(fildes, bno, 3);
	if (write(fildes, buf, 512) != 512)
	{
		printf("write error %u\n", bno);
		exitcode =| FATAL;
		exit(exitcode);
	}
}


/*
 *	findbad looks up blocks in `blist' to find their inode nrs
 *	(if they have been put there as a result of the checks).
 *	Note that it is not necessary to check indirect blocks of
 *	ordinary files, as their inums will already be known.
 */

findbad(ip)
struct inode *ip;
{
	register i, n, mode;
	int chkindir();

	if (((mode = ip->i_mode) & IALLOC) == 0 ||
		(mode & (IFCHR & IFBLK))) return;
	for (i = 0; i < 8; i++)
		if (n = ip->i_addr[i]) chklist(n, ino, 0);
	if ((mode & ILARG) == 0) return;
	if ((mode & IFMT) == IFDIR)
	{
		for (i = 0; i < 7; i++)
			if (n = ip->i_addr[i]) build(n);
		return;
	}
	if (n = ip->i_addr[7]) build(n);
}

build(bno)
register unsigned bno;
{
	if (bno < isize + 2 || bno >= fsize) return;
	if (nspecial == &specialbl[MAXSPECIAL])
	{
		chkindir();
		nspecial = specialbl;
	}
	nspecial->blno = bno;
	nspecial->which = ino;
	nspecial++;
}

chkindir()
{
	register struct specialbl *p;
	register i, n;

	if (nspecial > specialbl + 1)
		qsort(specialbl, nspecial - specialbl, sizeof specialbl[0], compar);
	for (p = specialbl; p < nspecial; p++)
	{
		bread(p->blno, indbuf, 512);
		for (i = 0; i < 256; i++)
			if (n = indbuf[i])
			{
				chklist(n, p->which, blook ? indr2 : 0);
				if (blook) store(n, p->which);
			}
	}
}

chklist(bno, inum, s)
register bno;
char *s;
{
	register unsigned *lp blist;

	while (*lp != bno && *++lp);
	if (*lp)
	{
		printf("blk %u from inum %u", bno, inum);
		if (s) printf("; class=%s", s);
		putchar('\n');
		addlist(inum, ilist);
	}
}

findblks(ip)
struct inode *ip;
{
	register i, n, mode;

	if (((mode = ip->i_mode) & IALLOC) == 0 ||
		(mode & (IFCHR & IFBLK))) return;
	if ((mode & ILARG) == 0)
	{
		for (i = 0; i < 8; i++)
			if (n = ip->i_addr[i]) chklist(n, ino, data_small);
	}
	else
	{
		for (i = 0; i < 7; i++)
		{
			if ((n = ip->i_addr[i]) == 0) continue;
			chklist(n, ino, indirect);
			store(n, ino);
		}
		if (n = ip->i_addr[7])
		{
			chklist(n, ino, indr_huge);
			build(n);
		}
	}
}

fstblk(ip)
register struct inode *ip;
{
	register mode;

	if (((mode = ip->i_mode) & IALLOC) == 0 || (mode & IFMT) != IFDIR) return;
	if ((mode & ILARG) == 0) store(ip->i_addr[0], ino);
	else storespec(ip->i_addr[0]);
}

checkdots()
{
	register *dot, *dotdot, *s;

	dot = dotdot = 0;
	s = indbuf;
	do
	{
		if (s[0])
			if (s[1] == '.\0') dot = s;
			else if (s[1] == '..' && s[2] == 0) dotdot = s;
		s =+ 8;
	} while (s != &indbuf[16]);
	s = 0;
	if (dot == 0)
	{
		printf("%s%u: `%s", dir_inum, pino, not_in);
		s++;
	}
	else if (*dot != pino)
	{
		printf("%s%u: `.' inum %u\n", dir_inum, pino, *dot);
		addlist(*dot, ilist);
		s++;
	}
	if (dotdot == 0)
	{
		printf("%s%u: `.%s", dir_inum, pino, not_in);
		s++;
	}
	else if (*dotdot != parent[pino])
	{
		printf("%s%u: from %u; `..' inum %u\n", dir_inum, pino, parent[pino], *dotdot);
		addlist(*dotdot, ilist);
		s++;
	}
	if (s)
	{
		addlist(pino, ilist);
		exitcode =| FATAL;
	}
}

countdir(ip)
struct inode *ip;
{
	register mode;

	if (((mode = ip->i_mode) & IALLOC) && (mode & IFMT) == IFDIR) ndir++;
}

markdirs(ip)
struct inode *ip;
{
	register mode;

	if (((mode = ip->i_mode) & IALLOC) && (mode & IFMT) == IFDIR) lookup(ino, 1);
}

lookup(inum, add)
register inum;
{
	register struct dirent *d, *dend;

	d = &dstart[inum % ndir];
	dend = &dstart[ndir];
	while (d->inum)
	{
		if (d->inum == inum) return d;
		if (++d >= dend) d = dstart;
	}
	if (add) d->inum = inum;
	return 0;
}

srchdirs(ip)
struct inode *ip;
{
	register i, n, mode;

	if (((mode = ip->i_mode) & IALLOC) == 0 || (mode & IFMT) != IFDIR) return;
	if ((mode & ILARG) == 0)
	{
		for (i = 0; i < 8; i++)
			if (n = ip->i_addr[i]) store(n, ino);
	}
	else
	{
		for (i = 0; i < 7; i++)
			if (n = ip->i_addr[i]) storespec(n);
	}
}

process(dp)
register *dp;
{
	register i, *ip;

	if ((i = dotname(dp + 1)) && ! dotstoo) return;
	if (i == 0 && (ip = lookup(*dp, 0)))		/* it's a directory */
	{
		ip->pinum = pino;
		ip->namep = addname(dp + 1);
	}
	ip = ilist;
	while (*ip != *dp && *++ip);
	if (*ip)
	{
		if (ilp == ilend)
		{
			if ((i = more(&ilend, &namend, IINCR * sizeof *ilend)) == -1) return;
			if (i)
			{
				ilp--;
				ilend = i;
				ilend->inum = ilp->inum;
				ilend->pinum = ilp->pinum;
				ilend->namep = ilp->namep;
				ilp->inum = 0;
				ilp = ilp->pinum = i;
				ilp++;
			}
			ilend =+ IINCR;
		}
		ilp->inum = *dp;
		ilp->pinum = pino;
		ilp->namep = addname(dp + 1);
		ilp++;
	}
}

addname(nm)
char *nm;
{
	register i, j;
	register char *cp;
	char *new;

	i = 0;
	cp = np;
	do
	{
		if (cp == namend)
		{
			if ((j = more(&namend, &ilend, NINCR)) == -1) return;
			if (new = j)
			{
				nm =- i;
				namend = np = cp = new;
				i = 0;
			}
			namend =+ NINCR;
		}
		*cp = *nm++;
	} while (*cp++ && i++ < 14);
	cp[-1] = 0;			/* in case name is 14 chars long */
	nm = np;
	np = cp;
	return nm;
}

more(me, him, incr)
register unsigned *me, *him, incr;
{
	if (*him < *me)		/* contiguous allocation possible */
	{
		if (chkcore(*me - ecount - 1 + incr)) return -1;
		return 0;
	}
	if (chkcore(*him - ecount - 1 + incr)) return -1;
	return *him;
}

printout()
{
	register n;
	register struct dirent *d, *dp;

	for (d = ilstrt; d != ilp; d++)
	{
		if ((dp = d)->inum == 0) d = dp = dp->pinum;	/* chain along */
		nspecial = specialbl;
		n = 0;
		do
			storespec(dp);
		while dp->pinum != 1 && ++n != MAXDEPTH && (dp = lookup(ino = dp->pinum, 0)) != 0;
		pname(dp);
	}
}

pname(dp)
register struct dirent *dp;
{
	register struct specialbl *p;

	p = nspecial - 1;
	if (dp == 0) printf("?%u?", ino);
	else if (dp->pinum != 1) printf(".../%u", dp->pinum);
	else putchar('1');
	while (p >= specialbl) printf("/%u", (p--)->blno->inum);
	printf(":\t");
	p = nspecial - 1;
	ctrlchrs++;
	if (dp == 0) printf("?%u?", ino);
	else if (dp->pinum != 1) printf("...");
	else printf("%s", (p--)->blno->namep);
	while (p >= specialbl) printf("/%s", (p--)->blno->namep);
	ctrlchrs = 0;
	putchar('\n');
}

/*
 *	addlist returns 1 if the given element is not already
 *	in the list, or if it is in the list and the verbose option is on
 */

addlist(n, list)
register unsigned n;
unsigned list[];
{
	register unsigned *lp list;
	static ioflw, boflw;

	if (*lp)
	{
		while (*lp != n && *++lp);
		if (*lp) return verbose;
		if (lp >= &list[BISZ - 1])
		{
			if (list == blist && boflw++ == 0) printf("b-list overflow\n");
			if (list == ilist && ioflw++ == 0) printf("i-list overflow\n");
			return 1;
		}
	}
	*lp = n;
	return 1;
}

number(s)
register char *s;
{
	register n 0;

	if (*s < '0' || *s > '9')
	{
		headpr++;
		return 0;
	}
	while (*s) n = n * 10 + *s++ - '0';
	return n;
}

chkcore(want)
register unsigned want;
{
	if (want > have)
	{
		if (sbrk(want - have) == -1)
		{
			printf("Not enough core\n");
			exitcode =| COREERR | FATAL;
			return 1;
		}
		have = want;
	}
	return 0;
}

/*
 *	This printf is smaller than the C-library version.
 *	It supports only %s, %u, and %nu (n a single digit).
 */

printf(s, a)
register char *s;
unsigned a;
{
	register c;
	register unsigned *p;
	int fw;

	p = &a;
	while (c = *s++)
	{
		if (c == '%')
		{
			if ((c = *s++) >= '0' && c <= '9')
			{
				fw = c - '0';
				c = *s++;
			}
			else fw = 0;
			switch (c)
			{
		    case 's':	printf(*p++);		/* CARE! */
				continue;

		    case 'u':	printl(*p++, fw);
				continue;

		    case 0:	return;
			}
		}
		putchar(c);
	}
}

printl(n, fw)
register unsigned n;
register fw;
{
	register char *s;
	char stack[10];

	s = &stack[sizeof stack];
	*--s = 0;
	do
	{
		*--s = n % 10 + '0';
		n =/ 10;
		fw--;
	} while (n);
	while (fw-- > 0) *--s = ' ';
	while (*s) putchar(*s++);
}

putchar(c)
{
	if ((c =& 0177) < ' ' && ctrlchrs)
	{
		putchar('^');
		c =| 0100;
	}
	write(1, &c, 1);
}
