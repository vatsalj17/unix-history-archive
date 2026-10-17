/*	@(#)fputc.c	2.1	SCCS id keyword	*/
#include <stdio.h>

fputc(c, fp)
FILE *fp;
{
	return(putc(c, fp));
}
