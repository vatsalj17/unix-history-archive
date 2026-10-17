/*	@(#)putw.c	2.1	SCCS id keyword	*/
#include	<stdio.h>

putw(i, iop)
register i;
register struct _iobuf *iop;
{
	putc(i, iop);
	putc(i>>8, iop);
}
