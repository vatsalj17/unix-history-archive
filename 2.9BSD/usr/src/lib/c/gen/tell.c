/*	@(#)tell.c	2.1	SCCS id keyword	*/
/*
 * return offset in file.
 */

long	lseek();

long tell(f)
{
	return(lseek(f, 0L, 1));
}
