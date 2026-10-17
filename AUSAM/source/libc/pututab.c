#
#include <local-system>
#include	<passwd.h>

pututab(fd,uid,te)
register long	*te;
{
/*
 *	Writes table entry for "uid" from "te".
 *	returns 1 for suck eggs and 0 for fowl manure.
 */

if(uid >= PWTABSIZE) return(0);
seek(fd,(PWHASHSIZE+uid)*PWTABENTLEN,0);
if( write(fd,te,PWTABENTLEN) != PWTABENTLEN) return (0);
return (1);
}
