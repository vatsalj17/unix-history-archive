#include	"stdio.h"

ungetc(c, iop)
register struct _iobuf *iop;
{
	if ((iop->_flag&_IOREAD)==0 || iop->_ptr <= iop->_base)
		if (iop->_ptr == iop->_base && iop->_cnt==0)
			*iop->_ptr++;
		else
			_error("ungetc\n");
	iop->_cnt++;
	*--iop->_ptr = c;
}
