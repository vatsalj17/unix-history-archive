#
#include <local-system>
#include	<passwd.h>

putentry(fd,pe,te)
register struct pwent	*pe;
register long	*te;
{
/*
 *	writes the "pwent" structure pointed to by "pe" to
 *	seek address "*te".
 *	Returns 0 for failure, 1 for success;
 */

if(*te < PWTABLEN) return(0);
lseek(fd,*te,0);
if(write(fd,pe,PWENTLEN) != PWENTLEN) return(0);
return(1);
}
