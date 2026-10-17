/*
 * Seek for standard library.  Coordinates with buffering.
 */

#include	"stdio.h"

fseek(iop, offset, ptrname)
FILE *iop;
long offset;
{
	register n, resync;

	if (iop->_flag&_IOREAD) {
		resync = 0;
		if (ptrname==1) {	/* relative */
			n = iop->_cnt;
			if (n<0)
				n = 0;
		} else {
			n = offset&01;
			resync = n;
		}
		n = lseek(fileno(iop), offset - n, ptrname);
		iop->_cnt = 0;
		if (resync)
			getc(iop);
		return(n);
	}
	if (iop->_flag&_IOWRT) {
		fflush(iop);
		return(lseek(fileno(iop), offset, ptrname));
	}
	_error("fseek\n");
}
