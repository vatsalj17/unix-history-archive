/*	@(#)fgetc.c	2.1	SCCS id keyword	*/
#include <stdio.h>

fgetc(fp)
FILE *fp;
{
	return(getc(fp));
}
