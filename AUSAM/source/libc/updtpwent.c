#
/*
 *	Rewrites the fixed portion of the pwentry whose uid matches "pe->pw_uid"
 *	The string section is not written and neither are the string pointers.
 *	returns 1 for success, 0 for failure and -1 for error
 */

#include <local-system>
#include	<passwd.h>

char	pwfd, pwfl;
extern char	*etcpasswd;

updtpwent(pe)
register struct pwent	*pe;
{
register struct pwent	*rp;
register	ruid, i;

struct pwent	p;
int	ri;
long	t;
char	*strings[PWSLENCNT];


ri = -1;

/*	open and lock	*/

if( !pwfl)
	{
	if((pwfd=open(etcpasswd,2)) < 0) return (-1);
	pwfl++;
	}
writelock(pwfd);

/*	get address	*/

do	{
	rp = &p;
	if(!getutab(pwfd,pe->pw_uid,&t)) break;
	if(!getentry(pwfd,rp,&t)) break;
	
	/* ok got entry, now fix data which must not alter... */
	
	pe->pw_next = rp->pw_next;
	pe->pw_last = rp->pw_last;
	for(i=0; i<PWSLENCNT; i++)
		{
		strings[i]=pe->pw_strings[i]; /* save for later return */
		pe->pw_strings[i] = rp->pw_strings[i];
		}
	
	/* alter entry */
	
	if(!putentry(pwfd,pe,&t)) break;
	
	/* and kindly return string pointers */

	for(i=0; i<PWSLENCNT; i++)
		pe->pw_strings[i]=strings[i];

	ri=1;
	} while(0);

unlock();

return(ri);
}
