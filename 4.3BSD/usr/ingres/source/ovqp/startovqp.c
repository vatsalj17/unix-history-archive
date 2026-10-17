# include	<ingres.h>
# include	<symbol.h>
# include	<tree.h>
# include	<aux.h>
# include	"../decomp/globs.h"
# include	"../ctlmod/pipes.h"
# include	<signal.h>
# include	<sccs.h>
# include	<errors.h>
# include	<math.h>
# include	<errno.h>

SCCSID(@(#)startovqp.c	8.3	1/31/86)

/*
**	startovqp is called at the beginning of
**	the execution of ovqp.
*/


startovqp()
{
	extern	flptexcep();

	if (Equel)
		startequel();

	De.ov_tupsfound = 0;	/* counts the number of tuples which sat the qual */
	De.ov_retrieve = De.ov_bopen = FALSE;
	/* catch floating point signals */
	signal(SIGFPE, (int) flptexcep);
}

/*
**	Give a user error for a floating point exceptions
*/
flptexcep()
{
	ov_err(FLOATEXCEP);
}

/* Someone here in Berkeley thinks this is a good idea. */
/* See 4.3 man 3m intro */
double
infnan(arg)
register arg;
{
	extern int	errno;

	switch (arg)
	{
	  case ERANGE:
		errno = ERANGE;
		return(HUGE);
	  case -ERANGE:
		errno = EDOM;
		return(-HUGE);
	  case EDOM:
		errno = EDOM;
		return(0.0);
	}
}
