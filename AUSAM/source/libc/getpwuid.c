#
/*
 *	The password entry with "login name" the same as "*pe->pw_strings[LNAME]
 *	is read into an area pointed to by "pe".
 *	If "size" is non-zero, up to "size" characters
 *	of the string portion of the password entry will be read
 *	into the area pointed to by "sbuf". If necessary the "pwent"
 *	structure string "p_string" pointers will be modified.
 *	They will be loaded with a pointer to the appropriate string
 *	for those strings completely or incompletely read,
 *	or the string length for those strings not read.
 *	Returns length of string portion read
 *	or size (which ever is the smaller), or -1 on error.
 */

#include <local-system>
#include	<passwd.h>
#define	EACCES	13	/* access deniged */

char	pwfd, pwfl;
extern char	*etcpasswd;

getpwuid(pe,sbuf,size)
register struct pwent	*pe;
register char	*sbuf;
register	size;
{
register	i;
int	sum;
int	ri;
long	t;

extern int	errno;

ri = -1;

/*	open and lock */

if( !pwfl)
	{
	if((pwfd=open(etcpasswd,2)) < 0)
		{
		if(errno==EACCES)
			{
			if((pwfd=open(etcpasswd,0)) < 0) return(-1);
			}
		  else
			return(-1);
		}
	pwfl++;
	}
readlock(pwfd);

/*	get pwent */

do	{
	i=pwhash(pe->pw_strings[LNAME]);
	if(!gethtab(pwfd,i,&t)) break;
	if(chkentry(pwfd,pe,&t) != 0) break;
	/* now t contains seek address of entry matching "login name" */
	if(!getentry(pwfd,pe,&t)) break;
	
	/*	get strings also */
	
	sum=0;
	if(size != 0)
		{
		for(i=0; i<PWSLENCNT; i++)
			sum =+ pe->pw_strings[i];
		if(sum > size) sum=size;
	
	/*	read strings */
	
		if(read(pwfd,sbuf,sum) != sum) break;
		}
	
	/*	fix pointers */
	
	for(i=0; (i<PWSLENCNT) && (size>0); i++)
		{
		/* still room for some characters */
		if((size =- (ri = pe->pw_strings[i])) < 0)
			{
			/* cant fit whole string, just a bit of it */
			ri =+ size;
			*(sbuf+ri-1) = '\0';
			}
		/* adjust pointer etc */
		pe->pw_strings[i] = sbuf;
		sbuf =+ ri;
		}
	
	/*	done */
	
	ri=sum;
	} while(0);

unlock();

return(ri);
}
