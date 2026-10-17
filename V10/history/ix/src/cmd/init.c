#include <signal.h>
#include <sys/types.h>
#include <sys/label.h>
#include <sys/pex.h>
#include <utmp.h>
#include <setjmp.h>
#include <sys/reboot.h>
#include <sys/filio.h>
#include <sys/ttyio.h>

#define	LINSIZ	sizeof(wtmp.ut_line)
#define	TABSIZ	100
#define	ALL	p = &itab[0]; p < &itab[TABSIZ]; p++
#define SCPYN(a, b)	strncpy(a, b, sizeof(a))
#define SCMPN(a, b)	strncmp(a, b, sizeof(a))

char	shell[]	= "/bin/sh";
char	nosh[]	= "/etc/nosh";
char	getty[]	= "/etc/getty";
char	minus[]	= "-";
char	afsck[]	= "/etc/rc.nosh.auto";
char	noshrc[]= "/etc/rc.nosh";
char	ifile[]	= "/etc/ttys";
char	utmp[]	= "/etc/utmp";
char	wtmpf[]	= "/usr/adm/wtmp";
char	ctty[]	= "/dev/console";
char	dev[]	= "/dev/";
char	floor[] = "/etc/floor";
char	*noargs[] = { 0 };
extern	int	tty_ld;
struct utmp wtmp;
struct	tab
{
	char	line[LINSIZ];
	char	comn;
	char	xflag;
	int	pid;
} line, itab[TABSIZ];

int	fi;
int	mergflag;
char	tty[20];
jmp_buf	sjbuf, shutpass;

int	reset();
char	*strcpy(), *strcat();
long	lseek();

extern struct label labelbot, labeltop;
struct label labelfloor;

#define T_KING (T_LOG|T_NOCHK|T_UAREA|T_SETLIC|T_SETPRIV|T_EXTERN)

/* needs T_EXTERN for labeling ports */
/* needs T_NOCHK for writing /dev/console */

main(ac, av)
char **av;
{
	register int r11;		/* passed thru from boot */
	int howto, oldhowto;

	howto = r11;
	if(ac > 1) {
		if(av[1][0] == 'a')
			howto = 0;
		else if(av[1][0] == 's')
			howto = RB_SINGLE;
	}
	setjmp(sjbuf);
	signal(SIGTERM, reset);
	signal(SIGSTOP, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	for(;;) {
		oldhowto = howto;
		howto = RB_SINGLE;
		if (setjmp(shutpass) == 0)
			shutdown();
		if (oldhowto & RB_SINGLE)
			single();
		if (runcom(oldhowto) == 0) 
			continue;
		merge();
		multiple();
	}
}

int	shutreset();

shutdown()
{
	register i;
	register struct tab *p;

	close(creat(utmp, 0644));
	signal(SIGHUP, SIG_IGN);
	for(ALL) {
		term(p);
		p->line[0] = 0;
	}
	signal(SIGALRM, shutreset);
	alarm(30);
	for(i=0; i<5; i++)
		kill(-1, SIGKILL);
	while(wait((int *)0) != -1)
		;
	alarm(0);
	shutend();
}

char shutfailm[] = "WARNING: Something is hung (wont die); ps axl advised\n";

shutreset()
{

	if (fork() == 0) {
		int ct = open(ctty, 1);
		write(ct, shutfailm, sizeof (shutfailm));
		sleep(5);
		exit(1);
	}
	sleep(5);
	shutend();
	longjmp(shutpass, 1);
}

shutend()
{
	register i;

	signal(SIGALRM, SIG_DFL);
	for(i=0; i<10; i++)
		close(i);
}

single()
{
	register pid;

	pid = fork();
	if(pid == 0) {
		signal(SIGTERM, SIG_DFL);
		signal(SIGHUP, SIG_DFL);
		signal(SIGALRM, SIG_DFL);
		setupio(ctty);
		setuplab(&labelbot, &labeltop, T_NOCHK, T_KING);
		execve(nosh, noargs, noargs);
		write(1, "no nosh, using sh\n", 18);
		execve(shell, noargs, noargs);
		write(1, "exec failed\n", 12);
		exit(0);
	}
	while(wait((int *)0) != pid)
		;
}

noshit(file)
char *file;
{
	register pid;
	int status;

	pid = fork();
	if(pid == 0) {
		setupio(ctty);
		setuplab(&labelbot, &labelfloor, T_NOCHK, T_KING);
		close(0);
		open("/dev/null", 0);
		execl(nosh, nosh, file, (char *)0);
		exit(1);
	}
	while(wait(&status) != pid)
		;
	return status;
}

runcom(oldhowto)
int oldhowto;
{
	register f;

	if((oldhowto&RB_SINGLE) == 0 && noshit(afsck) )
			return (0);
	if(noshit( noshrc ))
		return(0);
	f = open(wtmpf, 1);
	if (f >= 0) {
		lseek(f, 0L, 2);
		SCPYN(wtmp.ut_line, "~");
		SCPYN(wtmp.ut_name, "reboot");
		time(&wtmp.ut_time);
		write(f, (char *)&wtmp, sizeof(wtmp));
		close(f);
	}
	return(1);
}

setmerge()
{

	signal(SIGHUP, setmerge);
	mergflag = 1;
}

multiple()
{
	register struct tab *p;
	register pid;

loop:
	mergflag = 0;
	signal(SIGHUP, setmerge);
	for(;;) {
		pid = wait((int *)0);
		if(mergflag) {
			merge();
			goto loop;
		}
		if(pid == -1)
			return;
		for(ALL)
			if(p->pid == pid || p->pid == -1) {
				rmut(p);
				dfork(p);
			}
	}
}

term(p)
register struct tab *p;
{

	if(p->pid != 0) {
		rmut(p);
		kill(p->pid, SIGKILL);
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
	line.xflag = c;
	c = get();
	if(c <= 0)
		goto loop;
	line.comn = c;
	SCPYN(line.line, "");
	for (i=0; i<LINSIZ; i++) {
		c = get();
		if(c <= 0)
			break;
		line.line[i] = c;
	}
	while(c > 0)
		c = get();
	if(line.line[0] == 0)
		goto loop;
	if(line.xflag == '0')
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
	}
}

/*
 * NOCHK gets used by login to update utmp entry
 */
dfork(p)
struct tab *p;
{
	register pid;

	pid = fork();
	if(pid == 0) {
		signal(SIGTERM, SIG_DFL);
		signal(SIGHUP, SIG_DFL);
		strcpy(tty, dev);
		strncat(tty, p->line, LINSIZ);
		chown(tty, 0, 0);
		chmod(tty, 0622);
		setupio(tty);
		setuplab(&labelfloor, &labelfloor, 0, T_NOCHK);
		tty[0] = p->comn;
		tty[1] = 0;
		execl(getty, minus, tty, (char *)0);
		exit(0);
	}
	p->pid = pid;
}

rmut(p)
register struct tab *p;
{
	register f;

	f = open(utmp, 2);
	if(f >= 0) {
		while(read(f, (char *)&wtmp, sizeof(wtmp)) == sizeof(wtmp)) {
			if (SCMPN(wtmp.ut_line, p->line))
				continue;
			lseek(f, -(long)sizeof(wtmp), 1);
			SCPYN(wtmp.ut_name, "");
			time(&wtmp.ut_time);
			write(f, (char *)&wtmp, sizeof(wtmp));
		}
		close(f);
	}
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

reset()
{
	longjmp(sjbuf, 1);
}

setupio(tty)
char *tty;
{
	while (open(tty, 2) != 0)
		sleep(10);
	if(strcmp(tty, ctty)==0)
		ioctl(0, FIOAPX, (char *)0);
	setstsrc(0, tty);
	ioctl(0, TIOCSPGRP, (char *)0);
	while (ioctl(0, FIOPOPLD, (char *)0) >= 0)
		;
	ioctl(0, FIOPUSHLD, &tty_ld);
	dup(0);
	dup(0);
	dup(0);
}

setuplab(lp, lc, t, u)
struct label *lp, *lc;
{
	struct label lab;

	if(getflab(floor, &labelfloor)==-1)
		labelfloor = labelbot;
	labelfloor.lb_priv = labelbot.lb_priv;

	lab = *lp;
	lab.lb_t = 0;
	lab.lb_u = 0;

	fsetflab(1, &lab);
	lab = *lp;
	lab.lb_t = t;
	lab.lb_u = u;
	setplab(&lab, lc);
}
