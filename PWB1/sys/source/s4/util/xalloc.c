static char Sccsid[] "@(#)xalloc	1.3";
# include	"misc.h"

/*
	xalloc/xfree based on alloc/free in C library at one time.
	Also xfreeall() frees all memory allocated (calls brk(II)).
 
	Xfree always coalesces contiguous free blocks.
	Xalloc uses a first fit strategy.
	Xalloc always allocates words (rounds up).
	Xalloc actually allocates one more word than the
	amount requested.  The extra word (the first word of the
	allocated block) contains the size (in bytes) of the entire block.
	This size is used by xfree to identify contiguous blocks,
	and is used by xalloc to implement the first fit strategy.
 
	Bad things will happen if the size word is changed.
	Worse things happen if xfree is called with a
	garbage argument.
 
	Xalloc returns the address of the allocated area on success,
	fatal() on failure.
	Xfree and xfreeall don't return anything.
*/

struct fb {
	unsigned	f_size;
	char		*f_next;
};

int	Freelist[] {
	0,
	-1,
};

# define SLOP	2

extern	end;
unsigned Lastbrk	&end;

xalloc(asize)
unsigned asize;
{
	register unsigned size;
	register struct fb *np, *cp;

	if((size = asize) == 0)
		return(0);
	size =+ 3;
	size =& ~01;
	for(;;) {
		cp = Freelist;
		while((np.integ = cp->f_next) != -1) {
			if(np->f_size>=size) {
			/*
				Don't break the block up if it
				is not more than SLOP bigger than the
				amount needed.
			*/
				if(size+SLOP >= np->f_size)
					cp->f_next = np->f_next;
			/*
				Break the block into 2 pieces.
			*/
				else {
					cp = cp->f_next = np.integ + size;
					cp->f_size = np->f_size - size;
					cp->f_next = np->f_next;
					np->f_size = size;
				}
				return(&np->f_next);
			}
			cp = np;
		}
	/*
		Nothing on the free list is big enough;
		get more core from the operating system.
	*/
		asize = size<1024? 1024: size;
		asize = (asize+31) & (~31);
		if((cp.integ = sbrk(asize)) == -1)
			return(fatal("out of space (ut9)"));
		Lastbrk = cp.integ + asize;
		cp->f_size = asize;
	/*
		Add the new piece to the free list.
	*/
		xfree(&cp->f_next);
	}
}


xfree(aptr)
char *aptr;
{
	register struct fb *ptr, *cp, *np;

	if (aptr && aptr < Lastbrk) {
		ptr = aptr-2;
		cp = Freelist;
		while((np = cp->f_next) < ptr)
			cp = np;
	/*
		Try to coalesce with the following block.
	*/
		if(ptr.integ + ptr->f_size == np.integ) {
			ptr->f_size =+ np->f_size;
			ptr->f_next = np->f_next;
			np = ptr;
		} else
			ptr->f_next = np;
	/*
		Try to coalesce with the preceding block.
	*/
		if(cp.integ + cp->f_size == ptr.integ) {
			cp->f_size =+ ptr->f_size;
			cp->f_next = ptr->f_next;
		} else
			cp->f_next = ptr;
	}
}


xfreeall()
{
	brk(&end);
	Lastbrk = &end;
	Freelist[0] = 0;
	Freelist[1] = -1;
}
