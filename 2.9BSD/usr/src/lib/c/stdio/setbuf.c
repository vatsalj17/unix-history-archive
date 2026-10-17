#include	<stdio.h>

setbuf(iop, buf)
register struct _iobuf *iop;
char *buf;
{
	if (iop->_base != NULL && iop->_flag&_IOMYBUF)
		free(iop->_base);
#ifndef UCB_LINEBUF
	iop->_flag &= ~(_IOMYBUF|_IONBF);
#else UCB_LINEBUF
	iop->_flag &= ~(_IOMYBUF|_IONBF|_IOLBF);
#endif UCB_LINEBUF
	if ((iop->_base = buf) == NULL)
		iop->_flag |= _IONBF;
	else
		iop->_ptr = iop->_base;
	iop->_cnt = 0;
}
