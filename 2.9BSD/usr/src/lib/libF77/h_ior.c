/* IOR fortran callable.     PLW 8/5/79.*/
int h_ior(x,y)
        int *x,*y;
{
	return(*x | *y);
}
