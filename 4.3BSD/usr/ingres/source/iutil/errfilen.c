# include	<ingres.h>
# include	<aux.h>
# include	<version.h>
# include	<sccs.h>

SCCSID(@(#)errfilen.c	8.1	12/31/84)

/*
** Errfilen() -- Returns the pathname where the error file can be found
**
**	It is assumed that the error digit cannot be more than 999
*/

char *
errfilen(digit)
int	digit;
{
	extern char	*ztack(), *iocv();

	return (ztack(ztack(ztack(ztack(Pathname, "/files/error"), VERSION), "_"), iocv(digit)));
}
