/* function igetbt fortran callable for integer*2.    PLW 8/5/79*/
/* For integer x, return right shifted the n bits counting from
   low to high and beginning at p where the lowest order bit is
   0 and an integer is 15 bits plus the sign bit.               */
int h_getbit(x,p,n)
        unsigned int *x,*p,*n;
{
	unsigned int r;
	r=~(~0 << *n);
	return((((r << *p) & *x) >> *p) & r);
}
