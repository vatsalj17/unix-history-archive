/* IOR fortran callable.     PLW 8/5/79.*/
long int i_ior(x,y)
        long int *x,*y;
{
	return(*x | *y);
}
