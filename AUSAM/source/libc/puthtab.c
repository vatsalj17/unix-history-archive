#
#include <local-system>
#include	<passwd.h>

puthtab(fd,offset,te)
register long	*te;
{
/*
 *	Writes hashtable entry for "offset" from "te".
 *	returns 1 for suck eggs and 0 for fowl manure.
 */

if(offset >= PWHASHSIZE) return(0);
seek(fd,offset*PWTABENTLEN,0);
if( write(fd,te,PWTABENTLEN) != PWTABENTLEN) return (0);
return (1);
}
