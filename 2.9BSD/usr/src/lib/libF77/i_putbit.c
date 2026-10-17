/* function iputbt fortran callable for integer*4.   PLW 8/5/79*/
/* For integer x, replace the n bits starting at bit p with v.
   The rightmost or lowest bit is number 0 and integers are 31
   bits plus the sign bit. The n bits are counted from lowest
   to highest.                                                */

long int i_putbit(x,p,n,v)
	long int *x,*p,*n,*v;
{
	long int r;
	if (*n == 32)  r=~0;
	else r=~(~0 << *n);
	return(((r & *v) << *p) | (~(r<< *p) & *x));
}
