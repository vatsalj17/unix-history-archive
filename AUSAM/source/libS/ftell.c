/*
 * Return file offset.
 * Coordinates with buffering.
 */

#include	"stdio.h"

long	tell();

long ftell(iop)
FILE *iop;
{
	long tres;
	register adjust;

	if (iop->_cnt < 0)
		iop->_cnt = 0;
	if (iop->_flag&_IOREAD)
		adjust = - iop->_cnt;
	else if (iop->_flag&_IOWRT) {
		adjust = 0;
		if (iop->_base && (iop->_flag&_IONBF)==0)
			adjust = iop->_ptr - iop->_base;
	} else
		_error("ftell\n");
	tres = tell(fileno(iop));
	if (tres<0)
		return(tres);
	tres =+ adjust;
	return(tres);
}
