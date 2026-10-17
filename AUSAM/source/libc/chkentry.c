#
#include <local-system>
#include	<passwd.h>

chkentry(fd,pe,te)
register long	*te;
register struct pwent	*pe;
{
/*
 *	Check that the password entry list pointed to by "*te" does NOT
 *	contain the string pointed to by "pe->p_string[LNAME]".
 *	On return "*te" contains the seek address for the last entry
 *	in the list or "PWENTNULL" if the list is null.
 *	Returns 1 if string is NOT in list,
 *		0 if string IS in list and
 *		-1 on error
 */

register struct pwent	*rp;
register loglen;

struct pwent	p;
char 	buf[LOGLEN];

for (rp = &p; *te != PWENTNULL; *te = rp->pw_next)
	{
	lseek(fd,*te,0);
	if (read(fd,rp,PWENTLEN) != PWENTLEN) return (-1);
	if ( (loglen=rp->pw_strings[LNAME]) > LOGLEN) loglen=LOGLEN;
	if (read(fd,&buf,loglen) != loglen) return (-1);
	buf[LOGLEN-1] = '\0';
	if(!strcmp(pe->pw_strings[LNAME], &buf)) return(0);
	if(rp->pw_next == PWENTNULL) break;
	}

/*	now "*TE" is seek address of last entry or null */

return(1);
}
