#include <stdio.h>
#include <pwd.h>
#include <utmp.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <setjmp.h>
#include <whoami.h>

#define	MSGS		/* pipe mail to "msgs" to /usr/ucb/msgs */
/*copylet flags */
	/*remote mail, add rmtmsg */
#define REMOTE	1
	/* zap header and trailing empty line */
#define ZAP	3
#define ORDINARY 2
#define	FORWARD	4
#define	LSIZE	256
#define	MAXLET	300	/* maximum number of letters */
#define	MAILMODE (~0600)		/* mode of created mail */

char	line[LSIZE];
char	resp[LSIZE];
struct let {
	long	adr;
	char	change;
} let[MAXLET];
int	nlet	= 0;
char	lfil[50];
long	iop, time();
char	lettmp[] = "/tmp/maXXXXX";
char	maildir[] = "/usr/spool/mail/";
char	mailfile[] = "/usr/spool/mail/xxxxxxxxxxxxxxxxxxxxxxx";
char	dead[] = "dead.letter";
char	forwmsg[] = " forwarded\n";
char	*curlock;
int	lockerror;
FILE	*tmpf;
FILE	*malf;
char	*my_name;
struct	passwd	*getpwuid();
int	error;
int	locked;
int	changed;
int	forward;
char	from[] = "From ";
long	ftell();
int	delete();
char	*ctime();
int	flgf;
int	flgp;
int	delflg = 1;
jmp_buf	sjbuf;
char	hostname[32];

main(argc, argv)
char **argv;
{
	register i;
	char sobuf[BUFSIZ];

	setbuf(stdout, sobuf);
	mktemp(lettmp);
	unlink(lettmp);
	if (my_name == NULL) {
		struct passwd *pwent;
		pwent = getpwuid(getuid());
		if (pwent==NULL)
			my_name = "???";
		else
			my_name = pwent->pw_name;
	}
	if(setjmp(sjbuf)) done();
	for (i = SIGHUP; i <= SIGTERM; i++)
		setsig(i, delete);
	tmpf = fopen(lettmp, "w");
	if (tmpf == NULL) {
		fprintf(stderr, "mail: cannot open %s for writing\n", lettmp);
		done();
	}
	if (argv[0][0] != 'r' &&	/* no favors for rmail*/
	   (argc == 1 || argv[1][0] == '-'))
		printmail(argc, argv);
	else
		sendmail(argc, argv);
	done();
}

setsig(i, f)
int i;
int (*f)();
{
	if(signal(i, SIG_IGN) != SIG_IGN)
		signal(i, f);
}

printmail(argc, argv)
char **argv;
{
	int flg, i, j, print;
	char *p, *getarg();

	setuid(getuid());
	cat(mailfile, maildir, my_name);
	for (; argc>1; argv++, argc--) {
		if (argv[1][0]=='-') {
			if (argv[1][1]=='q')
				delflg = 0;
			else if (argv[1][1]=='p') {
				flgp++;
				delflg = 0;
			} else if (argv[1][1]=='f') {
				if (argc>=3) {
					strcpy(mailfile, argv[2]);
					argv++;
					argc--;
				}
			} else if (argv[1][1]=='r') {
				forward = 1;
			} else {
				fprintf(stderr, "mail: unknown option %c\n", argv[1][1]);
				done();
			}
		} else
			break;
	}
	malf = fopen(mailfile, "r");
	if (malf == NULL) {
		fprintf(stdout, "No mail.\n");
		return;
	}
	lock(mailfile);
	copymt(malf, tmpf);
	fclose(malf);
	fclose(tmpf);
	unlock();
	tmpf = fopen(lettmp, "r");

	changed = 0;
	print = 1;
	for (i = 0; i < nlet; ) {
		j = forward ? i : nlet - i - 1;
		if(setjmp(sjbuf)) {
			print=0;
		} else {
			if (print)
				copylet(j, stdout, ORDINARY);
			print = 1;
		}
		if (flgp) {
			i++;
			continue;
		}
		setjmp(sjbuf);
		fprintf(stdout, "? ");
		fflush(stdout);
		if (fgets(resp, LSIZE, stdin) == NULL)
			break;
		switch (resp[0]) {

		default:
			fprintf(stderr, "usage\n");
		case '?':
			print = 0;
			fprintf(stderr, "q\tquit\n");
			fprintf(stderr, "x\texit without changing mail\n");
			fprintf(stderr, "p\tprint\n");
			fprintf(stderr, "s[file]\tsave (default mbox)\n");
			fprintf(stderr, "w[file]\tsame without header\n");
			fprintf(stderr, "-\tprint previous\n");
			fprintf(stderr, "d\tdelete\n");
			fprintf(stderr, "+\tnext (no delete)\n");
			fprintf(stderr, "m user\tmail to user\n");
			fprintf(stderr, "! cmd\texecute cmd\n");
			break;

		case '+':
		case 'n':
		case '\n':
			i++;
			break;
		case 'x':
			changed = 0;
		case 'q':
			goto donep;
		case 'p':
			break;
		case '^':
		case '-':
			if (--i < 0)
				i = 0;
			break;
		case 'y':
		case 'w':
		case 's':
			flg = 0;
			if (resp[1] != '\n' && resp[1] != ' ') {
				printf("illegal\n");
				flg++;
				print = 0;
				continue;
			}
			if (resp[1] == '\n' || resp[1] == '\0')
				cat(resp+1, "mbox", "");
			for (p = resp+1; (p = getarg(lfil, p)) != NULL; ) {
				malf = fopen(lfil, "a");
				if (malf == NULL) {
					fprintf(stdout, "mail: cannot append to %s\n", lfil);
					flg++;
					continue;
				}
				copylet(j, malf, resp[0]=='w'? ZAP: ORDINARY);
				fclose(malf);
			}
			if (flg)
				print = 0;
			else {
				let[j].change = 'd';
				changed++;
				i++;
			}
			break;
		case 'm':
			flg = 0;
			if (resp[1] == '\n' || resp[1] == '\0') {
				i++;
				continue;
			}
			if (resp[1] != ' ') {
				printf("invalid command\n");
				flg++;
				print = 0;
				continue;
			}
			for (p = resp+1; (p = getarg(lfil, p)) != NULL; )
				if (!sendrmt(j, lfil))	/* couldn't send it */
					flg++;
			if (flg)
				print = 0;
			else {
				let[j].change = 'd';
				changed++;
				i++;
			}
			break;
		case '!':
			system(resp+1);
			printf("!\n");
			print = 0;
			break;
		case 'd':
			let[j].change = 'd';
			changed++;
			i++;
			if (resp[1] == 'q')
				goto donep;
			break;
		}
	}
   donep:
	if (changed)
		copyback();
}

copyback()	/* copy temp or whatever back to /usr/spool/mail */
{
	register i, n, c;
	int new = 0;
	struct stat stbuf;

	signal(SIGINT, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	lock(mailfile);
	stat(mailfile, &stbuf);
	if (stbuf.st_size != let[nlet].adr) {	/* new mail has arrived */
		malf = fopen(mailfile, "r");
		if (malf == NULL) {
			fprintf(stdout, "mail: can't re-read %s\n", mailfile);
			done();
		}
		fseek(malf, let[nlet].adr, 0);
		fclose(tmpf);
		tmpf = fopen(lettmp, "a");
		fseek(tmpf, let[nlet].adr, 0);
		while ((c = fgetc(malf)) != EOF)
			fputc(c, tmpf);
		fclose(malf);
		fclose(tmpf);
		tmpf = fopen(lettmp, "r");
		let[++nlet].adr = stbuf.st_size;
		new = 1;
	}
	malf = fopen(mailfile, "w");
	if (malf == NULL) {
		fprintf(stderr, "mail:  can't rewrite %s\n", lfil);
		done();
	}
	n = 0;
	for (i = 0; i < nlet; i++)
		if (let[i].change != 'd') {
			copylet(i, malf, ORDINARY);
			n++;
		}
	fclose(malf);
	if (new)
		fprintf(stdout, "new mail arrived\n");
	unlock();
}

copymt(f1, f2)	/* copy mail (f1) to temp (f2) */
FILE *f1, *f2;
{
	long nextadr;

	nlet = nextadr = 0;
	let[0].adr = 0;
	while (fgets(line, LSIZE, f1) != NULL) {
		if (isfrom(line))
			let[nlet++].adr = nextadr;
		nextadr += strlen(line);
		fputs(line, f2);
	}
	let[nlet].adr = nextadr;	/* last plus 1 */
}

copylet(n, f, type) FILE *f;
{	int ch;
	long k;
	fseek(tmpf, let[n].adr, 0);
	k = let[n+1].adr - let[n].adr;
	while(k-- > 1L && (ch=fgetc(tmpf))!='\n')
		if(type!=ZAP) fputc(ch,f);
	if(type==REMOTE) {
		gethostname(hostname, sizeof (hostname));
		fprintf(f, " remote from %s\n", hostname);
	}
	else if (type==FORWARD)
		fprintf(f, forwmsg);
	else if(type==ORDINARY)
		fputc(ch,f);
	while(k-->1L)
		fputc(ch=fgetc(tmpf), f);
	if(type!=ZAP || ch!= '\n')
		fputc(fgetc(tmpf), f);
}

isfrom(lp)
register char *lp;
{
	register char *p;

	for (p = from; *p; )
		if (*lp++ != *p++)
			return(0);
	return(1);
}

sendmail(argc, argv)
char **argv;
{

	time(&iop);
	fprintf(tmpf, "%s%s %s", from, my_name, ctime(&iop));
	iop = ftell(tmpf);
	flgf = 1;
	while (fgets(line, LSIZE, stdin) != NULL) {
		if (line[0] == '.' && line[1] == '\n')
			break;
		if (isfrom(line))
			fputs(">", tmpf);
		fputs(line, tmpf);
		flgf = 0;
	}
	fputs("\n", tmpf);
	nlet = 1;
	let[0].adr = 0;
	let[1].adr = ftell(tmpf);
	fclose(tmpf);
	if (flgf)
		return;
	tmpf = fopen(lettmp, "r");
	if (tmpf == NULL) {
		fprintf(stderr, "mail:  cannot reopen %s for reading\n", lettmp);
		return;
	}
	while (--argc > 0)
		if (!send(0, *++argv))	/* couldn't send to him */
			error++;
	if (error && safefile(dead)) {
		setuid(getuid());
		malf = fopen(dead, "w");
		if (malf == NULL) {
			fprintf(stdout, "mail:  cannot open %s\n", dead);
			fclose(tmpf);
			return;
		}
		copylet(0, malf, ZAP);
		fclose(malf);
		fprintf(stdout, "Mail saved in %s\n", dead);
	}
	fclose(tmpf);
}

sendrmt(n, name)
char *name;
{
	FILE *rmf, *popen();
	register char *p;
	char rsys[64], cmd[64];
	register local, pid;
	int ret, sts;

	local = 0;
	if (*name=='!')
		name++;
	for(p=rsys; *name!='!'; *p++ = *name++)
		if (*name=='\0') {
			local++;
			break;
		}
	*p = '\0';
	if ((!local && *name=='\0') || (local && *rsys=='\0')) {
		fprintf(stdout, "null name\n");
		return(0);
	}
	if ((pid = fork()) == -1) {
		fprintf(stderr, "mail:  can't create proc for remote\n");
		return(0);
	}
	if (pid) {
		while ((ret = wait(&sts)) != pid) {
			if (ret == -1)
				return(0);
		}
		return(!sts);
	}
	setuid(getuid());
	if (local)
		sprintf(cmd, "mail %s", rsys);
	else {
		if (index(name+1, '!'))
			sprintf(cmd, "uux - %s!rmail \\(%s\\)", rsys, name+1);
		else
			sprintf(cmd, "uux - %s!rmail %s", rsys, name+1);
	}
	if ((rmf=popen(cmd, "w")) == NULL)
		exit(1);
	copylet(n, rmf, local?  FORWARD : REMOTE);
	exit(pclose(rmf) != 0);
}

send(n, name)	/* send letter n to name */
int n;
char *name;
{
	char file[50];
	register char *p;
	register mask;
	struct passwd *pw, *getpwnam();
	struct stat statb;
#ifdef MSGS
	FILE *pip;
	char line[256];
	int count;
#endif

	for (p = name; *p != '!' && *p != '^' && *p != '\0'; p++)
		;
	if (*p == '!' || *p == '^')
		return(sendrmt(n, name));
#ifdef MSGS
/*
 * modification to call Berkeley 'msgs' program when the 'recipient'
 * is "msgs".  Assumption is that 'n' is 0, no REMOTE, FORWARD, etc.
 * pipe the letter to "msgs -s"  -- pag 11/10/79
 */
	if(!strcmp(name,"msgs"))	/* this letter is really a mesg */
	{
	    pip = popen("/usr/ucb/msgs -s", "w");
	    if (pip == NULL)
		return(0);
	    fseek(tmpf,0L,0);
	    count = 0;
	    while(fgets(line,256,tmpf) != NULL)
	    {
		count++;
		if(count == 2)
		{
		    fprintf(pip,"To: msgs\n");
 /*
  * check 2nd line for a Berkeley 'Mail'-added "To:" line....
  * we don't want to add 2 "To:"'s
  */
		    if(!strncmp(line,"To: msgs",8))
			continue;
		}
		fputs(line,pip);
	    }
	    pclose(pip);
	    return(1);
	}
#endif
	if ((pw = getpwnam(name)) == NULL) {
		fprintf(stdout, "mail:  can't send to %s\n", name);
		return(0);
	}
	cat(file, maildir, name);
	if (stat(file, &statb) >= 0 && (statb.st_mode & S_IFMT) == S_IFDIR) {
		strcat(file, "/");
		strcat(file, name);
	}
	mask = umask(MAILMODE);
	if (!safefile(file))
		return(0);
	lock(file);
	malf = fopen(file, "a");
	umask(mask);
	if (malf == NULL) {
		unlock();
		fprintf(stdout, "mail:  cannot append to %s\n", file);
		return(0);
	}
	chown(file, pw->pw_uid, pw->pw_gid);
	copylet(n, malf, ORDINARY);
	fclose(malf);
	unlock();
	return(1);
}

delete(i)
{
	setsig(i, delete);
	fprintf(stderr, "\n");
	if(delflg)
		longjmp(sjbuf, 1);
	done();
}

done()
{
	if(!lockerror)
		unlock();
	unlink(lettmp);
	exit(error+lockerror);
}

lock(file)
char *file;
{
	struct stat stbuf;

	if (locked || flgf)
		return;
	if (stat(file, &stbuf)<0)
		return;
	if (stbuf.st_mode&01) { 	/* user x bit is the lock */
		if (stbuf.st_ctime+60 >= time((long *)0)) {
			fprintf(stderr, "%s busy; try again in a minute\n", file);
			lockerror++;
			done();
		}
	}
	locked = stbuf.st_mode & ~01;
	curlock = file;
	chmod(file, stbuf.st_mode|01);
}

unlock()
{
	if (locked)
		chmod(curlock, locked);
	locked = 0;
}

cat(to, from1, from2)
char *to, *from1, *from2;
{
	int i, j;

	j = 0;
	for (i=0; from1[i]; i++)
		to[j++] = from1[i];
	for (i=0; from2[i]; i++)
		to[j++] = from2[i];
	to[j] = 0;
}

char *getarg(s, p)	/* copy p... into s, update p */
register char *s, *p;
{
	while (*p == ' ' || *p == '\t')
		p++;
	if (*p == '\n' || *p == '\0')
		return(NULL);
	while (*p != ' ' && *p != '\t' && *p != '\n' && *p != '\0')
		*s++ = *p++;
	*s = '\0';
	return(p);
}

safefile(f)
	char *f;
{
	struct stat statb;

#ifdef	UCB_SYMLINKS
	if (lstat(f, &statb) < 0)
#else
	if (stat(f, &statb) < 0)
#endif
		return(1);
	if (statb.st_nlink != 1 || (statb.st_mode & S_IFMT) == S_IFLNK) {
		fprintf(stderr, "mail:  %s has more than one link or is a symbolic link\n", f);
		return(0);
	}
	return(1);
}
