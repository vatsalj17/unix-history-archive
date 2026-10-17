/*	@(#)putchar.c	2.1	SCCS id keyword	*/
/*
 * A subroutine version of the macro putchar
 */
#include <stdio.h>

#undef putchar

putchar(c)
register c;
{
	putc(c, stdout);
}
