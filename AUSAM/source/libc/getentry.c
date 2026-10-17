#
#include <local-system>
#include	<passwd.h>

getentry(fd,pe,te)
register struct pwent	*pe;
register long	*te;
{
/*
 *	reads the "pwent" structure at seek address "*te" into
 *	the structure pointed to by "pe".
 *	Returns 0 for failure, 1 for success;
 */

if(*te < PWTABLEN) return(0);
lseek(fd,*te,0);
if(read(fd,pe,PWENTLEN) != PWENTLEN) return(0);
return(1);
}
