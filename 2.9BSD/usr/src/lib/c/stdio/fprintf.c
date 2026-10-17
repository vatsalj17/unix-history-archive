/*	@(#)fprintf.c	2.1	SCCS id keyword	*/
#include	<stdio.h>

fprintf(iop, fmt, args)
FILE *iop;
char *fmt;
{
	_doprnt(fmt, &args, iop);
	return(ferror(iop)? EOF: 0);
}
