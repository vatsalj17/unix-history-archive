# include	<stdio.h>
# include	<ingres.h>
# include	<aux.h>
# include	<symbol.h>
# include	<access.h>
# include 	<func.h>
# include	<batch.h>
# include	<catalog.h>
# include	<pv.h>
# include	<sccs.h>

SCCSID(@(#)ksort.c	8.4	12/8/85)

# define	N	7
# define	MEM	(32768 - 2)
# define	BUCKETSIZE	4
# define	ENDKEY	MAXDOM + 1



/*
**	Parameters:
**
**		pv[0]:		Fileset
**		pv[1]:		Infile from which reln is read
**		pv[2]:		Outfile to which reln is written
**		pv[3...]:	the desc of the new relation
**
**	Trace Flag:	Z37
*/

extern short	tTdbu[100];
extern int	ksort();
extern int	null_fn();

struct fn_def KsortFn =
{
	"KSORT",
	ksort,
	null_fn,
	null_fn,
	NULL,
	0,
	tTdbu,
	100,
	'Z',
	0
};

static char		*Infile;
static char		*Outfile;
static DESC		Desc;
static char		Descsort[MAXDOM+1];
static FILE		*Oiop;
static int		Tupsize;
static int		Bucket;
static char		File[15];
static char		*Fileset;
static char		*Filep;
static int		Nlines;
static long		Ccount;
static char		**Lspace;
static char		*Tspace;
extern int		cmpa();
static long		Tupsout;
static int		firstime	= 1;
static FILE		*Btree_fp;
DESC			Btreesec;
int			Btree_fd;
int			Nfiles;

ksort(pc, pv)
int	pc;
PARM	*pv;
{
	extern char	*Proc_name;
	register int	i;
	register int	j;
	unsigned int	mem;
	char		*start;
	int		maxkey, rev;
	extern char	*malloc();

# ifdef xZTR1
	if (tTf(37,0))
	{
		lprintf("entering ksort\n");
		prvect(pc,pv);
	}
# endif

	Nfiles = 1;
	Fileset = pv[0].pv_val.pv_str;

	/* first, the struct relation reldum */
	strcpy(Desc.reldum.relid, pv[3].pv_val.pv_str);
	strcpy(Desc.reldum.relowner, pv[4].pv_val.pv_str);
	Desc.reldum.relspec = pv[5].pv_val.pv_int;
	Desc.reldum.relindxd = pv[6].pv_val.pv_int;
	Desc.reldum.relstat2 = pv[7].pv_val.pv_int;
	Desc.reldum.relstat = pv[8].pv_val.pv_int;
	Desc.reldum.relsave = (long) pv[9].pv_val.pv_int;
	Desc.reldum.reltups = (long) pv[10].pv_val.pv_int;
	Desc.reldum.relatts = pv[11].pv_val.pv_int;
	Desc.reldum.relwid = pv[12].pv_val.pv_int;
	Desc.reldum.relprim = (long) pv[13].pv_val.pv_int;
	Desc.reldum.relfree = (long) pv[14].pv_val.pv_int;
	Desc.reldum.relstamp = (long) pv[15].pv_val.pv_int;
	Desc.reldum.reldim = pv[16].pv_val.pv_int;

	strcpy(Desc.relvname, pv[17].pv_val.pv_str);
	Desc.relfp = pv[18].pv_val.pv_int;
	Desc.relopn = pv[19].pv_val.pv_int;
	Desc.reladds = (long) pv[20].pv_val.pv_int;
	Desc.reltid.ltid = pv[21].pv_val.pv_int;
	j = 22;
	for (i = 0; i <= Desc.reldum.relatts; ++i)
	{
		Desc.reloff[i] = pv[j++].pv_val.pv_int;
		Desc.relfrmt[i] = pv[j++].pv_val.pv_int;
		Desc.relfrml[i] = pv[j++].pv_val.pv_int;
		Desc.relxtra[i] = pv[j++].pv_val.pv_int;
		Desc.relgiven[i] = pv[j++].pv_val.pv_int;
	}

	if (Desc.reldum.reldim > 0)
	{
		if ((Desc.relbtree = (DESC *) calloc(1, sizeof(DESC))) == NULL)
			syserr("bad calloc in ksort");
		/* first, the struct relation reldum */
		strcpy(Desc.relbtree->reldum.relid, pv[j++].pv_val.pv_str);
		strcpy(Desc.relbtree->reldum.relowner, pv[j++].pv_val.pv_str);
		Desc.relbtree->reldum.relspec = pv[j++].pv_val.pv_int;
		Desc.relbtree->reldum.relindxd = pv[j++].pv_val.pv_int;
		Desc.relbtree->reldum.relstat2 = pv[j++].pv_val.pv_int;
		Desc.relbtree->reldum.relstat = pv[j++].pv_val.pv_int;
		Desc.relbtree->reldum.relsave = pv[j++].pv_val.pv_int;
		Desc.relbtree->reldum.reltups = pv[j++].pv_val.pv_int;
		Desc.relbtree->reldum.relatts = pv[j++].pv_val.pv_int;
		Desc.relbtree->reldum.relwid = pv[j++].pv_val.pv_int;
		Desc.relbtree->reldum.relprim = pv[j++].pv_val.pv_int;
		Desc.relbtree->reldum.relfree = pv[j++].pv_val.pv_int;
		Desc.relbtree->reldum.relstamp = pv[j++].pv_val.pv_int;
		Desc.relbtree->reldum.reldim = pv[j++].pv_val.pv_int;

		strcpy(Desc.relbtree->relvname, pv[j++].pv_val.pv_str);
		Desc.relbtree->relfp = pv[j++].pv_val.pv_int;
		Desc.relbtree->relopn = pv[j++].pv_val.pv_int;
		Desc.relbtree->reladds = pv[j++].pv_val.pv_int;
		Desc.relbtree->reltid.ltid = pv[j++].pv_val.pv_int;

		for (i = 0; i <= Desc.relbtree->reldum.relatts; ++i)
		{
			Desc.relbtree->reloff[i] = pv[j++].pv_val.pv_int;
			Desc.relbtree->relfrmt[i] = pv[j++].pv_val.pv_int;
			Desc.relbtree->relfrml[i] = pv[j++].pv_val.pv_int;
			Desc.relbtree->relxtra[i] = pv[j++].pv_val.pv_int;
			Desc.relbtree->relgiven[i] = pv[j++].pv_val.pv_int;
		}
	}

# ifdef xZTR1
	if (tTf(37,0))
	{
		lprintf(" Desc read in \n");
		printdesc(&Desc);
	}
#endif

	/* set up Descsort to indicate the sort order for tuple */
	/* if domain zero is given prepare to generate "hash bucket"
	** value for tuple */

	maxkey = 0;
	for (i = 0; i <= Desc.reldum.relatts; i++)
		if (j = Desc.relgiven[i])
		{
			if ((rev = j) < 0)
				j = -j;
			if (maxkey < j)
				maxkey = j;
			Descsort[--j] = rev < 0 ? -i : i;
		}

	Descsort[maxkey] = ENDKEY;	/* mark end of list */

	Tupsize = Desc.reldum.relwid;

	if (Bucket = (Descsort[0] == 0))
	{
		/* we will be generating hash bucket */
		Tupsize += BUCKETSIZE;
		Desc.relfrml[0] = BUCKETSIZE;
		Desc.relfrmt[0] = INT;
		Desc.reloff[0] = Desc.reldum.relwid;
	}

# ifdef xZTR1
	if (tTf(37,0))
	{
		lprintf("ksort: reldum.relatts is %d\n", Desc.reldum.relatts);
		lprintf("Bucket is %d,Sort is:\n", Bucket);
		for (i = 0; (j = Descsort[i]) != ENDKEY; i++)
			lprintf("Descsort[%d]=%d\n", i, j);
	}
# endif
	if (i = (maxkey - Bucket - Desc.reldum.relatts))
	{
		lprintf("MAXKEY=%d\n", maxkey);
		lprintf("ATTS=%d\n", Desc.reldum.relatts);
		syserr("%d domains missing\n", -i);
	}
	Infile = pv[1].pv_val.pv_str;
	Outfile = pv[2].pv_val.pv_str;

	/* get up to 2**15 - 1 bytes of memory for buffers */
	/* note that mem must end up positive so that Nlines computation is right */
	mem = MEM;	/* take at most 2**15 - 1 bytes */
	if (firstime)
	{
		while ((Lspace = (char **) malloc(mem)) == NULL)
			mem -= 1024;
		firstime = 0;
	}

	/* compute pointers and sizes into buffer memory */
	Nlines = mem / (Tupsize + sizeof(char *));
	Tspace = (char *) (Lspace + Nlines);
# ifdef xZTR1
	if (tTf(37,0))
		lprintf("Tspace=%x,Lspace=%x,Nlines=%x,mem=%d\n",
			Tspace, Lspace, Nlines, mem);
# endif

	/* set up temp files */
	concat(ztack("_SYSS", Fileset), "Xaa", File);
	Filep = File;
	while (*Filep != 'X')
		Filep++;
	Filep++;

	if (abs(Desc.reldum.relspec) == M_ORDER)
		if ((Btree_fp = fopen(Infile, "r")) == NULL)
			syserr("can't open %s", Infile);

	/* sort stage -- create a bunch of temporaries */
	Ccount = 0;
# ifdef xZTR1
	if (tTf(37,0))
		lprintf("sorting\n");
# endif
	sort();
# ifdef xZTR1
	if (tTf(37,0))
	{
		lprintf("done sorting\n%ld tuples written to %d files\n", Tupsout, Nfiles - 1);
		lprintf("sort required %ld compares\n", Ccount);
	}
# endif

	/* merge stage -- merge up to N temps into a new temp */
	Ccount = 0;
	for (i = 1; i + N < Nfiles; i += N) 
	{
		newfile();
		merge(i, i + N);
	}

	/* merge last set of temps into target file */
	if (i != Nfiles) 
	{
		oldfile();
		merge(i, Nfiles);
	}
# ifdef xZTR1
	if (tTf(37,0))
	{
		lprintf("%ld tuples in out file\n", Tupsout);
		lprintf("merge required %ld compares\n", Ccount);
	}
# endif
	term(0);
}
/*
**  SORT
*/

sort()
{
	register char	*cp;
	register char	**lp;
	register int	i;
	int		done;
	long		ntups;
	struct tup_id	tid, ltid;
	char		*xp;
	long		pageid;
	long		rhash();
	char		btree[MAXNAME + 4], btreefile[MAXNAME + 4];
	char		relfile[MAXNAME + 4], btreestruct[MAXNAME + 4];

	done = 0;
	ntups = 0;
	Tupsout = 0;
	if (abs(Desc.reldum.relspec) != M_ORDER)
	{
		if ((Desc.relfp = open(Infile, O_RDONLY)) < 0)
			cant(Infile);
		Desc.relopn = (Desc.relfp + 1) * 5;
	}
	if (Desc.reldum.reldim > 0 && abs(Desc.reldum.relspec != M_ORDER))
	/* open all needed btree files */
	{
		capital(Desc.reldum.relid, btree);
		ingresname(btree, Desc.reldum.relowner, btreefile);
		if ((Desc.relbtree->relfp = open(btreefile, O_RDONLY)) < 0)
			cant(btreefile);
		Desc.relbtree->relopn = (Desc.relbtree->relfp + 1) * 5;
		ingresname(Desc.reldum.relid, Desc.reldum.relowner, relfile);
		btreename(relfile, btreestruct);
		if ((Desc.btree_fd = open(btreestruct, O_RDWR)) < 0)
			cant(btreestruct);
	}

	/* initialize tids for full scan */
	pageid = 0;
	tid.line_id = -1;
	stuff_page(&tid, &pageid);
	pageid = -1;
	ltid.line_id = -1;
	stuff_page(&ltid, &pageid);

	do 
	{
		cp = Tspace;
		lp = Lspace;
		while (lp < Lspace + Nlines)
		{
			if (abs(Desc.reldum.relspec) == M_ORDER)
			{
				/* not reading from a relation */
				if ((i = fread(cp, 1, Desc.reldum.relwid, Btree_fp)) != Desc.reldum.relwid)
				{
					if (i != 0)
						syserr("read error %d", i);
					fclose(Btree_fp);
					done++;
					break;
				}
			}
			else if ((i = kget(&Desc, &tid, &ltid, cp, TRUE)) != 0)
			{
				if (i < 0)
					syserr("get %d", i);
				close(Desc.relfp);
				Desc.relopn = 0;
				done++;
				break;
			}
# ifdef xZTR1
			if (tTf(37,0))
				printup(&Desc, cp);
# endif
			if (Bucket)
			{
				/* compute hash bucket and insert at end */
				pageid = rhash(&Desc, cp);
				bmove(&pageid, cp + Desc.reldum.relwid, BUCKETSIZE);
			}
			*lp++ = cp;
			cp += Tupsize;
			ntups++;
		}
		qsort(Lspace, lp - Lspace, sizeof(char *), cmpa);
		if (done == 0 || Nfiles != 1)
			newfile();
		else
			oldfile();
		while (lp > Lspace) 
		{
			cp = *--lp;
			xp = cp;
			if ((lp == Lspace) || (i = abs(cmpa(&xp, &lp[-1]))) != 0 || (i == 0 && abs(Desc.reldum.relspec) == M_ORDER))
			{
# ifdef xZTR1
				if (tTf(37,0))
				{
					lprintf("writing ");
					printup(&Desc, cp);
				}
# endif
				if ((i = fwrite(cp, 1, Tupsize, Oiop)) != Tupsize)
					syserr("cant write outfile %d (%d)", i, Nfiles);
				Tupsout++;
			}
		}
		fclose(Oiop);
	} while (done == 0);
	if (Desc.reldum.reldim > 0 && Desc.reldum.relspec != M_ORDER)
	{
		close(Desc.relbtree->relfp);
		Desc.relbtree->relopn = 0;
		close(Desc.btree_fd);
	}
# ifdef xZTR1
	if (tTf(37,0))
		lprintf("%ld tuples in\n", ntups);
# endif
}
/*
**  MERGE
*/

struct merg
{
	char		tup[MAXTUP+BUCKETSIZE];
	int		filedes;
	FILE		*fiop;
};

merge(a, b)
int	a;
int	b;
{
	register struct merg	*merg;
	register int		i, j;
	char			*f, *yesno;
	struct merg		*mbuf[N + 1];
	char			*setfil();

# ifdef xZTR1
	if (tTf(37,0))
		lprintf("merge %d to %d\n", a, b);
# endif
	merg = (struct merg *) Lspace;
	j = 0;
	for (i = a; i < b; i++) 
	{
		f = setfil(i);
		mbuf[j] = merg;
		merg->filedes = i;
		if ((merg->fiop = fopen(f, "r")) == NULL)
			cant(f);
		if (!rline(merg))
			j++;
		merg++;
	}

	i = j - 1;
# ifdef xZTR1
	if (tTf(37,0))
		lprintf("start merg with %d\n", i);
# endif
	while (i >= 0) 
	{
# ifdef xZTR1
		if (tTf(37,0))
			lprintf("mintup %d\n", i);
# endif
		if (mintup(mbuf, i, cmpa))
		{
			if (fwrite(mbuf[i]->tup, 1, Tupsize, Oiop) != Tupsize)
				syserr("cant write merge output");
			Tupsout++;
		}
		merg = mbuf[i];
		if (rline(merg))
		{
			yesno = "not ";
# ifdef xZTR1
			if (!tTf(37,0))
			{
				/* truncate temporary files to zero length */
				yesno = "";
				close(creat(setfil(merg->filedes), 0600));
			}
# endif
# ifdef xZTR1
			if (tTf(37,0))
				lprintf("dropping and %struncating %s\n", yesno, setfil(merg->filedes));
# endif
			i--;
		}
	}

	fclose(Oiop);
}
/*
**	Mintup puts the smallest tuple in mbuf[cnt-1].
**	If the tuple is a duplicate of another then
**	mintup returns 0, else 1.
**
**	Cnt is the number of compares to make; i.e.
**	mbuf[cnt] is the last element.
*/

mintup(mbuf, cnt, cmpfunc)
struct merg	*mbuf[];
int		cnt;
int		(*cmpfunc)();
{
	register struct merg	**next, **last;
	struct merg		*temp;
	register int		nodup;
	int			j;

	nodup = TRUE;
	next = mbuf;
	last = &next[cnt];

	while (cnt--)
	{
		if (j = (*cmpfunc)(last, next))
		{
			/* tuples not equal. keep smallest */
			if (j < 0)
			{
				/* exchange */
				temp = *last;
				*last = *next;
				*next = temp;
				nodup = TRUE;
			}
		}
		else
			nodup = FALSE;

		next++;
	}
	return (nodup);
}


rline(mp)
struct merg	*mp;
{
	register struct merg	*merg;
	register int		i;

	merg = mp;
	if ((i = fread(merg->tup, 1, Tupsize, merg->fiop)) != Tupsize)
	{
		if (i == 0)
		{
			fclose(merg->fiop);
			return (1);
		}
		syserr("rd err %d on %s", i, setfil(merg->filedes));
	}
	return (0);
}

newfile()
{
	char	*setfil();

	makfile(setfil(Nfiles));
	Nfiles++;
}
/*
**	Convert the number i to a char
**	sequence aa, ab, ..., az, ba, etc.
*/

char *
setfil(i)
int	i;
{
	register int	j;

	j = i;
	j--;
	Filep[0] = j/26 + 'a';
	Filep[1] = j%26 + 'a';
	return (File);
}

oldfile()
{
	makfile(Outfile);
	Tupsout = 0;
}
/*
**	Create a file by the name "name"
**	and place its fio pointer in Oiop
*/

makfile(name)
char	*name;
{
	if ((Oiop = fopen(name, "w")) == NULL)
		cant(name);
}

cant(f)
char	*f;
{
	syserr("open %s", f);
}

term(error)
int	error;
{
	register int	i;

	if (Nfiles == 1)
		Nfiles++;
# ifdef xZTR1
	if (tTf(37,0))
		lprintf("temp files not removed\n");
	else
# endif
		for (i = 1; i < Nfiles; i++) 
		{
			unlink(setfil(i));
		}
	return(error);
}
/*
**  CMPA -- compare tuples
*/

cmpa(a, b)
char	**a;
char	**b;
{
	int			af[4];
	int			bf[4];
	char			*pa, *pb;
	register union anytype	*tupa, *tupb;
	int			dom;
	register int		frml;
	int			frmt;
	int			off;
	int			temp;
	int			rt;
	char			*dp;

	pa = *a;
	pb = *b;
	Ccount++;
	dp = Descsort;
	while ((temp = *dp++) != ENDKEY)
	{
		if ((dom = temp) < 0)
			dom = -temp;
		frml = Desc.relfrml[dom];
		frmt = Desc.relfrmt[dom];
		off = Desc.reloff[dom];
		tupa = (union anytype *) &pa[off];
		tupb = (union anytype *) &pb[off];
		if (temp < 0)
		{
			tupb = tupa;
			tupa = (union anytype *) &pb[off];
		}
		if (frmt == CHAR)
		{
			frml &= I1MASK;
			if (rt = scompare(tupb, frml, tupa, frml))
				return (rt);
			continue;
		}

		/* domain is a numeric type */
		if (bequal(tupa, tupb, frml))
			continue;
		/* copy to even word boundary */
		bmove(tupa, af, frml);
		bmove(tupb, bf, frml);
		tupa = (union anytype *) af;
		tupb = (union anytype *) bf;

		switch (frmt)
		{

		  case INT:
			switch (frml)
			{

			  case 1:
				return (tupa->i1type > tupb->i1type ? -1 : 1);

			  case 2:
				return (tupa->i2type > tupb->i2type ? -1 : 1);

			  case 4:
				return (tupa->i4type > tupb->i4type ? -1 : 1);
			}

		  case FLOAT:
			switch (frml)
			{

			  case 4:
				return (tupa->f4type > tupb->f4type ? -1 : 1);

			  case 8:
				return (tupa->f8type > tupb->f8type ? -1 : 1);
			}
		}
	}
	return (0);
}
/*
**	KGET_PAGE
**	Replacement for access method routine get_page();
**	and associated globals and routines.
*/

long		Accuread, Accuwrite;

kget_page(d, tid)
register DESC	*d;
struct tup_id	*tid;
{
	register int		i;
	long			pageid;
	register struct accbuf	*b;
	extern struct accbuf	*choose_buf();

# ifdef xZTR1
	if (tTf(37,0))
	{
		lprintf("kget_page: %.14s,", d->reldum.relid);
		dumptid(tid);
	}
# endif
	pluck_page(tid, &pageid);
	if ((b = choose_buf(d, pageid)) == NULL)
	{
# ifdef xZTR1
		if (tTf(37,0))
			lprintf(" choose_buf: buffer not avail \n");
# endif
		return(-1);
	}
	top_acc(b);

	i = 0;
	if (b->thispage != pageid)
	{
# ifdef xZTR1
		if (tTf(37,0))
			lprintf("kget_page: rdg pg %ld\n", pageid);
# endif
		b->thispage = pageid;
		if ((lseek(d->relfp, pageid * PGSIZE, 0) < 0) ||
		    ((read(d->relfp, b, PGSIZE)) != PGSIZE))
		{
			i = AMREAD_ERR;
		}
		Accuread++;
	}
	return (i);
}

/*
**  KGET - get a single tuple
**
**	Get either gets the next sequencial tuple after
**	"tid" or else gets the tuple specified by tid.
**
**	If getnxt == TRUE, then tid is incremented to the next
**	tuple after tid. If there are no more, then get returns
**	1. Otherwise get returns 0 and "tid" is set to the tid of
**	the returned tuple.
**
**	Under getnxt mode, the previous page is reset before
**	the next page is read. This is done to prevent the previous
**	page from hanging around in the am's buffers when we "know"
**	that it will not be referenced again.
**
**	If getnxt == FALSE then the tuple specified by tid is
**	returned. If the tuple was deleted previously,
**	get retuns 2 else get returns 0.
**
**	If getnxt is true, limtid holds the the page number
**	of the first page past the end point. Limtid and the
**	initial value of tid are set by calls to FIND.
**
**	returns:
**		<0  fatal error
**		0   success
**		1   end of scan (getnxt=TRUE only)
**		2   tuple deleted (getnxt=FALSE only)
*/


kget(d, tid, limtid, tuple, getnxt)
register DESC	*d;
register TID	*tid;
TID		*limtid;
int		getnxt;
char		*tuple;
{
	register int	i;
	long		pageid, lpageid;

#	ifdef xATR1
	if (tTf(23, 0) || tTf(37,0))
	{
		lprintf("kget: %.14s,", d->reldum.relid);
		dumptid(tid);
		lprintf("kget: lim");
		dumptid(limtid);
	}
#	endif
	if (kget_page(d, tid))
	{
		return (-1);
	}
	if (getnxt)
	{
		pluck_page(limtid, &lpageid);
		do
		{
			while (((++(tid->line_id)) & I1MASK) >= Acc_head->nxtlino)
			{
				tid->line_id = -1;
				pageid = Acc_head->ovflopg;
				stuff_page(tid, &pageid);
				if (pageid == 0)
				{
					pageid = Acc_head->mainpg;
					stuff_page(tid, &pageid);
					if (pageid == 0 || pageid == lpageid + 1)
						return (1);
				}
				if (i = resetacc(Acc_head))
					return (i);
				if (i = get_page(d, tid))
					return (i);
			}
		} while (!Acc_head->linetab[-(tid->line_id & I1MASK)]);
	}
	else
	{
		if (i = invalid(tid))
			return (i);
	}
	get_tuple(d, tid, tuple);
#	ifdef xATR2
	if (tTf(23, 1) || tTf(37,0))
	{
		printf("kget: ");
		printup(d, tuple);
	}
#	endif
	return (0);
}
