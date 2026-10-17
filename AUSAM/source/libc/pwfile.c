#
/*
 *	The password file name is altered to that passed
 */

#include <local-system>
#include	<passwd.h>

char	pwfd, pwfl;
extern char	*etcpasswd;

pwfile(s)
char	*s;
{
	if ( pwfl )
	{
		close( pwfd );
		pwfl = 0;
	}

	etcpasswd = s;
}
