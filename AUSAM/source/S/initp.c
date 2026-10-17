/*
 *	program to set a users passwd to null, and set initial shell to
 *	/usr/logins/initsh, mostly used after a user has forgotten his passwd.
 *
 *		initp username1 [username2 .. usernamen]
 *
 *	Peter Ivanov, 22/7/79
 *
 */

#include	<local-system>
#include	<passwd.h>

struct pwent	pe;
char	buf[256];
char	is[]	"/usr/logins/initsh";

main(argc, argv)
register int	argc;
char	*argv[];
{
register	i;
register	size;

if(argc < 2) error("usage: initp  lname1 [lname2 .. lnamen]\n");

argc--;
argv++;

while(argc > 0)
	{
	pe.pw_strings[LNAME] = *argv;
	if((size = getpwuid(&pe, buf, sizeof buf)) == sizeof buf)
		error("buffer too small\n");

	if(size > 0)
		{
		/* got an entry, zap it */
		pe.pw_strings[SHELLPATH] = is;
		for(i=0; i < 8; pe.pw_pword[i++] = '\0');
		if(chngpwent(&pe)  != 1)
			error("cant change passwd file\n");
		}
	  else
		{
		/* no such person */
		prints(2,pe.pw_strings[LNAME]);
		prints(2," no such user!\n");
		}
	argc--;
	argv++;
	}
}

error(s)
char	*s;
{
prints(2,"zaptime: ");
prints(2,s);
exit(1);
}
