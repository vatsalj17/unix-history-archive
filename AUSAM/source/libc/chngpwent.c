#
/*
 *	alters the passwd entry specified by "pe->pw_uid" and "*pe->pw_strings[LNAME]
 *	to contain the data in the area pointed to by "pe".
 *	Returns 1 for success, 0 for failure and -1 for error
 *	Note: the uid and login name MUST match
 */

#include <local-system>
#include	<passwd.h>

char	pwfd, pwfl;
extern char	*etcpasswd;

chngpwent(pe)
register struct pwent	*pe;
{
register struct pwent	*rp;

struct pwent	p;
int	ht;
int	ri;
long	hte, te, newt;


ri = -1;

/*	open and lock	*/

if( !pwfl)
	{
	if((pwfd=open(etcpasswd,2)) < 0) return (-1);
	pwfl++;
	}
writelock(pwfd);

/*	initialise	*/

do	{
	rp = &p;
	if(!getutab(pwfd,pe->pw_uid,&te)) break;
	if(equalentry(pwfd,pe,&te) < 1)
		{
		ri=0;
		break;
		}
	if(!getentry(pwfd,rp,&te)) break;
	
	/* got entry ok */
	
	pe->pw_next = rp->pw_next;
	pe->pw_last = rp->pw_last;
	
	/* add new entry before deleting old */
	
	if(addentry(pwfd,pe,&newt) <= 0)break;
	
	/* now newt is the address of the added entry */
	
	if(pe->pw_last == PWENTNULL)
		{
		/* beginning of list - update the hash ptr */
		ht=pwhash(pe->pw_strings[LNAME]);
		if(!gethtab(pwfd,ht,&hte))break;
		if(hte != te) break;
		if(!puthtab(pwfd,ht,&newt)) break;
		}
	
	if(pe->pw_next != PWENTNULL)
		{
		/* this element has a next in list */
		if(!getentry(pwfd,rp,&pe->pw_next))break;
		rp->pw_last=newt;
		if(!putentry(pwfd,rp,&pe->pw_next))break;
		}
	
	if(pe->pw_last != PWENTNULL)
		{
		/* this element has a last in list */
		if(!getentry(pwfd,rp,&pe->pw_last))break;
		rp->pw_next=newt;
		if(!putentry(pwfd,rp,&pe->pw_last))break;
		}
	
	/* done, zap the uid entry and that is all */
	
	if(!pututab(pwfd,pe->pw_uid,&newt))break;
	
	ri=1;
	} while(0);

unlock();

return(ri);
}
