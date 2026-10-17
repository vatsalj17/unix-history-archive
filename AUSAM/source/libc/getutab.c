#

/*
 *	Reads table entry for "uid" into "te".
 *	returns 1 for suck eggs and 0 for fowl manure.
 */

#include <local-system>
#include	<passwd.h>

getutab(fd,uid,te)
register long	*te;
register int	uid;
{
if( (uid < 0) || (uid >= PWTABSIZE) ) return(0);
seek(fd,(PWHASHSIZE+uid)*PWTABENTLEN,0);
if( read(fd,te,PWTABENTLEN) != PWTABENTLEN ) return (0);
return (1);
}
