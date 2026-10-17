#include	"stdio.h"

struct _iobuf *fopen(file, mode)
register char *mode;
{
	register f;
	register struct _iobuf *iop;
	extern struct _iobuf *_lastbuf;

	for (iop = _iob; iop->_flag&(_IOREAD|_IOWRT); iop++)
		if (iop >= _lastbuf)
			return(NULL);
	if (*mode=='w')
		f = creat(file, 0600);
	else if (*mode=='a') {
		if ((f = open(file, 1)) < 0)
			f = creat(file, 0600);
		seek(f, 0, 2);
	} else
		f = open(file, 0);
	if (f < 0)
		return(NULL);
	iop->_flag =& ~(_IOREAD|_IOWRT);
	iop->_file = f;
	if (*mode != 'r')
		iop->_flag =| _IOWRT;
	else
		iop->_flag =| _IOREAD;
	return(iop);
}
