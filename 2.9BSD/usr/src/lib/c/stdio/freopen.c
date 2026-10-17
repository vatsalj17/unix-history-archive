/*	@(#)freopen.c	2.1	SCCS id keyword	*/
#include <stdio.h>

FILE *
freopen(file, mode, iop)
	char *file, *mode;
	register FILE *iop;
{
	FILE *_endopen();

	fclose(iop);
	return(_endopen(file, mode, iop));
}
