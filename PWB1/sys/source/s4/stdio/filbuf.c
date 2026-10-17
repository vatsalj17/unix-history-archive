#include	"stdio.h"
_filbuf(iop)
register struct _iobuf *iop;
{
	static char smallbuf[_NFILE];

	if ((iop->_flag&_IOREAD) == 0)
		_error("Reading bad file\n");
	if (iop->_flag&_IOSTRG)
		return(-1);
tryagain:
	if (iop->_base==NULL) {
		if (iop->_flag&_IONBF) {
			iop->_base = &smallbuf[fileno(iop)];
			goto tryagain;
		}
		if ((iop->_base = malloc(BUFSIZ)) == NULL) {
			iop->_flag =| _IONBF;
			goto tryagain;
		}
		iop->_flag =| _IOMYBUF;
	}
	iop->_ptr = iop->_base;
	iop->_cnt = read(fileno(iop), iop->_ptr, iop->_flag&_IONBF?1:BUFSIZ);
	if (--iop->_cnt < 0) {
		if (iop->_cnt == -1)
			iop->_flag =| _IOEOF;
		else
			iop->_flag =| _IOERR;
		iop->_cnt = 0;
		return(-1);
	}
	return(*iop->_ptr++&0377);
}
