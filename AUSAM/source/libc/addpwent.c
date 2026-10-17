#
/*
 *	This function adds the entry pointed to by "pe" to a
 *	password file. If the pointer at uidtable(pe->pw_uid)==NULL
 *	then this is the new uid. If not then new uid is allocated.
 *	Returns the new uid or -1 for error
 */

#include <local-system>
#include	<passwd.h>

char	pwfd, pwfl;
extern char	*etcpasswd;

addpwent(pe)
register struct pwent	*pe;
{

register struct pwent	*rp;

struct pwent p;
long	t, hte;
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
	pe->pw_next = PWENTNULL;
	ht=pwhash(pe->pw_strings[LNAME]);
	if( !gethtab(pwfd,ht,&pe->pw_last)) break;
	
	/*	now list from "pe->pw_last" may/maynot be null and
		may/maynot contain the entry.	*/
	
	if( chkentry(pwfd,pe,&pe->pw_last) < 1) break;
	
	/*	now "pe->pw_last" is the seek address for the last list element
		or "PWENTNULL" for a null list	*/
	
	if(( !getutab(pwfd,pe->pw_uid,&hte)) || (hte != PWENTNULL))
		{
		if((pe->pw_uid=freetable(pwfd)) < 0) break;
		}
	
	if(pe->pw_last == PWENTNULL)
		{
		/* no list, so just add it in */
		if(addentry(pwfd,pe,&t) <= 0) break;
		if(!puthtab(pwfd,ht,&t)) break;
		if(!pututab(pwfd,pe->pw_uid,&t)) break;
		}
	  else
		{
		/* here for a list at least one long, so ad on at end */
		if(!getentry(pwfd,rp,&pe->pw_last)) break;
		if(addentry(pwfd,pe,&rp->pw_next) <= 0) break;
		/* now "pe->pw_last" contains the seek address of the last list
		   element stored in "p". "rp->pw_next" contains the seek address
		   for the new element contained in "*pe". "pe->pw_uid" is the
		   new uid, and only remains to write data out */
		if(!pututab(pwfd,pe->pw_uid,&rp->pw_next)) break;
		if(!putentry(pwfd,rp,&pe->pw_last)) break;
		}
	
	/*	return uid and that is all */
	
	ri=pe->pw_uid;
	} while(0);

unlock();

return(ri);
}
