# include	"../ingres.h"
# include	"../aux.h"
# include	"../fileio.h"
# include	"monitor.h"

/*
**  PRINT QUERY BUFFER
**
**	The logical query buffer is printed on the terminal regardless
**	of the "Nodayfile" mode.  Autoclear is reset, so the query
**	may be rerun.
**
**	Uses trace flag 6
*/

print()
{
	FILE		*iop;
	char		iobufx[IOBUFSIZ];
	register char	c;

	/* BACK UP FILE & UPDATE LAST PAGE */
	Autoclear = 0;
	clrline(1);
	fflush(Qryiop);
	if ((iop = fopen(Qbname, "r", iobufx)) == NULL)
		syserr("print: open 1");

	/* list file on standard output */
	Notnull = 0;
	while ((c = getc(iop)) > 0)
	{
		putchar(c);
		Notnull++;
	}

	fclose(iop);
	cgprompt();
}
