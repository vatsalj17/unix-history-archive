/*
char id_scann_[] = "@(#)scann_.c	1.1";
 *
 * Fortran-callable formatted input routines
 * Bruce R. Julian, USGS, March 1980
 */
#include <stdio.h>
#include "ioprim.h"
FORTINT doscan();

FORTINT scann_(fmt, args)
char *fmt;
{
	return(doscan(stdin, fmt, &args));
}

FORTINT fscann_(f, fmt, args)
FILEID *f;
char *fmt;
{
	FILE *getfile();

	return(doscan(getfile(*f), fmt, &args));
}

FORTINT sscann_(str, fmt, args)
register char *str;
char *fmt;
{
	FILE _strbuf;

	_strbuf._flag = _IOREAD|_IOSTRG;
	_strbuf._ptr = _strbuf._base = str;
	_strbuf._cnt = 0;
	while (*str++)
		_strbuf._cnt++;
	return(doscan(&_strbuf, fmt, &args));
}
