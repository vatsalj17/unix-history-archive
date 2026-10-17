/* function iputbt fortran callable for integer*2.    PLW 8/5/79*/
/* For integer x, replace the n bits starting at bit p with v.
   The rightmost or lowest bit is number 0 and integers are 15
   bits plus the sign bit. The n bits are counted from lowest
   to highest.                                                */

int h_putbit(x,p,n,v)
	unsigned int *x,*p,*n,*v;
{
	unsigned int r;
	r=~(~0 << *n);
	return(((r & *v) << *p) | (~(r<< *p) & *x));
}
