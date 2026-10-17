#include	"stdio.h"

fread(ptr, size, count, iop)
register char *ptr;
register struct _iobuf *iop;
{
	register c;
	int ndone, s;

	ndone = 0;
	if (size)
	for (; ndone<count; ndone++) {
		s = size;
		do {
			if ((c = getc(iop)) >= 0)
				*ptr++ = c;
			else
				return(ndone);
		} while (--s);
	}
	return(ndone);
}

fwrite(ptr, size, count, iop)
register char *ptr;
register struct _iobuf *iop;
{
	register s;
	int ndone;

	ndone = 0;
	if (size)
	for (; ndone<count; ndone++) {
		s = size;
		do {
			putc(*ptr++, iop);
		} while (--s);
		if (ferror(iop))
			break;
	}
	return(ndone);
}
