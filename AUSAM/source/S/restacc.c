/*
 *	A program to look up a login name in a list of the form
 *		loginname /directory/pathname/for/old/files/to/be/restored
 *
 *	and if the login name is in the list execute the following
 *
 *		copy /directory/pathname... %loginname
 *		+ loginname
 *		mail loginname < message about the restoration
 *
 *	The optional arg sets a different filename for the list.
 *
 *	Peter Ivanov, July 1979.
 */

/* some inclusions */

#include	<local-system>
#include	<passwd.h>
#include	<stdio.h>

/* some defines */

#define	BUFSIZE	100
#define	PROMPT(x)	if(prompt) prints(1, x)

/* some global data */

struct pwent pe;
char	buf[BUFSIZE];
char	chkbuf[BUFSIZE];
char	logname[BUFSIZE/2];
int	prompt;
char	*direct;

char	*filist	"/user3/EECF-Maint/accounts/filist";
FILE	*fistruct;

main(argc, argv)
int argc;
char	*argv[];
{
	register int	size;

if(argc == 2)
	{
	/* assume we have a different pathname to open */
	filist = argv[1];
	}

if(ttyn(0) != 'x')
	prompt ++;

PROMPT("login name: ");

while(gets(logname) != 0)
	{
	pe.pw_strings[LNAME] = logname;
	if((size = getpwuid(&pe, buf, sizeof buf)) == sizeof buf)
		error("string buffer too small\n");
	if(size<0)
		{
		/* no such entry */
		prints(2,logname);
		prints(2," no such user\n");
		}
	  else
		{
		/* there is a user, does he have any files ? */
		if(check())
			{
			copy();
			mail();
			}
		}
	PROMPT("login name: ");
	}
}

check()
{
/*
 *	look down the list of people, set direct if applicable and
 *	return 1 for success.
 */

	register char	*s;

	if(fistruct)
		{
		/* the file is open already */
		rewind(fistruct);
		}
	  else
		{
		/* must be opened */
		if((fistruct = fopen(filist, "r")) == 0)
			error("cant find login-directory list\n");
		}

	while(fgets(chkbuf, BUFSIZE, fistruct) != 0)
		{
		/* segment the line and point direct */
		for(s = chkbuf; *s != ' '; s++);
		*s++ = '\0';
		direct = s;
		for( ; *s != '\n'; s++);
		*s = '\0';
		/* is this the one ? */
		if(strcmp(logname, chkbuf) == 0) return(1);

		/* no it is not */
		}

	/* get here for a miss */
	return(0);
}

copy()
{
/*
 *	here we have to do a copy and change the owners of things
 *	copy from direct to pe.pw_strings[DIRPATH]
 *	then + logname.
 */

	register int cpid;
	int status;

	if(cpid = fork())
		{
		/* parent */ 
		if(cpid == -1)
			{
			error("cant fork copy\n");
			}
		else
			{
			wait(&status);
			if(status&0177400)
				error(" copy failed\n");
			}
		}
	  else
		{
		/* child */
		execl("/bin/copy", "restcopy", "-r", direct, pe.pw_strings[DIRPATH], 0);
		error("cant exec copy\n");
		}

	/* now the chown */

	if(cpid = fork())
		{
		/* parent */ 
		if(cpid == -1)
			{
			error("cant fork shell of +\n");
			}
		else
			{
			wait(&status);
			if(status&0177400)
				error(" shell of + failed\n");
			}
		}
	  else
		{
		/* child */
		execl("/bin/sh", "restsh", "/etc/bin/+", logname, 0);
		error("cant exec shell of +\n");
		}
}

mail()
{
/*
 *	tell logname that his files are indeed back....
 */

int	status;
int fd;

if(status = fork())
	{
	if(status < 0)
		error("cant fork mail\n");
	wait(&status);
	if(status & 0177400)
		error("mail failed\n");
	return(0);
	}

close(0);
open("/usr/lib/pwe/restacc.mail", 0);
execl("/bin/mail", "restacc.mail", logname, 0);
error("Cannot exec mail\n");
}

error(ss)
char	*ss;
{
prints(2,"restacc: ");
prints(2,ss);
exit(1);
}
