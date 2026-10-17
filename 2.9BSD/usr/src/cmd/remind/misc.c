#include "remind.h"

bye()
{	unlink(rmdproc);
	unlink(rmdlock);
	unlink(reminders);
	exit(1);
}

cleanup()
{
	unlink (reminders);
	link (rmdtemp, reminders);
	unlink (rmdtemp);
	unlink(rmdlock);
}

/*	Forking is complicated by the fact that a process
 *	that exits becomes a "ZOMBIE".	It does not leave
 *	UNIX until it is waited for by its parent.  Thus,
 *	when remind spawns a process, it actually spawns
 *	a child and a grandchild, so that the grandchild,
 *	which actually does the work, will be adopted by
 *	the init process when the child exits.
 */
spawn()
{	register int k, l;
	int pstat;

	while ((k = fork()) == -1) sleep(10);
	if (k)
	{	wait(&pstat);	/* Wait for k below to exit */
		return (1);	/* Returns non-zero in parent */
	}
	else
	{	while ((l = fork()) == -1) sleep(10);
		if (l) exit(0);	/* After creating l, exit. */
		return(0);
	}
}

/* Use lock file as semaphore.  Can't run as superuser, so forks. */
lock()
{       register int i;
	int x;
	if (fork())
		wait(&x);
	else
	{       setuid(3);      /* 3 is bin, NOT root */
		while ((i = creat(rmdlock,0444)) == -1)
			sleep(5);
		close(i);
		exit(0);
	}
}

puserid(p)
char *p;
{	if (m.nrcvrs < MAXRCVRS)
		strxfer(p,rcvrlist[m.nrcvrs++],8);
	else
		fprintf (stderr,"Receiver list too long. '%.8s truncated\n", p);
}

puidfil(p)
char *p;
{	char username[8];
	register FILE *pf;
	register int c, i;

	if ((pf = fopen(p,"r")) == NULL)
	{
		fprintf (stderr,nofile,p);
		exit(1);
	}
	do
	{       while ((c=getc(pf)) == '\n' || c==' ' || c=='\0' || c=='\t') ;
		for (i = 0;
			i<8 && (c!='\n' && c!=' ' && c!='\t'
			&& c!= -1 && c!='\0') ;
			c = getc(pf))
				username[i++] = c;
		if (i < 8) username[i] = '\0';
		if (i > 0) puserid(username);
	} while (c != EOF);
	fclose(pf);
}

pflags(pp)
char *pp;
{	register char *p;
	register int i;
	p = pp;
	switch (*p++)
	{
	  case 'p':
		m.bits |= PRIORITY;
		return;
	  case 'x':
	  case 'e':
		*exdir = getuid();	/* Real user id */
		getwdir();              /* Working directory */
		return;
	  case 'm':
	  case 'r':
		for (i = 0; i<MAXMSG && (msg[i++] = *p++); ) ;
		m.msgbytes = --i;
		return;
	  case 'f':
		puidfile(p);	/* filename of receivers */
		return;
	  case 'l':             /* letter only          */
		lflag = 1;
		return;
	  default:
		printf (invalid, p-2);
	}
}

getmsg()
{	register int i,c;
	register char *cp;
	FILE *input;

	if((input=fdopen(0,"r"))==NULL)
	{
		fprintf(stderr,"Cannot read message\n");
		bye();
	}
	cp = msg;
	for (i = 0; i++<MAXMSG && ((c = getc(input)) != -1); )
		*cp++  = c;
	if (i==MAXMSG) fprintf(stderr,"...Message truncated\n");
	m.msgbytes = --i;
}

/* Get name of working directory to exdir[1]..., its length+1 to m.dirsize */
getwdir()
{       register int i;
	int fd[2], x;

	pipe(fd);
	while ((i = fork()) == -1) sleep(5);
	if (i)
	{	wait(&x);      /* Any address */
		exdir[1] = '/';
		i = read (fd[0], &exdir[2], DIRSIZE-2);
		if (i>0 && exdir[i+1] == '\n')
		{	m.dirsize = i+2;	/* uid, /, (name, \n) */
			close (fd[0]);
			close (fd[1]);
			return;
		}
	}
	else
	{       close (1);
		dup (fd[1]);
		execl("/bin/pwd","remindpwd",0);
	}
	error("Bad working directory");
}

strxfer(pp,qq,nn)
/* Transfer string pp to qq, null padding to nn chars */
char *pp, *qq;
int nn;
{	register char *p, *q;
	register int n;
	p = pp;
	q = qq;
	for (n = nn; n && *p && *p!=' '; n--)
		*q++ = *p++;
	while (n--)
		*q++ = '\0';
}

nametty(flag, tt, nm)
int flag;	/* 1 to get name, 0 to get tty(s).
		 * If name or tty not found in login file (/etc/utmp),
		 * returns 0.  If flag is 1, returns name in nm and 1 as
		 * value.  If flag is 0, returns number of ttys at which
		 * user specified in nm is logged in, filling in the
		 * array tt with the tty names.
		 */
char *tt[], *nm;
{
	register FILE *ut;
	register int i;
	register char *ttp;
	struct utmp logins;
	int tcount = 0;

	ttp = (char *)tt++;
	if ((ut = fopen(loginfile,"r")) == NULL)
	{
		fprintf(stderr,"remind: %s unreadable\n",loginfile);
		return(0);
	}
	while (fread((char *)&logins,sizeof logins,1,ut) == 1)
	{	if (flag)
		{
			if (strcmp(logins.ut_line,ttp)==0)
			{	strxfer(logins.ut_name,nm,8);
				fclose(ut);
				return(1);
			}
		}
		else
		{	for (i = 0; i<8 &&
				logins.ut_name[i] == nm[i];
				i++ ) ;
			if (i >= 8 || (nm[i]=='\0' && logins.ut_name[i]==' '))
			{
				strncpy(ttp,logins.ut_line,8);
				tcount++;
				ttp = tt[tcount];
			}
		}
	}
	fclose(ut);
	return(tcount);
}

checkusers()
/* Verify that all receivers exist (are in password file) */
{	register int i,c;
	register int found;
	char name[8];
	char foundflg[MAXRCVRS];
	FILE *pf;

	if((pf=fopen(usrfile,"r"))==NULL)
	{	fprintf (stderr,nofile, usrfile);
		exit(1);
	}
	for (i = found = m.nrcvrs; i--; ) foundflg[i] = 0;

	do
	{	/* Get a name */
		for (i = 0; (c=getc(pf)) != ':' && i < 8 && c != -1 ; )
			name[i++] = c;
		if (c == ':')
		{	while (i<8) name[i++] = '\0';
			for (i=0; i<m.nrcvrs; i++)
			{       if (foundflg[i]==0 && equal(name,rcvrlist[i]))
					if (--found <= 0)
					{       fclose(pf);
						return(0);
					}
					else foundflg[i] = 1;
			}
			while ((c=getc(pf)) != '\n' && c != -1) ;
		}
	} while (c != -1);
	for (i = 0; i<m.nrcvrs; i++)
		if (foundflg[i] == 0)
			fprintf(stderr,"'%.8s' is not a UNIX user\n",rcvrlist[i]);

	fclose(pf);
	return(1);
}

equal(pp,qq)
char *pp, *qq;
{	register char *p, *q;
	register int i;
	p = pp;
	q = qq;
	for (i = 8; i--; )
	{	if (*p != *q++) return(0);
		if (*p++ ==  '\0') break;
	}
	return(1);
}

error(s)
char *s;
{
	fprintf (stderr,"remind: %s\n",s);
	exit(1);
}
