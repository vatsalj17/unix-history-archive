#
#include <local-system>
#include	<passwd.h>

addentry(fd,pe,te)
register long	*te;
register struct pwent	*pe;
{
/*
 *	Updates the long that "te" points to with the seek address
 *	for the new entry. The new entry is written from "pe".
 *	Returns the sum of variable length string fields written or
 *	-1 for error
 */

extern	long getoffset();

register	i;

char	*strings[PWSLENCNT];
int	total;

for(i=0; i<PWSLENCNT; i++)
	{
	strings[i] = pe->pw_strings[i];
	pe->pw_strings[i] = strlen(strings[i])+1;	/* EH PETE */
	}

/*	pointers saved and changed to counts in "pe" */

*te = getoffset(fd);
lseek(fd,*te,0);

/*	ready to write */

if( write(fd,pe,PWENTLEN) != PWENTLEN ) return (-1);

total=0;
for(i=0; i < PWSLENCNT; i++)
	if(write(fd,strings[i],pe->pw_strings[i]) != pe->pw_strings[i])
		return(-1);
	  else
		total =+ pe->pw_strings[i];

/*	all written, restore pointers to be nice	*/

for(i=0; i<PWSLENCNT; i++)
	pe->pw_strings[i] = strings[i];

/*	now return count	*/

return (total);
}
