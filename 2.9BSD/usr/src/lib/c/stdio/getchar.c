/*	@(#)getchar.c	2.1	SCCS id keyword	*/
/*
 * A subroutine version of the macro getchar.
 */
#include <stdio.h>

#undef getchar

getchar()
{
	return(getc(stdin));
}
