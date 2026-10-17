#
/*	realloc(p,bytes) reallocates a block obtained from malloc()
 *	to have a new size nbytes
 *	returns new location, or -1 on failure
*/
#define BYTESPERPTR 2
#define NULL 0
struct { char *ptr; };

char *alloc();
char *allocx;

char *realloc(p,nbytes)
register char *p;
unsigned nbytes;
{
	register char *q;
	char *s, *t;
	register unsigned nb;
	nb = (p-BYTESPERPTR)->ptr - p;
	nbytes = (nbytes+BYTESPERPTR-1) & ~(BYTESPERPTR-1);
	q = malloc(nbytes);
	if(q==NULL || q==p)
		return(q);
	s = p;
	t = q;
	if(nbytes<nb)
		nb = nbytes;
	while(nb--!=0)
		*t++ = *s++;
	if(q<p && q+nbytes>p)
		(q+(q+nbytes-p))->ptr = allocx;
	return(q);
}
