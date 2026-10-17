/*
 *	init.c
 */
#include "whoami.h"
#include <signal.h>
#include <sys/types.h>
#include <utmp.h>
#include <setjmp.h>
#ifdef	UCB_AUTOBOOT
#include <sys/reboot.h>
#endif
#include <errno.h>
#include <sys/autoconfig.h>

#define	VHANGUP			/* undefine if you don't have vhangup */
#define	CONFIGOPTS	"-vc"

#define	LINSIZ	sizeof(wtmp.ut_line)
#define	TABSIZ	100
#define	ALL	p = &itab[0]; p < &itab[TABSIZ]; p++
#define	EVER	;;
#define SCPYN(a, b)	strncpy(a, b, sizeof(a))
#define SCMPN(a, b)	strncmp(a, b, sizeof(a))

char	shell[]	= "/bin/sh";
char	getty[]	 = "/etc/getty";
char	minus[]	= "-";
char	plus[] = "+";		/* this tells getty we are in sp. sess. */
char	runc[]	= "/etc/rc";
char	ifile[]	= "/etc/ttys";
char	utmp[]	= "/etc/utmp";
char	wtmpf[]	= "/usr/adm/wtmp";
char	ctty[]	= "/dev/console";
char	dev[]	= "/dev/";
char	config[]= "/etc/autoconfig";

struct utmp wtmp;
struct
{
	char	line[LINSIZ];
	char	comn;
	char	flag;
} line;
struct	tab
{
	char	line[LINSIZ];
	char	comn;
	char	xflag;
	int	pid;
} itab[TABSIZ];

extern	errno;

int	fi;
int	mergflag;
int	multiuser;
/*
 *  Modes: bits in first char of /etc/ttys line
 */
#define	NORMAL	1		/* anyone can login */
#define	SP_SESS	2		/* only root can login */
int	mode = NORMAL;

char	tty[20];
jmp_buf	sjbuf, shutpass;
time_t	time0;

time_t	time();
int	reset();
int	idle(), sp_ss(), setmerge();
char	*strcpy(), *strcat();
long	lseek();

#ifdef	UCB_AUTOBOOT
main(ac, av)
char	**av;
#else
main ()
#endif
{
	int howto, oldhowto;

	time0 = time((time_t *) 0);
#ifdef	UCB_AUTOBOOT
	signal(SIGQUIT, idle);

	if (ac > 1) {
		howto = * ((int *) av[1]);
		* ((int *) av[1]) = 0;	/* don't confuse ps with binary args */
	 } else
		howto = RB_SINGLE;
	if (autoconfig() == 0)
		howto = RB_SINGLE;
#else
	autoconfig();
#endif
	setjmp(sjbuf);
	signal(SIGTERM, reset);
	signal(SIGINT, sp_ss);
	signal(SIGHUP, setmerge);
	for(EVER) {
		shutdown();
#ifdef	UCB_AUTOBOOT
		oldhowto = howto;
		howto = RB_SINGLE;
		if (oldhowto & RB_SINGLE)
			single();
		if (runcom(oldhowto) == 0) 
			mode = SP_SESS;
#else
		single();
		runcom();
#endif
		merge();
		multiple();
	}
}

int	shutreset();

shutdown()
{
	register i, f;
	register struct tab *p;

	multiuser = 0;
	for(ALL) {
		term(p);
		p->line[0] = 0;
	}
	close(creat(utmp, 0644));
	signal(SIGALRM, shutreset);
	if (setjmp(shutpass) == 0) {
		alarm(30);
		for(i=0; i<5; i++)
			kill(-1, SIGKILL);
		while(wait((int *)0) != -1)
			;
		alarm(0);
	}
	acct(0);
	signal(SIGALRM, SIG_DFL);
	for(i=0; i<10; i++)
		close(i);
	f = open(wtmpf, 1);
	if (f >= 0) {
		lseek(f, 0L, 2);
		SCPYN(wtmp.ut_line, "~");
		SCPYN(wtmp.ut_name, "shutdown");
		time(&wtmp.ut_time);
		write(f, (char *)&wtmp, sizeof(wtmp));
		close(f);
	}
}

shutreset()
{
	cmesg("WARNING: Something is hung (won't die); ps axl advised\n", 0, 0);
	longjmp(shutpass, 1);
}

single()
{
	register pid;
	register xpid;
	extern	errno;

	multiuser = 0;
   do {
	pid = fork();
	if(pid == 0) {
/*
		alarm(300);
*/
		signal(SIGTERM, SIG_DFL);
		signal(SIGHUP, SIG_DFL);
		signal(SIGALRM, SIG_DFL);
		open(ctty, 2);
		dup(0);
		dup(0);
		execl(shell, minus, (char *)0);
		cmesg("Init: can't exec ", shell, "\r\n");
		exit(0);
	}
	while((xpid = wait((int *)0)) != pid)
		if (xpid == -1 && errno == ECHILD)
			break;
   } while (xpid == -1);
}

#ifdef	UCB_AUTOBOOT
runcom(howto)
int howto;
#else
runcom()
#endif
{
	register pid, f;
	int status;
	char *arg1, *arg2;

	pid = fork();
	if(pid == 0) {
		open("/", 0);
		dup(0);
		dup(0);
#ifdef	UCB_AUTOBOOT
		if ((howto & RB_SINGLE) || (howto & RB_NOFSCK))
			arg1 = "fastboot";
		else
			arg1 = "autoboot";
		if (howto & RB_POWRFAIL)
			arg2 = "powerfail";
		else
			arg2 = (char *)0;
		execl(shell, shell, runc, arg1, arg2, (char *)0);
		exit(1);
#else
		execl(shell, shell, runc, (char *)0);
		exit(1);
#endif
	}
	while(wait(&status) != pid)
		;
#ifdef	UCB_AUTOBOOT
	if(status)
		return(0);
#endif
	f = open(wtmpf, 1);
	if (f >= 0) {
		lseek(f, 0L, 2);
		SCPYN(wtmp.ut_line, "~");
		SCPYN(wtmp.ut_name, "reboot");
		if (time0) {
			wtmp.ut_time = time0;
			time0 = 0;
		} else
			time(&wtmp.ut_time);
		write(f, (char *)&wtmp, sizeof(wtmp));
		close(f);
	}
	return(1);
}

setmerge()
{
	signal(SIGHUP, SIG_IGN);
	mergflag = 1;
}

multiple()
{
	register struct tab *p;
	register pid;

loop:
	multiuser = 1;
	mergflag = 0;
	signal(SIGHUP, setmerge);
	for(EVER) {
		pid = wait((int *)0);
		if(mergflag) {
			merge();
			goto loop;
		}
		if(pid == -1) {
			if (errno == ECHILD) {
				cmesg("Init: ", "no children left", "\r\n");
				return;
			}
			goto loop;
		}
		for(ALL)
			if(p->pid == pid || p->pid == -1) {
#ifdef	UCB_SUBMIT
				if (p->pid != -1)
					killbkg(p->pid, SIGKILL);
#endif
				rmut(p);
				dfork(p);
			}
	}
}

term(p)
register struct tab *p;
{

	if(p->pid != 0 && p->pid != -1) {
		rmut(p);
		kill(p->pid, SIGKILL);
#ifdef	UCB_SUBMIT
		killbkg(p->pid, SIGKILL);
#endif
	}
	p->pid = 0;
}

rline()
{
	register c, i;

loop:
	c = get();
	if(c < 0)
		return(0);
	if(c == 0)
		goto loop;
	line.flag = c;
	c = get();
	if(c <= 0)
		goto loop;
	line.comn = c;
	SCPYN(line.line, "");
	for (i=0; i<LINSIZ; i++) {
		c = get();
		/*
		 * If a blank, newline, or end of file, or tab,
		 * cease accumulating line name
		 */
		if(c <= 0 || c == ' ' || c == '\t')
			break;
		line.line[i] = c;
	}
	while(c > 0)
		c = get();
	if(line.line[0] == 0)
		goto loop;
	if(line.flag == '0')
		goto loop;
	strcpy(tty, dev);
	strncat(tty, line.line, LINSIZ);
	if(access(tty, 06) < 0)
		goto loop;
	return(1);
}

get()
{
	char b;

	if(read(fi, &b, 1) != 1)
		return(-1);
	if(b == '\n')
		return(0);
	return(b);
}

#define	FOUND	1
#define	CHANGE	2

merge()
{
	register struct tab *p;

	fi = open(ifile, 0);
	if(fi < 0)
		return;
	for(ALL)
		p->xflag = 0;
	while(rline()) {
		if ((line.flag < '1') || (line.flag > '9'))
			continue;
		if (((line.flag-'0') & mode) == 0)
			continue;
		for(ALL) {
			if (SCMPN(p->line, line.line))
				continue;
			p->xflag |= FOUND;
			if(line.comn != p->comn) {
				p->xflag |= CHANGE;
				p->comn = line.comn;
			}
			goto contin1;
		}
		for(ALL) {
			if(p->line[0] != 0)
				continue;
			SCPYN(p->line, line.line);
			p->xflag |= FOUND|CHANGE;
			p->comn = line.comn;
			goto contin1;
		}
	contin1:
		;
	}
	close(fi);
	for(ALL) {
		if((p->xflag&FOUND) == 0) {
			term(p);
			p->line[0] = 0;
		}
		if((p->xflag&CHANGE) != 0) {
			term(p);
			dfork(p);
		}
#ifdef	UCB_AUTOBOOT
		/*
		 * If we are resuming after an idle (possibly a failed reboot)
		 * we need to restart the lines that shut down.
		 */
		if (p->pid == -1)
			dfork(p);
#endif
	}
}

dfork(p)
struct tab *p;
{
	register pid;

	pid = fork();
	if(pid == 0) {
		signal(SIGTERM, SIG_DFL);
		signal(SIGHUP, SIG_IGN);
		strcpy(tty, dev);
		strncat(tty, p->line, LINSIZ);
		chown(tty, 0, 0);
		chmod(tty, 0622);
		if (open(tty, 2) < 0) {
			int repcnt = 0;
			do {
				if (repcnt % 10 == 0)
					cmesg("init: ",tty,": cannot open\n\r");
				repcnt++;
				sleep(60);
			} while (open(tty, 2) < 0);
		}
#ifdef	VHANGUP
		vhangup();
#endif
		signal(SIGHUP, SIG_DFL);
		open(tty, 2);
		close(0);
		dup(1);
		dup(0);
		tty[0] = p->comn;
		tty[1] = 0;
		if(mode == SP_SESS)
			execl(getty, plus, tty, (char *)0);
		else
			execl(getty, minus, tty, (char *)0);
		exit(0);
	}
	p->pid = pid;
}

rmut(p)
register struct tab *p;
{
	register f;
	int found = 0;

	f = open(utmp, 2);
	if(f >= 0) {
		while(read(f, (char *)&wtmp, sizeof(wtmp)) == sizeof(wtmp)) {
			if (SCMPN(wtmp.ut_line, p->line) || wtmp.ut_name[0]==0)
				continue;
			lseek(f, -(long)sizeof(wtmp), 1);
			SCPYN(wtmp.ut_name, "");
			time(&wtmp.ut_time);
			write(f, (char *)&wtmp, sizeof(wtmp));
			found++;
		}
		close(f);
	}
	if (found) {
		f = open(wtmpf, 1);
		if (f >= 0) {
			SCPYN(wtmp.ut_line, p->line);
			SCPYN(wtmp.ut_name, "");
			time(&wtmp.ut_time);
			lseek(f, (long)0, 2);
			write(f, (char *)&wtmp, sizeof(wtmp));
			close(f);
		}
	}
}

reset()
{
	longjmp(sjbuf, 1);
}

/*
 *  Toggle special-session mode.
 *  Do a shutdown() so that getty's are reissued in the new mode.
 */
sp_ss()
{
	signal(SIGINT, SIG_IGN);
	mergflag++;
	shutdown();
	if (mode == NORMAL) 
		mode = SP_SESS;
	else {
		/*
		 * Returning to normal operation.
		 * Run the rc file; either it hasn't been finished
		 * since we failed a filesystem check, or we shut down.
		 */
#ifdef	UCB_AUTOBOOT
		(void) runcom(RB_NOFSCK);
#else
		(void) runcom();
#endif
		mode = NORMAL;
	}
	signal(SIGINT, sp_ss);
}

#ifdef	UCB_AUTOBOOT
idle()
{
	register struct tab *p;
	register pid;

	signal(SIGQUIT,idle);
	for (;;) {
		pid = wait((int *) 0);
		if (mergflag) {
			if (!multiuser)
				reset();
			else
				return;
		}
		if (pid == -1)
			pause();
		else {
			for (ALL)
				if (p->pid == pid) {
					rmut(p);
					p->pid = -1;
				}
		}
	}
}
#endif

cmesg(s1, s2, s3)
char *s1, *s2, *s3;
{
	register int pid;

	pid = fork();
	if (pid == 0) {
		int fd = open(ctty, 2);
		write(fd, s1, strlen(s1));
		if (s2)
			write(fd, s2, strlen(s2));
		if (s3)
			write(fd, s3, strlen(s3));
		close(fd);
		exit(0);
	}
	while (wait((int *)0) != pid)
		;
}

autoconfig()
{
	int pid, status;

	cmesg("\r\nCONFIGURE SYSTEM:\n", 0, 0);
	if ((pid = fork()) == 0) {
		open(ctty, 2);
		dup(0);
		dup(0);
		execl(config, "autoconfig", CONFIGOPTS, 0);
		printf("Couldn't exec %s\n", config);
		exit(AC_SETUP);
	}
	while (wait(&status) != pid)
		;
	if ((status & 0377) == 0)
		status >>= 8;
	else
		status = AC_SINGLE;
	switch (status) {
		case AC_SETUP:
			cmesg("Configuration setup error\n", 0, 0);
			return 0;
		case AC_SINGLE:
			cmesg("SERIOUS CONFIGURATION ERROR\n", 0, 0);
			return 0;
		case AC_OK:
			return 1;
		default:
			cmesg("Unrecognized return from configure\n", 0, 0);
			return 0;
	}
}
