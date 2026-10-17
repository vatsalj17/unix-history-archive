#include <local-system>
#include <passwd.h>
#include <stdio.h>
#include <class.h>

/*	to make new accounts:
 *
 *		mkacc CLASS [-c] [-o] [-w]
 *
 *	accepts new members of class CLASS, creating direc. etc, optionally
 *		-c	setting new class mask bit on already existant
 *			accounts (IFF bit 64 is set, CLEAR MASK FIRST)
 *		-o	allows acceptance of OTHER field.
 *		-w	warn if new account number is not 7 chars.
 *
 *	NOTE: The password entry templates are COMPILED IN (quick but nasty)
 *		and so new class templates must be added with care.....
 *
 *	All data comes from the compiled-in templates.
 *
 */
#define	PROMPT(x)	if(prompt) prints(1, x)

#define LOGNAME 100
#define FIRST	50
#define LAST	40
#define OTHERF	100
#define DIREC	100
#define	LOGLENG	7

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

struct pwent tpw;
struct pwent *dp;
char	logname[LOGNAME];
char	first[FIRST];
char	last[LAST];
char	otherf[OTHERF];
char	direc[DIREC];
char	buf[250];
char	prompt, wflag, cflag, oflag, stopf;
char	*class;
char	*dflt	"default";

main(argc, argv)
int argc;
char *argv[];
{
register int	i;
char	ccflag;
int	icnt, size;
char	dflag;
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
			case 'o':	oflag ++;
					break;
			case 'c':	cflag ++;
					break;
			case 'w':	wflag ++;
					break;
			default:	prints(2, "mkacc: bad arg\n");
					exit(1);
			}
		}
	else
		{
		if(class == 0)
			{
			class = *argv;
			}
		else
			{
			prints(2, "mkacc: too many classes\n");
			exit(1);
			}
		}
	}

/* args are now set up */
if(ttyn(0) != 'x')
	prompt ++;

/* prompts set up */
if(!class)
	class = dflt;

while (!stopf)
{
dflag = 0;
ccflag=0;
settplt();
PROMPT("login name: ");
if(gets(logname))
	{
	for(i=0; logname[i] != '\0'; i++);
	if(i != LOGLENG)
		{
		prints(2,logname);
		prints(2," not seven chars !\n");
		}
	tpw.pw_strings[LNAME] = logname;
	size = getpwuid(&tpw, buf, sizeof buf);
	if (size == sizeof buf)
		{
		prints(2,"mkacc: string buffer too small\n");
		exit(1);
		}

	if(size > 0)
		{
		/* there is an entry */
		dflag++;
		prints(1,logname);
		prints(1," user already exists\n");
		if((!cflag) && prompt)
			{
			/* ask about change */
			prints(2,"shall I change?\n");
			gets(direc);
			if(*direc == 'y')
				/* yes */
				ccflag = 1;
			 else
				/* no */
				ccflag = 0;
			}
		if(cflag || ccflag)
			{
			setclass();
			prints(1,logname);
			prints(1," user class mask set\n");
			}
		if(updtpwent(&tpw) != 1)
			error(" update failed\n");
		}
	  else
		{
		/* there is no entry */
		while (1)
			{
			icnt = 0;
			PROMPT("family name: ");
			if(gets(last))
				{
				tpw.pw_strings[LASTNAME] = last;
				icnt++;
				}
			  else
				break;
			PROMPT("other names: ");
			if(gets(first))
				{
				tpw.pw_strings[FIRSTNAME] = first;
				icnt++;
				}
			  else
				break;
			if(oflag)
				{
				PROMPT("other information: ");
				if(gets(otherf))
					{
					tpw.pw_strings[OTHER] = otherf;
					icnt++;
					}
				  else
					break;
				}
			break;
			}
		if( !((icnt = 3) || ((icnt = 2) && oflag)))
			{
			prints(2,"mkacc: last entry not completed\n");
			exit(1);
			}

		if((strlen(tpw.pw_strings[DIRPATH])+strlen(logname))>(DIREC-2))
			{
			prints(2,"mkacc: directory path too long\n");
			exit(1);
			}
		  else
			{
			strcpy(direc,tpw.pw_strings[DIRPATH]);
			strcat(direc,logname);
			tpw.pw_strings[DIRPATH] = direc;
			}

		if(addpwent(&tpw) < 0)
			error(" add failed\n");
		mkit();
		}
	if(dflag && (!prompt))
		{
		gets(buf);
		gets(buf);
		if(oflag) gets(buf);
		}
	}
  else
	{
	prints(2,"mkacc: end\n");
	exit(0);
	}
}
}

mkit()
{
	register int cpid;
	int status;


	if(cpid = fork())
	{
		/* parent */ 
		if(cpid == -1)
		{
			prints(2,"mkacc: cant fork mkdir\n");
			exit(1);
		}
		else
		{
			wait(&status);
			if(status&0177400)
				error(" mkdir failed\n");
			else
			{
				if(chown(tpw.pw_strings[DIRPATH], tpw.pw_uid) < 0)
					error(" chown failed\n");
			}
		}
	}
	else
	{
		/* child */ 
		execl("/bin/mkdir", "mkdir", tpw.pw_strings[DIRPATH], 0);
		error(" cannot exec mkdir\n");
	}
}

error(s1)
char *s1;
{
prints(2,"mkacc: ");
prints(2,logname);
prints(2, s1);
exit(1);
}

settplt()
{
register int	i;

/*
 *	If first time thru, find the struct and point dp at it
 *	else, dp is set so zap straight thru.
 */

if(dp == 0)
	for(i=0; i< (CMASKSIZE*16); i++)
		if(strcmp(class, data[i].da_class) == 0)
			{
			dp = &data[i].da_dpw;
			break;
			}

if(dp == 0)
	{
	prints(2,"mkacc: no such class template\n");
	exit(1);
	}

/* now dp points at the template */

tpw.pw_next = dp->pw_next;
tpw.pw_last = dp->pw_last;
tpw.pw_uid = dp->pw_uid;
tpw.pw_shares = dp->pw_shares;
tpw.pw_usage = dp->pw_usage;

for(i=0; i<CMASKSIZE; i++)
	tpw.pw_cmask[i] = dp->pw_cmask[i];

tpw.pw_flags = dp->pw_flags;
tpw.pw_dlimit = dp->pw_dlimit;
tpw.pw_doverflw = dp->pw_doverflw;
tpw.pw_plimit = dp->pw_plimit;
tpw.pw_climit = dp->pw_climit;
tpw.pw_tmask = dp->pw_tmask;

for(i=0; i<8; i++)
	tpw.pw_pword[i] = dp->pw_pword[i];

tpw.pw_contime = dp->pw_contime;
tpw.pw_cputime = dp->pw_cputime;
tpw.pw_extime = dp->pw_extime;
tpw.pw_warn = dp->pw_warn;
tpw.pw_pages = dp->pw_pages;
#ifdef	TERMBOOK
tpw.pw_tblim = dp->pw_tblim;
tpw.pw_tbrate = dp->pw_tbrate;
#endif	TERMBOOK
for(i=0; i<PWSLENCNT; i++)
	tpw.pw_strings[i] = dp->pw_strings[i];

/* and thats the lot */
}

setclass()
{
register unsigned	*rn, *rm;
register unsigned	rmask;
register int i;

if(tpw.pw_cmask[CMASKSIZE-1] & 01)
	/* must clear all mask */
	for(i=0; i<CMASKSIZE; i++)
		tpw.pw_cmask[i] = 0;

rn = dp->pw_cmask;
rm = tpw.pw_cmask;

for(i=0; i<CMASKSIZE; i++)
	*rm++ =| *rn++;
}

stop()
{
signal(2,1);
signal(3,1);
stopf++;
}
