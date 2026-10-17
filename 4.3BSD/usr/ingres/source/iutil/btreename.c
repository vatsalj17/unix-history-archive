# include	<ingres.h>
# include	<sccs.h>

SCCSID(@(#)btreename.c	8.1	12/31/84)

btreename(relname, outname)

char *relname, *outname;
{
	char	*temp;

	temp = outname;
	bmove(relname, temp, MAXNAME + 2);
	smove("B\0", temp + MAXNAME + 2);
}
