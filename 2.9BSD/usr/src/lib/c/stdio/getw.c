/*	@(#)getw.c	2.1	SCCS id keyword	*/
#include	<stdio.h>

getw(iop)
register struct _iobuf *iop;
{
	register i;

	i = getc(iop);
	if (iop->_flag&_IOEOF)
		return(-1);
	return(i | (getc(iop)<<8));
}
