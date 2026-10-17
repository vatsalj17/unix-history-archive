#
/*
 *	The passwd file is closed and the flag reset.
 */

#include <local-system>
#include	<passwd.h>

char	pwfd, pwfl;

pwclose()
{
	if ( pwfl )
	{
		close(pwfd);
		pwfl=0;
	}
}
