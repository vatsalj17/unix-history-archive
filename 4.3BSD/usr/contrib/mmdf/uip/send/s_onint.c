/* name:
	onint, onint2, onint3

function:
	to take appropriate actions before terminating from a del.

calls:
	exit

called by:
	input

*/

/*
**	R E V I S I O N  H I S T O R Y
**
**	03/31/83  GWH	Split the SEND program into component parts
**			This module contains : msgreview, onint, onint2,
**			onint3, confirm.
**
**	10/05/83  GWH	Fixed s_exit so that the draft file will not be
**			removed in abnormal terminations.
**
*/

#include "./s.h"
#include "./s_externs.h"

/*ARGSUSED*/
msgreview (curline)
	register int curline;
{
    FILE *fp;
    register int c;

    fp = fdopen (dup (drffd), "r");
    while ((c = getc (fp)) != EOF && !aborted)
    {
#ifdef FNCYPRNT
	switch (c)
	{
	    case '\n':
		if (++curline <= 20)
		{
		    putchar ('\n');
		    break;
		}
	    case '\f':
		if (c == '\f')
		    printf ("^L");
		putchar ('\n');
		printf (" Continue [Confirm] ");
		fflush (stdout);
		switch (getchar ())
		{
		    case 'Y':
		    case 'y':
			while (getchar () != '\n')
			    continue;
		    case '\n':
			curline = 0;
			break;

		    default:
			while (getchar () != '\n')
			    continue;
			fclose (fp);
			return;
		}
		break;

	    default:
		putchar (c);
	}
#else
	putchar (c);
#endif
    }
    fclose (fp);
}

onint ()
{
    write(1, " XXX\n", 5);
    signal (SIGINT, onint);
    if (inrdwr)
	aborted = TRUE;
    else
	longjmp (savej, 1);
}

onint2 ()
{
    write(1, " XXX\n", 5);
    s_exit (-1);
}

onint3 ()
{
    signal (SIGINT, onint3);

    if (body)
	longjmp (savej, 1);

    s_exit (0);
}

confirm ()
{
    char answer[32];

    clearerr(stdin);	/*  Be friendlier if user types control-D  */
    printf (" [Confirm] ");
    gather (answer, sizeof (answer));
    return (prefix ("yes", answer) ? TRUE : FALSE);
}

/* preliminary exit routine to clean up draft file */
s_exit( xcode )
int xcode;
{
	if( xcode == 0 || !body )
		unlink(drffile);
	exit( xcode );
}
