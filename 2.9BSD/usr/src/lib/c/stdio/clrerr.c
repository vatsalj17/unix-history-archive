/*	@(#)clrerr.c	2.1	SCCS id keyword	*/
#include	<stdio.h>

clearerr(iop)
register struct _iobuf *iop;
{
	iop->_flag &= ~(_IOERR|_IOEOF);
}
