#
/*
 *	This function deletes the password entry pointed to by "pe".
 *	Both the uid and login name passed must correspond
 *	with the password entry.
 *	Returns 1 for success, 0 for failure, and -1 for error.
 */

#include <local-system>
#include	<passwd.h>

char	pwfd, pwfl;
extern char	*etcpasswd;

delpwent(pe)
register struct pwent	*pe;
{
register struct pwent	*rp;

struct pwent	p;
long	hte,te;
int	ht;
int	ri;

ri = -1;

/*	open and lock */

if( !pwfl)
	{
	if((pwfd=open(etcpasswd,2)) < 0) return (-1);
	pwfl++;
	}
writelock(pwfd);

/*	initialise */

do	{
	rp = &p;
	if(!getutab(pwfd,pe->pw_uid,&te)) break;
	if(te == PWENTNULL) return(0);
	if(equalentry(pwfd,pe,&te) < 1)
		{
		ri=0;
		break;
		}
	if(!getentry(pwfd,rp,&te)) break;
	
	/*	got elements and all ok, now delete */
	
	pe->pw_next=rp->pw_next;
	pe->pw_last=rp->pw_last;
	
	if(pe->pw_last == PWENTNULL)
		{
		/* a beginning of list item - update hash pointers */
		ht=pwhash(pe->pw_strings[LNAME]);
		if(!gethtab(pwfd,ht,&hte)) break;
		if(hte != te) break;
		hte=pe->pw_next;
		if(!puthtab(pwfd,ht,&pe->pw_next)) break;
		}
	
	if(pe->pw_next != PWENTNULL)
		{
		/* this element has a next in list */
		if(!getentry(pwfd,rp,&pe->pw_next)) break;
		rp->pw_last=pe->pw_last;
		if(!putentry(pwfd,rp,&pe->pw_next)) break;
		}
	
	if(pe->pw_last != PWENTNULL)
		{
		/* this element has a last in list */
		if(!getentry(pwfd,rp,&pe->pw_last)) break;
		rp->pw_next=pe->pw_next;
		if(!putentry(pwfd,rp,&pe->pw_last)) break;
		}
	
	/*	done - zap uid and return */
	
	te=PWENTNULL;
	if(!pututab(pwfd,pe->pw_uid,&te)) break;
	
	ri=1;
	} while(0);

unlock();

return(ri);
}
