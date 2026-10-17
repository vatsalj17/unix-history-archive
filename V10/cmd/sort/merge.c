/* Copyright 1990, AT&T Bell Labs */
#include <stdlib.h>
#include <string.h>
#include "fsort.h"

#define NMERGE 16

enum { IEOF, IDUP, IOK };

int nextfile;

struct merge {
	char *name;		/* name of file for diagnostics */
	FILE *file;
	struct rec *rec;	/* pointer to the data (and key) */
	short del;		/* delete at close */
	short serial;	/* adjudicates equality for stable sort */
};


struct merge mfile[2*NMERGE];	/* -u needs 2 records per file */
int nmfile = 0;			/* number of initialized structs */
struct merge *flist[2*NMERGE];
int nflist = 0;

static void mergephase(int, char*);
static int insert(void);
static int compare(struct merge*, struct merge*);
extern int link(char*, char*);
extern int unlink(char*);
static void mv(int, int);

static void
recalloc(struct merge *m)
{
	if(m->rec)
		return;
	m->rec = (struct rec*)malloc(MINREC);
	if(m->rec == 0)
		fatal("no space for merge records", "", 0);
	m->rec->next = (struct rec*)((uchar*)m->rec + MINREC);
}

static void
recinit(int j, int n, int flag)
{
	int i;
	struct merge *m;
	while(nmfile < (uflag&&!flag? 2*n: n))
		recalloc(&mfile[nmfile++]);
	for(i=0; i<n; i++) {
		m = &mfile[i];
		m->name = flag? filename(j+i): files[j+i];
		m->file = fileopen(m->name, "r");
		m->serial = i;
		m->del = flag;
		mfile[i+n].name = m->name; /*for uflag*/
		mfile[i+n].file = m->file;
		mfile[i+n].serial = i;
	}
}

static void
recfinal(int n, int flag)
{
	int i;
	if(flag)
		for(i=0; i<n; i++)
			free(mfile[i].name);
}

/* misfortune : fields are parsed in their entirety
   before comparison.  lazy parsing might be in order */
/* flag is 0 for merging input files, 1 for intermediates */

void
merge(int nf, int flag)
{
	char buf[BUFSIZ];
	FILE *input, *output;
	char *name;
	int i, j, bunches, n;
	int nmgd = 0;

	do {
		bunches = (nf+NMERGE-1)/NMERGE;
		
		for(i=bunches,j=0; i>0; nmgd+=n, j+=n, nf-=n,i--) {
			n = (nf+i-1)/i;
			recinit(j, n, flag);
			if(bunches > 1 || !flag && overwrite(nmgd))
				name = filename(nextfile++);
			else
				name = oname;
			mergephase(uflag&&!flag? 2*n: n, name);
			recfinal(n, flag);
		}
		if(name == oname)
			return;
		for(i=nextfile, j=bunches; --j>=0 && --i>j; )
			mv(i, j);
		nf = nextfile = bunches;
		flag = 1;
	} while(nf > 1);

	input = fileopen(name, "r");
	output = fileopen(oname, "w");
	while(n = fread(buf, 1, sizeof(buf), input))
		if(fwrite(buf, 1, n, output) != n)
			fatal("error writing", oname, 0);
	fileclose(input, name);
	unlink(name);
	fileclose(output, oname);
}

static void
mv(int i, int j)
{
	char *old = filename(i);
	char *new = filename(j);
	unlink(new);
	if(link(old,new) == -1 || unlink(old) == -1)
		fatal("cannot move", old, 0);
	free(old);
	free(new);
}

		/* merge 1st n>=1 files in mfile[] */
static void
mergephase(int n, char *name)
{
	int i, c;
	struct merge *m;
	uchar *p, *e;
	FILE *output = fileopen(name, "w");

	nflist = 0;
	for(i=0; i<n; i++) {
		flist[nflist] = &mfile[i];
		while(insert() == IDUP)
			continue;
	}
	while(nflist > 0) {
		m = flist[0];
		p = data(m->rec);
		i = m->rec->dlen;
		e = p + i++;
		c = *e;
		*e = '\n';
		if(fwrite((char*)p, 1, i, output) != i)
			fatal("error writing", oname, 0);
		*e = c;
		nflist--;
		memmove(flist, flist+1, nflist*sizeof(*flist));
		flist[nflist] = m;
		while(insert() == IDUP)
			continue;
	}

	for(i=0; i<n; i++) {
		fileclose(mfile[i].file, 0);
		if(mfile[i].del)
			unlink(mfile[i].name);
	}
	fileclose(output, name);
}

static int
fillrec(struct merge *m)
{
	struct rec *r = getline(m->rec, m->file);
	if(r == 0)
		return IOK;
	if(r == ENDFILE)
		return IEOF;
	free(m->rec);
	m->rec = r;
	return IOK;
}

	/* opportunity for optimization:
	   one call of insert is preceded by memmove, which
           insert will undo right away if there is significant
	   clustering so that successive outputs are likely
	   to come from the same input file */

static int
insert(void)
{
	int i;
	int bot,top, t;
	struct merge *m = flist[nflist];
	struct merge *temp;
	if(fillrec(m) == IEOF)
		return IEOF;
	bot = 0;
	top = nflist;
	while(bot < top) {
		i = (bot+top)/2;
		t = compare(m, flist[i]);
		if(t < 0)
			top = i;
		else if(t > 0)
			bot = i + 1;
		else if(uflag) {
			if(m->serial < flist[i]->serial) {
				temp = flist[i];
				flist[i] = flist[nflist];
				flist[nflist] = temp;
			}
			return IDUP;
		} else if(m->serial < flist[i]->serial)
			top = i;
		else
			bot = i + 1;
	}
	memmove(flist+bot+1,flist+bot,(nflist-bot)*sizeof(*flist));
	flist[bot] = m;
	nflist++;
	return IOK;
}		

static int		
compare(struct merge *mi, struct merge *mj)
{
	uchar *ip, *jp;
	uchar *ei, *ej;
	uchar *trans, *keep;
	int li, lj, k;
	if(simplekeyed) {
		trans = fields->trans;
		keep = fields->keep;
		ip = data(mi->rec);
		jp = data(mj->rec);
		ei = ip + mi->rec->dlen;
		ej = jp + mj->rec->dlen;
		for( ; ; ip++, jp++) {
			while(ip<ei && !keep[*ip]) ip++;
			while(jp<ej && !keep[*jp]) jp++;
			if(ip>=ei || jp>=ej) break;
			k = trans[*ip] - trans[*jp];
			if(k != 0) break;
		}
		if(ip<ei && jp<ej)
			return k*signedrflag;
		else if(ip < ei)
			return signedrflag;
		else if(jp < ej)
			return -signedrflag;
		else if(sflag)
			return 0;
	} else if(keyed) {
		ip = key(mi->rec);
		jp = key(mj->rec);
		li = mi->rec->klen;
		lj = mj->rec->klen;
		for(k=li<lj?li:lj; --k>=0; ip++, jp++)
			if(*ip != *jp)
				break;
		if(k < 0) {
			if(li != lj)
				fatal("theorem disproved","",0);
			if(sflag)
				return 0;
		} else
			return *ip - *jp;
		
	}
	li = mi->rec->dlen;
	lj = mj->rec->dlen;
	ip = data(mi->rec);
	jp = data(mj->rec);
	for(k=li<lj?li:lj; --k>=0; ip++, jp++)
		if(*ip != *jp)
			break;
	return (k<0? li-lj: *ip-*jp)*signedrflag;
}

void
check(char *name)
{
	int i, t;

	recalloc(&mfile[0]);
	recalloc(&mfile[1]);
	mfile[0].file = mfile[1].file = fileopen(name, "r");
	if(mfile[0].file == 0)
		fatal("can't open ", name, 0);

	if(fillrec(&mfile[0]) == IEOF)
		return;
	for(i=1; fillrec(&mfile[i])!=IEOF; i^=1) {
		t = compare(&mfile[i^1], &mfile[i]);
		if(t>0 || t==0 && uflag) {
			if(mfile[i].rec->dlen)
				fatal("disorder:",
				      (char*)data(mfile[i].rec),
				      mfile[i].rec->dlen);
			else
				fatal("disorder at empty record","",0);
		}

	}
}
