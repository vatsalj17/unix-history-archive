#include "stdio.h"

struct _iobuf *freopen(file, mode, iop)
char *file;
register char *mode;
register struct _iobuf *iop;
{
	register f;

	fclose(iop);
	if (*mode=='w')
		f = creat(file, 0666);
	else if (*mode=='a') {
		if ((f = open(file, 1)) < 0)
			f = creat(file, 0666);
		seek(f, 0, 2);
	} else
		f = open(file, 0);
	if (f < 0)
		return(NULL);
	iop->_file = f;
	if (*mode != 'r')
		iop->_flag =| _IOWRT;
	else
		iop->_flag =| _IOREAD;
	return(iop);
}
