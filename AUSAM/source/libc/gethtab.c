#

/*
 *	Reads hashtable entry for "offset" into "te".
 *	returns 1 for suck eggs and 0 for fowl manure.
 */

#include <local-system>
#include	<passwd.h>

gethtab(fd,offset,te)
register long	*te;
register int	offset;
{
if( (offset < 0) || (offset >= PWHASHSIZE) ) return(0);
seek(fd,offset*PWTABENTLEN,0);
if( read(fd,te,PWTABENTLEN) != PWTABENTLEN ) return (0);
return (1);
}
