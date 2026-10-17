#include	"stdio.h"

fprintf(iop, fmt, args)
struct _iobuf *iop;
char *fmt;
{
	_doprnt(fmt, &args, iop);
}
