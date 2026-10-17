/*ISHFT fortran callable.    PLW 8/7/79.*/
/* For integer x shift by amount y.
   y >0 shift left, right fill with 0
   y =0 no shift
   y <0 shift right, left fill with 0's even if sign
        bit is on                                   */
long int i_ishft(x,y)
	long int *x,*y;
{
	if (*y > 0)
		return (*x << *y);
	else if (*y < 0)
		return ((*x >>(-*y)) & (~(~0 << (32 + *y))));
	else
		return (*x);
}
