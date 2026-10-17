#
#include <local-system>
#include	<passwd.h>

equalentry(fd,pe,te)
register struct pwent *pe;
register long	*te;
{
/*
 *	Checks to see if the password entry pointed to by
 *	the long pointed to by "te"
 *	has a "login name" the same as that pointed to
 *	by "pe->pw_strings[LNAME]".
 *	returns 1 for equal, 0 for not equal and -1 for error.
 */

struct pwent	p1;
char	buf[LOGLEN];

register struct pwent	*rp1;
register	loglen;

rp1 = &p1;

/*	seek and read */

lseek(fd,*te,0);

/*	assume the login name is < LOGLEN chars, and zap the LOGLENth to ensure
	strings compare ok if its not	*/

if(read(fd,rp1,PWENTLEN) != PWENTLEN) return (-1);
if( (loglen=rp1->pw_strings[LNAME]) > LOGLEN) loglen=LOGLEN;
if(read(fd,buf,loglen) != loglen) return (-1);
buf[LOGLEN-1]='\0';
if( strcmp(pe->pw_strings[LNAME], &buf) != 0 ) return (0);
return (1);
}
