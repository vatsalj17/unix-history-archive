/* IAND fortran callable.    PLW 8/5/79.*/
long int i_iand(x,y)
        long int *x,*y;
{
	return(*x & *y);
}
