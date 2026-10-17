#
#define ASSERT(p)
#ifdef debug
#define ASSERT(p) if(!(p)){botch("p");}
botch(s)
char *s;
{
	printf("assertion botched: %s\n",s);
	abort();
}
#endif
/*	C storage allocator
 *	circular first-fit strategy
 *	works with noncontiguous, but monotonically linked, arena
 *	each block is preceded by a ptr to the (pointer of) 
 *	the next following block
 *	blocks are even number of words long; low
 *	bit in ptr is 1 for busy, 0 for idle
 *	gaps in arena are merely noted as busy blocks
 *	last block of arena (pointed to by alloct) is empty and
 *	has a pointer to first
 *	idle blocks are coalesced during space search
*/
/*	all these defines must be powers of 2 */
#define BLOCK 1024
#define BYTESPERPTR 2
#define BUSY 01
#define NULL 0

char *sbrk();

struct { char *ptr; };
struct { char byte[2*BYTESPERPTR]; };
struct { char *ptrs[2]; } allocs	/* initial arena */
{
	&allocs.byte[1*BYTESPERPTR]+BUSY,
	&allocs.byte[0*BYTESPERPTR]+BUSY 
};

char *allocp &allocs.byte[BYTESPERPTR];	/*current search ptr*/
char *alloct &allocs.byte[BYTESPERPTR];	/*last cell of arena*/
char *allocx;		/*for benefit of realloc*/
#define block allocx	/*perhaps gratuitous space saving*/

malloc(nbytes)
{
	register nb;
	register char *p, *q;

	nb = (nbytes+BYTESPERPTR+BYTESPERPTR-1)&~(BYTESPERPTR-1);
	ASSERT(allocp>(&allocs.byte[0])&&allocp<=alloct);
	for(p=allocp; ; ) {
		for(allocx=0; ; ) {
			if(!(p->ptr&BUSY)) {
				while(!((q=p->ptr)->ptr&BUSY)) {
					ASSERT(q>p);
					ASSERT(q<alloct);
					p->ptr = q->ptr;
				}
				if(q>=p+nb && p+nb>p)
					goto found;
			}
			q = p;
			p = p->ptr & ~BUSY;
			if(p>q) {
				ASSERT(p<=alloct);
			} else if(q!=alloct||p!=&allocs.byte[0]) {
				write(2,"corrupt arena\n",14);
				exit(0175);
			} else if(++allocx>1)
				break;
		}
		block = (nb+BYTESPERPTR+BLOCK-1)&~(BLOCK-1);
		if((q = sbrk(block)) == -1)
			return(NULL);
		ASSERT(q>alloct);
		alloct->ptr = q;
		if(q != alloct+BYTESPERPTR)
			alloct->ptr =| BUSY;
		alloct = (q->ptr = q+block-BYTESPERPTR);
		alloct->ptr = allocs.byte+BUSY;
	}
found:
	allocp = p+nb;
	ASSERT(allocp<=alloct);
	if(q>allocp) {
		allocx = allocp->ptr;
		allocp->ptr = p->ptr;
	}
	p->ptr = allocp|BUSY;
	return(p+BYTESPERPTR);
}

/*	freeing strategy tuned for LIFO allocation
*/
free(p)
char *p;
{
	ASSERT((p-BYTESPERPTR)->ptr & BUSY);
	(allocp = p-BYTESPERPTR)->ptr =& ~BUSY;
	ASSERT(allocp->ptr>=allocs.ptrs[1]&&allocp->ptr<alloct);
	ASSERT(allocp->ptr > allocp);
}
