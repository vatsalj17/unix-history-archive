/*	@(#)findiop.c	2.1	SCCS id keyword	*/
#include <stdio.h>

FILE *
_findiop()
{
	extern FILE *_lastbuf;
	register FILE *iop;

	for(iop = _iob; iop->_flag & (_IOREAD|_IOWRT|_IORW);)
		if (++iop >= _lastbuf)
			return(NULL);

	return(iop);
}
