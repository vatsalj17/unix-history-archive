/*
char id_printn_[] = "@(#)printn_.c	1.1";
 *
 * Fortran-callable formatted output routines
 * Bruce R. Julian, USGS, March 1980
 */
#include <stdio.h>
#include "ioprim.h"

printn_(fmt, args)
char *fmt;
{
	doprnt(fmt, &args, stdout);
}

fprntn_(f, fmt, args)
FILEID *f;
char *fmt;
{
	FILE *getfile();

	doprnt(fmt, &args, getfile(*f));
}

char *sprntn_(str, ls, fmt, args)
char *str, *fmt;
STRLARG ls;
{
	FILE _strbuf;
	char *end;

	_strbuf._flag = _IOWRT+_IOSTRG;
	_strbuf._ptr = str;
	_strbuf._cnt = (int)ls;
	doprnt(fmt, &args, &_strbuf);
	end = str + ls;
	while (_strbuf._ptr < end)
		putc(' ', &_strbuf);
}
