/* IEOR fortran callable.     PLW 8/7/79.*/
long int i_ieor(x,y)
	long int *x,*y;
{
	return(*x ^ *y);
}
