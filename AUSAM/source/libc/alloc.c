#
/*
 *	C storage allocator
 *	(circular first fit strategy)
 */ 
#define BLOK 512
#define BUSY 01

char *allocs[2] /*initial empty arena*/
{
	&allocs[1],
	&allocs[0]
};
struct
{
	int word;
};
char **allocp &allocs[1];	/*current search pointer*/ 
char **alloct &allocs[1];	/*top of arena (last cell)*/ 

alloc(nbytes)
{
	register int nwords;
	register char **p, **q;
	static char **t;
	static int temp;



	allocs[0].word =| BUSY;	/*static initialization*/ 
	allocs[1].word =| BUSY;

	nwords = (nbytes+(2*sizeof(p)-1))/sizeof(p);
	for(p = allocp; ; )
	{
		do
		{
			if((p->word&BUSY) == 0)
			{
				while(((q = *p)->word&BUSY) == 0)
					*p = *q;
				if(q >= &p[nwords])
					goto found;
			}
			q = p;
			p = p->word&~BUSY;
		}
		while(q >= allocp || p < allocp);
		if((temp = t = sbrk(BLOK*sizeof(p))) == -1)
			return(-1);
		else
			*alloct = t;
		if(t != alloct+1)
			alloct->word =| BUSY;
		alloct = (*t = &t[BLOK]-1);
		*alloct = allocs;
		alloct->word =| BUSY;
	}
found:
	allocp = &p[nwords];
	if(q > allocp)
		*allocp = *p;
	*p = allocp.word|BUSY;
	return(p+1);
}

free(p)
char **p;
{
	register char **r, **s;


	r = allocs;
	do
	{
		s = r->word&~BUSY;
	}
	while(s != p-1 && (r = s) != allocs);
	if(r == allocs)
	{
		abort("free error");
	}
	allocp = p-1;
	allocp->word =& ~BUSY;
}
