# include	"../unix.h"

/*
**  GAMMA -- log gamma function
**
**	Version seven does not define this routine, so here it
**	is.  Actually it just syserr's.
**
**	History:
**		8/15/79 (eric) (6.2/7) -- written
*/

# ifdef xV7_UNIX
double
gamma(arg)
double	arg;
{
	syserr("gamma unknown");
}
# endif
