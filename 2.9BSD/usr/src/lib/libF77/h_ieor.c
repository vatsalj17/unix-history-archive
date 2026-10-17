/* IEOR fortran callable.     PLW 8/7/79.*/
int h_ieor(x,y)
	int *x,*y;
{
	return(*x ^ *y);
}
