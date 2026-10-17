/*	DELREMIND command -- Steve Zucker, April, 1976	   */
/*	 Compile -i, as	super user and set mode	04555	   */
#include <stdio.h>
#include <pwd.h>
#include <signal.h>

#define	MAXRCVRS 50
#define	MAXMSG	 1000
#define	DIRSIZE	 254	/* No bigger than 254 */
#define	MAXREMS	 100	/* Maximum that	this program can process at once */
char *rmdlock	= "/usr/spool/rmd/rmdlock";	/* exists while	proc is	busy */
char *reminders	= "/usr/spool/rmd/rmdfile";
char *tempfile	= "/usr/spool/rmd/d.XXXXXX";
char *rmdtemp	= "/usr/spool/rmd/rmdtemp";
char *nofile	= "Unable to open %s\n";
char *norems	= "No reminders\n";
struct rmdblock
{	int	tdeliver[2];
	int	nobytes;
	int	tsent[2];
	char	sender[8];
	char	bits;
	char	dirsize;
	char	nrcvrs;
	int	msgbytes;
	char	rcvr1[8];
} m, hdr[MAXREMS];
#define	PRIORITY 01	/* bits	*/
#define	DELETED	 01	/* mark	bit */
char	marks[MAXREMS];
int	nrmds;
FILE	*fdcopy, *fdrem, *fdtemp;
char	rmdbuff[8*MAXRCVRS-8 + MAXMSG +	DIRSIZE];
char	rmddata[8*MAXRCVRS-8 + MAXMSG +	DIRSIZE];
char	command[160];
char	user[10];	/* user id null padded */

main()
{	register int i;
	register char *p;
	register char c;
	int j;
	int uid;
	struct passwd *pw;
	struct passwd *getpwuid();
	extern errno;
	/* Get user name if not	superuser */
	uid = getuid();
	if((pw=getpwuid(uid))==NULL)
	{
	    fprintf(stderr,nofile,"password file");
	    exit(1);
	}
	strcpy(user,pw->pw_name);
	/* Make	tempfile name */
	p = index(tempfile,'X');
	j = getpid();
	for (i=18; i =-	3; )
		*p++ = ((j>>i)&07) + '0';
	lock();
	if ((fdcopy = fopen(tempfile,"w")) == NULL)
	{	fprintf(stderr,nofile, tempfile);
		exit(1);
	}
	if ((fdrem = fopen(reminders,"r")) == NULL)
	{	fprintf(stderr,norems);
		cleanup();
	}
	for (i=0; i<MAXREMS; )
	{	p = &hdr[i];
		if (fread(p,sizeof m,1,fdrem) != 1) break;
/*
		if (errno) ioerror();
*/
		if (uid == 0 || equal(user,p->sender,8) ||
		   (p->nrcvrs==1 && equal(user,p->rcvr1,8)))
		{	/* Copy	relevant portions of reminder file */
			if(fread(rmddata,p->nobytes-8,1,fdrem)!=1)
			    fprintf(stderr,"Read error\n");
			else
			    if(fwrite(rmddata,p->nobytes-8,1,fdcopy)!=1)
				fprintf(stderr,"write error\n");
/*
			if (errno) ioerror();
*/
			i++;
		}
		else
			fseek(fdrem,(long)p->nobytes-8,1);
	}
	fclose(fdcopy);
	fclose(fdrem);
	unlock();
	if ((nrmds = i)	== 0)
	{	printf(norems);
		cleanup();
	}
	printf ("%d messages\n",i);
	/* Collect and execute one command at a	time */
	for (;;)
	{	for (i=0; i < sizeof command; )
		{  switch(c = getchar())
		   {	case ' ':
			case '\t':
				continue;  /* Ignore blanks and	tabs */
			case '\0':
				cleanup(); /* Exit with	no change on EOT */
			case '\n':
				command[i++] = '\0';
				if (parse()) goto wrapup;
				goto nextcom;
			default:
				command[i++] = c;
		   }
		}
		printf ("Line too long\n");
		while ((c = getchar()) != '\n' && c != '\0') ;
    nextcom:  ;	/* Make	C compiler happy */
	}
wrapup:	/* Delete marked messages if any */
	for (i=0; i<nrmds; i++)
		if (marks[i]&DELETED) break;
	if (i >= nrmds)	cleanup();
	printf ("Marked	reminders are being deleted\n");
	lock();
	if ((fdrem = fopen(reminders,"r")) == NULL)
		cleanup();  /* They're all gone	anyway */
	if ((fdtemp = fopen(rmdtemp,"w")) == NULL)
	{	fprintf(stderr,nofile,rmdtemp);
		cleanup();
	}
	while (fread(&m, sizeof m, 1, fdrem) == 1)
	{	fread(rmddata,m.nobytes-8,1,fdrem);
		for (i=0; i < nrmds; i++)
		{	if (marks[i]&DELETED &&	equal(&m,&hdr[i],sizeof	m))
			{	getcopy(i);
				if (equal(rmdbuff,rmddata,m.nobytes-8))
				{	/* delete (don't copy) it */
					marks[i] = 0;
					goto nocopy;
				}
			}
		}
		fwrite (&m, sizeof m,1,fdtemp);
		fwrite (rmddata,m.nobytes-8,1,fdtemp);
    nocopy:   ;	/* Make	C compiler happy */
	}
	unlink(reminders);
	link(rmdtemp,reminders);
	cleanup();
}
ioerror()
{
	perror("I/O error");
	cleanup();
}
cleanup()
{	unlink(rmdtemp);
	unlink(tempfile);
	unlock();
	exit(0);
}
parse()
{	/* Parse command and carry it out */
	register char *p;
	register int i;
	int first, last;
	char *convert();
	p = command;
	switch(*p++)
	{	default: printf	("Invalid command\nCommands are:\n");
			printf ("  headers [list]\n  show [list]\n");
			printf ("  delete [list]\n  keep [list]\n");
			printf ("  write\n  abort\n");
			return(0);
		case 'w': /* Write   */
			return(1);
		case 'a': /* Abort   */
			cleanup();
		case 'h': /* Headers */
		case 's': /* Show    */
			printf ("%-5s%-9s%-9s%-26s%-25s%-5s\n",
				" No","From","To","Sent","For delivery",
				"Flags");
		case 'd': /* Delete  */
		case 'k': /* Keep    */
		     ; /* Keep C compiler happy	*/
	}
	/* Scan	over other alphabetics */
	while (*p >= 'a' && *p <= 'z') p++;
	if (*p == '\0')
	{	perform(command[0],1,nrmds);
		return(0);
	}
	while (p = convert(p,&first))
	{	if (*p == ',' || *p == '\0')
			perform(command[0],first,first);
		else if	(*p++ == '-')
		{	if ((p = convert(p,&last)) &&
			    (*p	== ',' || *p ==	'\0'))
				perform(command[0],first,last);
			else break;
		}
		else break;
		if (*p++ == '\0') return(0);
	}
	printf ("Syntax	error or index out of range\n");
	return(0);
}
perform(cmnd,first,last)
char cmnd; int first, last;
{	register int i,	j;
	register char *p;
	int k, ii;
	j = (--first <=	--last ? 1 : -1);
	for (i = first;	(i-j) != last; i =+ j)
	{  p = &hdr[i];
	   switch(cmnd)
	   {	case 'd': marks[i] =| DELETED;
			  printf("%5d Marked for deletion\n",i+1);
			  break;
		case 'k': marks[i] =& ~DELETED;
			  printf ("%5d Kept\n",i+1);
			  break;
		case 's':
		case 'h':
			printf("%c%-4d%-8s %-8s	%-26.24s",
				(marks[i]&DELETED ? '*'	: ' '),	i+1,
				p->sender, p->rcvr1, ctime(p->tsent));
			printf ("%-25.24s%c%c\n", ctime(p->tdeliver),
				(p->dirsize ? 'E' : ' '),
				(p->bits&PRIORITY ? 'P'	: ' '));
	  }
	  if (cmnd == 's')
	  {	getcopy(i);
		p = rmdbuff;
#ifdef DEBUG
	fprintf(stderr,"nrcvrs = %d\n",hdr[i].nrcvrs);
#endif DEBUG
		for (k = hdr[i].nrcvrs;	--k>0; p =+ 8)
			printf ("%14s%s\n", " ", p);
		if (k =	hdr[i].dirsize)
		{	printf ("Directory: ");
			/* Skip	over user id byte (++p)	*/
			for(ii=0,++p;ii<k-1;ii++)
			    putchar(*p++);
		}
#ifdef DEBUG
	fprintf(stderr,"msgbytes = %d\n",hdr[i].msgbytes);
#endif DEBUG
		printf ("\nMessage:\n");
		for(ii=0;ii<hdr[i].msgbytes;ii++)
		    putchar(*p++);
		putchar('\n');
	  }
	}
}
getcopy(item)
{	register int i;
	register FILE *f;
	long ftell();

	if((f=fopen(tempfile,"r"))==0)
	{
	    fprintf(stderr,"Cannot open %s\n",tempfile);
	    cleanup();
	}
	for (i=0; i<item; i++)
		if(fseek(f,(long)(hdr[i].nobytes-8),1)==EOF)
		{
		    perror("fseek");
		    cleanup();
		}
	if(fread(rmdbuff,hdr[i].nobytes-8,1,f) != 1)
	{
	    if(feof(f))
	    {
		fprintf(stderr,"Unexpected EOF\n");
		cleanup();
	    }
	    if(ferror(f))
	    {
		perror("fread");
		cleanup();
	    }
	    fprintf(stderr,"HUH??\n");
	}
	fclose(f);
}
char *convert(pp,an)
char *pp; int *an;
{	register char *p;
	register int i;
	p = pp;
	for (i=0; i <= nrmds; p++)
	{	if (*p >= '0' && *p <= '9')
			i = 10*i + *p -	'0';
		else
		{	if (*an	= i) return(p);
			else break;
		}
	}
	return(0);
}
/* Use lock file as semaphore.	Can't run as superuser,	so forks. */
lock()
{	register int i;
	int x;
#ifdef DEBUG
	signal(SIGHUP,SIG_IGN);
	signal(SIGINT,SIG_IGN);
	signal(SIGQUIT,SIG_IGN);
#endif DEBUG
	if (fork())
		wait(&x);
	else
	{	setuid(3);	/* 3 is	bin, NOT root */
		while ((i = creat(rmdlock,0444)) == -1)
			sleep(5);
		close(i);
		exit(0);
	}
}
unlock()
{	unlink(rmdlock);
	unlink(rmdtemp);
	signal(SIGHUP,&cleanup);
	signal(SIGINT,&cleanup);
	signal(SIGQUIT,&cleanup);
}
equal(pp,qq)
char *pp, *qq;
{	register char *p, *q;
	register int i;
	p = pp;
	q = qq;
	for (i = 8; i--; )
		if (*p++ != *q++) return(0);
	return(1);
}
