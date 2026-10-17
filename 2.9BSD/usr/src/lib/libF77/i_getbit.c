/* function igetbt fortran callable for integer*4.   PLW 8/5/79*/
/* For integer x, return right shifted the n bits counting from
   low to high and beginning at p where the lowest order bit is
   0 and an integer is 31 bits plus the sign bit.               */
long int i_getbit(x,p,n)
        long int *x,*p,*n;
{
	long int r;
        if (*n == 32) r=~0;
	else r=~(~0 << *n);
	return((((r << *p) & *x) >> *p) & r);
}
