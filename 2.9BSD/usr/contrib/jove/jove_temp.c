/*
   Jonathan Payne at Lincoln-Sudbury Regional High School 5-25-83
  
   Much of this code was lifted from VI (ex_temp.c).  These functions
   deal with (put/get)ing lines in/from the tmp file.  */

#include "jove.h"
#include "jove_temp.h"

int	DOLsave = 0;	/* Do Lsave flag.  If lines aren't being save
			   when you think they should have been, this
			   flag is probably not being set, or is being
			   cleared before lsave() was called. */

tmpinit()
{
	tfname = mktemp(TMPFILE);
	tline = 2;
	iblock1 = oblock = iblock2 = -1;
	hitin2 = ichng1 = ichng2 = 0;
	ignore(close(creat(tfname, 0600)));
	tmpfd = open(tfname, 2);
	if (tmpfd == -1) {
		putstr(sprint("%s?\n", tfname));
		finish(0);
	}
}

/* Get a line at `tl' in the tmp file into `buf' which should be LBSIZE
   long. */

char *
getline(tl, buf)
disk_line	tl;
char	*buf;
{
	register char	*bp,
			*lp;
	register int	nl;

	lp = buf;
	bp = getblock(tl, READ);
	nl = nleft;
	tl &= ~OFFMSK;
	while (*lp++ = *bp++) {
		if (--nl == 0) {
			/* += INCRMT moves tl to the next block in
			   the tmp file. */
			bp = getblock(tl += INCRMT, READ);
			nl = nleft;
		}
	}

	return buf;
}

/* Put `buf' and return the disk address */

disk_line
putline(buf)
char	*buf;
{
	register char	*bp,
			*lp;
	register disk_line	nl;
	disk_line	tl;

	lp = buf;
	tl = tline;
	bp = getblock(tl, WRITE);
	nl = nleft;
	tl &= ~OFFMSK;
	while (*bp = *lp++) {
		if (*bp++ == '\n') {
			*--bp = 0;
			break;
		}
		if (--nl == 0) {
			bp = getblock(tl += INCRMT, WRITE);
			nl = nleft;
		}
	}
	nl = tline;
	tline += (((lp - buf) + BNDRY - 1) >> SHFT) & 077776;
	return nl | DIRTY;	/* So it will be redisplayed */
}

/* Get a block which contains at least part of the line with the address
 * atl.  Returns a pointer to the block and sets the global variable
 * nleft (number of good characters left in the buffer)
 */

char *
getblock(atl, iof)
disk_line	atl;
{
	extern int	read(),
			write();
	register int	bno,
			off;

	if (atl == 0)
		complain("Free line in list!!!");
	bno = (atl >> OFFBTS) & BLKMSK;
	off = (atl << SHFT) & LBTMSK;
	if (bno >= NMBLKS)
		error("Tmp file too large.  Get help");
	nleft = BUFSIZ - off;
	if (bno == iblock1) {
		ichng1 |= iof;
		return ibuff1 + off;
	}
	if (bno == iblock2) {
		ichng2 |= iof;
		return ibuff2 + off;
	}
	if (bno == oblock)
		return obuff + off;
	if (iof == READ) {
		if (hitin2 == 0) {
			if (ichng2)
				blkio(iblock2, ibuff2, write);
			ichng2 = 0;
			iblock2 = bno;
			blkio(bno, ibuff2, read);
			hitin2 = 1;
			return (ibuff2 + off);
		}
		hitin2 = 0;
		if (ichng1)
			blkio(iblock1, ibuff1, write);
		ichng1 = 0;
		iblock1 = bno;
		blkio(bno, ibuff1, read);
		return (ibuff1 + off);
	}
	if (oblock >= 0)
		blkio(oblock, obuff, write);
	oblock = bno;
	return obuff + off;
}

#ifdef	VMUNIX
#define	INCORB	64
char	incorb[INCORB+1][BUFSIZ];
#define	pagrnd(a)	((char *)(((int)a)&~(BUFSIZ-1)))
#endif

blkio(b, buf, iofcn)
short	b;
char	*buf;
int	(*iofcn)();
{

#ifdef VMUNIX
	if (b < INCORB) {
		if (iofcn == read) {
			bcopy(pagrnd(incorb[b+1]), buf, BUFSIZ);
			return;
		}
		bcopy(buf, pagrnd(incorb[b+1]), BUFSIZ);
		return;
	}
#endif
	ignore(lseek(tmpfd, (long) (unsigned) b * BUFSIZ, 0));
	if ((*iofcn)(tmpfd, buf, BUFSIZ) != BUFSIZ)
		error("IO error");
}

#ifdef VMUNIX

/* block copy from from to to, count bytes */

bcopy(from, to, count)
#ifdef vax
	char *from, *to;
	int count;
{

	asm("	movc3	12(ap),*4(ap),*8(ap)");
}
#else
	register char *from, *to;
	register int count;
{
	while ((--count) >= 0)
		*to++ = *from++;
}
#endif
#endif VMUNIX

/*
 * Save the current contents of linebuf, if it has changed.
 */

lsave()
{
	char	tmp[LBSIZE];

	if (curbuf == 0 || !DOLsave)	/* Nothing modified recently */
		return;

	if (!strcmp(linebuf, getline(curline->l_dline, tmp)))
		return;		/* They are the same. */
	SavLine(curline, linebuf);	/* Put linebuf on the disk */
	DOLsave = 0;
}

getDOT()
{
	ignore(getline(curline->l_dline, linebuf));
}

SavLine(addr, buf)
LINE	*addr;
char	*buf;
{
	addr->l_dline = putline(buf);
}
