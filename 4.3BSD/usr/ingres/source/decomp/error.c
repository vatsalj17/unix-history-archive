# include	<ingres.h>
# include	<aux.h>
# include	<tree.h>
# include	<symbol.h>
# include	"globs.h"
# include	<sccs.h>

SCCSID(@(#)error.c	8.1	12/31/84)


derror(eno)
{
	endovqp(NOACK);
	reinit();
	error(eno, 0);
}
