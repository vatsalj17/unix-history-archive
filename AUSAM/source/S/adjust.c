/*
 *	A program to examine the classes a user is in, pick the class with
 *	the largest disc limits and install said user in said class, adjusting
 *	said users limits in the process.
 *	Flags are:
 *		-w	mail user of the change
 *		-l	always adjust limits
 *		-f	force adjustment of important limits (ie disc, proc)
 *
 *	This program:
 *		checks to see the user is not logged on,
 *		works out if a change is necessary,
 *			moves directories using ln or copy
 *			adjust passwd entry if less than possible
 *		warns of people who are over supposed maximum limits.
 *
 *	NOTE: The password entry templates are COMPILED IN (quick but nasty)
 *		and so new class templates must be added with care.....
 *
 *	All data comes from the compiled-in templates.
 *
 *	Useful for end-of-session cleanups, and giving people what they
 *	deserve........   tee hee.......
 *
 *	Peter Ivanov,       June 1979.
 *
 */

/*	include all the inclusions......	*/

#include	<wtmp.h>
#include	<local-system>
#include	<passwd.h>
#include	<lnode.h>
#include	<stdio.h>

/*	global data	*/

#define	PROMPT(x)	if(prompt) prints(1, x)

#define LOGNAME 100
#define DIREC	100

struct data
	{
	char	*da_class;
	struct pwent	da_dpw;
	}
	data[CMASKSIZE*16]
	{
	/* the compiled-in templates */
#		include <classincludes.h>
	};

struct pwent upw;
struct pwent *dp;
struct lnode	lnod;
char	logname[LOGNAME];
char	direc[DIREC];
char	buf[250];
char	*oldirec;
int	prompt, lflag, wflag, fflag;
char	stopf;


main(argc, argv)
int	argc;
char	*argv[];
{
register int	eh;
register int	size;
extern stop();

signal(2,stop);
signal(3,stop);
while (argc-- > 1)
	{
	if(**++argv == '-')
		{
		while(*++*argv)
			switch(**argv)
			{
			case 'l':	lflag++;
					break;
			case 'w':	wflag++;
					break;
			case 'f':	fflag++;
					break;
			default:	error("Usage: adjust [-lwf]\n");
			}
		}
	else
		{
		error("Usage: adjust [-lwf]\n");
		}
	}

	/* args are now set up */
	if(ttyn(0) != 'x')
		prompt ++;

	/* prompts set up */
	
	while (!stopf)
	{
	PROMPT("login name: ");
	if(gets(logname))
		{
		upw.pw_strings[LNAME] = logname;
		size = getpwuid(&upw, buf, sizeof buf);
		if (size == sizeof buf)
			{
			error("string buffer too small\n");
			}
	
		if(size > 0)
			{
			/* there is an entry */

			/* see is the user is logged in, via lnode */
			lnod.l_uid = upw.pw_uid;
			if(limits(&lnod, L_OTHLIM) < 0)
				{
				/* all ok */
				if(eh = adjust())
					{
					if((eh == 1) && (updtpwent(&upw) != 1))
						error("updtpwent failed\n");
					if((eh == 2) && (chngpwent(&upw) != 1))
						error("chngpwent failed\n");
					if(wflag) mail();
					}
				}
			  else
				{
				/* dont touch, cos he is logged in */
				prints(1,logname);
				prints(1," is presently logged in\n");
				}
			}
		  else
			{
			/* there is no entry */
			prints(1, logname);
			prints(1, " no such login name\n");
			}
		}
	  else
		{
		prints(2,"adjust: end\n");
		exit(0);
		}
	}
}

mvit()
{
	register int cpid;
	register char	*c, *sl;
	int status;
	char	od[DIREC], nd[DIREC];

	if(cpid = fork())
		{
		/* parent */ 
		if(cpid == -1)
			{
			error(" cannot fork copy\n");
			}
		/* await the completeion if mails are to be done */
		if(wflag)
			{
			while(wait(&status) >= 0);
			}
		}
	else
		{
		/* child */ 
/*	bloody copy chucks away empty directories if used like this......	*/
/*		execl("/bin/copy", "adjust.copy", "-m", "-s", "-o", "-r", oldirec, direc, 0);*/


/*	use links to move the directory, as long as on same disc	*/
		strcpy(od, oldirec);
		strcpy(nd, direc);
		strcat(od,"/..");
		for(c=nd,sl=0; *c; c++)
			{
			if(*c == '/')	sl=c;
			}
/*	now sl should be null or point to the '/' to be zapped	*/
		if(sl == 0) error(" cant make new pathname/..\n");
		*sl = '\0';
/*	suck it and see	*/
		if(link(oldirec,direc)<0)
			error(" cant relink directory\n");
/*	now we know that links are possible so do em	*/
		if(unlink(od) < 0) prints(2," cant unlink ..\n");
		if(link(nd, od) < 0) prints(2,"cant link ..\n");
		if(unlink(oldirec) < 0) prints(2,"cant unlink oldirec\n");

/*	and that is all	*/

		exit(0);
		}
}

error(s1)
char *s1;
{
prints(2,"adjust: ");
prints(2, s1);
exit(1);
}

adjust()
{
register int	i;
register int	j;
register int	firstclass;
int	candobetter, inaclass;

/*
 *	Set tpw structure to contain the class with largest disc limit.
 *	If the disc limits is larger and not dlfag, warn but do not alter.
 *	Make global "dp" a pointer to the template in core.
 *	Return 0 for no change needed, 1 for updtpwent, 2 for chngpwent.
 */

	firstclass = 0;
	inaclass = 0;
	candobetter = 0;
	for(i=0; i < (CMASKSIZE*16); i++)
		if((upw.pw_cmask[i/16]) & ( 0100000 >> (i%16)))
			{
			firstclass = i;
			inaclass++;
			break;
			}
	dp = &data[firstclass].da_dpw;

	/* now dp should point to a valid class if any */

	for(i=firstclass; i< (CMASKSIZE*16); i++)
		{
		if(data[i].da_dpw.pw_cmask[i/16] & upw.pw_cmask[i/16])
			if(data[i].da_dpw.pw_dlimit >  dp->pw_dlimit)
				dp = &data[i].da_dpw;
		}

	if(dp->pw_dlimit > upw.pw_dlimit) candobetter++;

	/*
	 * now dp should point to the best possible class, if any.
	 * now have a number of possible actions:
	 *	if(dflag) force adjustment of limits
	 *	if(candobetter) adjust limits and directory
	 *		including case where (oldirec != direc)
	 */

	/* now the new directory string - and save the old one */
	strcpy(direc, dp->pw_strings[DIRPATH]);
	oldirec = upw.pw_strings[DIRPATH];
	strcat(direc, logname);
	upw.pw_strings[DIRPATH] = direc;

	/* must he be moved ?? */
	if(strcmp(direc,oldirec) != 0) candobetter++;

	/* now he is in a class or he is not in a class */
	if(inaclass)
		{
		/* adjust his limits? */
		if(candobetter || lflag)
			{
prints(2,"\n");
prints(2,logname);
prints(2,": adjusting limits\n");
			if(fflag)
				{
				/* force adjustment */
				upw.pw_dlimit = dp->pw_dlimit;
				upw.pw_doverflw = dp->pw_doverflw;
				upw.pw_plimit = dp->pw_plimit;
				upw.pw_tmask = dp->pw_tmask;
				upw.pw_shares = dp->pw_shares;
				upw.pw_flags = dp->pw_flags;
				}
			  else
				{
				/* check them all */
				if(upw.pw_dlimit > dp->pw_dlimit)
					warn("dlimit");
				  else
					upw.pw_dlimit = dp->pw_dlimit;
				if(upw.pw_doverflw > dp->pw_doverflw)
					warn("doverflw");
				  else
					upw.pw_doverflw = dp->pw_doverflw;
				if(upw.pw_plimit > dp->pw_plimit)
					warn("plimit");
				  else
					upw.pw_plimit = dp->pw_plimit;
				if(upw.pw_tmask > dp->pw_tmask)
					warn("tmask");
				  else
					upw.pw_tmask = dp->pw_tmask;
				if(upw.pw_shares > dp->pw_shares)
					warn("shares");
				  else
					upw.pw_shares = dp->pw_shares;
				}
			upw.pw_flags = dp->pw_flags;
			upw.pw_climit = dp->pw_climit;
			upw.pw_pages = dp->pw_pages;
#			ifdef	TERMBOOK
			upw.pw_tblim = dp->pw_tblim;
			upw.pw_tbrate = dp->pw_tbrate;
#			endif	TERMBOOK
			if(candobetter == 0) return(1);
			/* now have to move the directory */
			mvit();
			return(2);
			}
		  else
			return(0);
		}

/* and here he is not in any class and so should not have been looked at */
	prints(2,logname);
	prints(2," has no classes\n");
	return(0);

/* and thats the lot */
}

stop()
{
signal(2,1);
signal(3,1);
stopf++;
}

mail()
{
int fd;

if(fork())
	return(0);

close(0);
open("/usr/lib/pwe/adjust.mail", 0);
execl("/bin/mail", "adjust.mail", logname, 0);
error("Cannot exec mail\n");
}

warn(s)
char	*s;
{
prints(1,logname);
prints(1,": present ");
prints(1,s);
prints(1," limit too large\n");
}
