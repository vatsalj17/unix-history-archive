/*	alloc - old-style memory allocator
 *	returns -1 on fail rather than 0
*/
alloc(n)
{
	register p;
	p = malloc(n);
	return(p?p:-1);
}
