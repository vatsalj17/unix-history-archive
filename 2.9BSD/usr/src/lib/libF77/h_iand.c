/* IAND fortran callable.    PLW 8/5/79.*/
int h_iand(x,y)
        int *x,*y;
{
	return(*x & *y);
}
