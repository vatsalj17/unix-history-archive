#
#include <local-system>
#include	<passwd.h>
#include	<local-system>

freetable(fd)
{
/*
 *	Scans up the uid table for a free element.
 *	Returns the uid of the free element or -1 for table full or error.
 */

register	ruid;
long	t;

/*	scan the table */

for (ruid=UIDLB; (ruid<PWTABSIZE) && (getutab(fd,ruid,&t)); ruid++)
	if(t == PWENTNULL) return (ruid);

return (-1);
}
