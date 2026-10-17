#include	"stdio.h"

rewind(iop)
register struct _iobuf *iop;
{
	fflush(iop);
	seek(fileno(iop), 0, 0);
	iop->_cnt = 0;
	iop->_ptr = iop->_base;
	iop->_flag =& ~(_IOERR|_IOEOF);
}
