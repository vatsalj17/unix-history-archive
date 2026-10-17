/*
 * Unix/v7m initialization program - init.c
 *
 * This version of init has been modified for
 * local/remote terminal operation and to reread
 * the ttys file when a hangup signal is received.
 * This allows local terminals to operate without
 * a null modem and for terminals to be placed
 * on-line and taken off-line without rebooting the
 * system.
 *
 * Fred Canter 9/25/81
 *
 * Thanks to Bill Shannon for most of the code changes.
 */
#include <signal.h>
#include <sys/types.h>
#include <utmp.h>
#include <setjmp.h>

#define LINSIZ	sizeof(wtmp.ut_line)
#define	TABSIZ	100
#define	ALL	p = &itab[0]; p < &itab[TABSIZ]; p++
#define	EVER	;;
#define SCPYN(a, b)	strncpy(a, b, sizeof(a))
#define SCMPN(a, b)	strncmp(a, b, sizeof(a))

char	shell[]	= "/bin/sh";
char	getty[]	 = "/etc/getty";
char	minus[]	= "-";
char	runc[]	= "/etc/rc";
char	ifile[]	= "/etc/ttys";
char	utmp[]	= "/etc/utmp";
char	wtmpf[]	= "/usr/adm/wtmp";
char	ctty[]	= "/dev/console";
char	dev[]	= "/dev/";

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

int	fi;
int	mergflag;
char	tty[20];
jmp_buf	sjbuf;

main()
{
	int reset();

	setjmp(sjbuf);
	signal(SIGINT, reset);
	for(EVER) {
		shutdown();
		single();
		runcom();
		merge();
		signal(SIGINT, reset);
		multiple();
	}
}

shutdown()
{
	register i;
	register struct tab *p;

	close(creat(utmp, 0644));
	signal(SIGINT, SIG_IGN);
	for(ALL)
		term(p);
	signal(SIGALRM, reset);
	alarm(60);
	for(i=0; i<5; i++)
		kill(-1, SIGKILL);
	while(wait((int *)0) != -1)
		;
	alarm(0);
	signal(SIGALRM, SIG_DFL);
	for(i=0; i<10; i++)
		close(i);
}

single()
{
	register pid;

	pid = fork();
	if(pid == 0) {
/*
		alarm(300);
*/
		signal(SIGHUP, SIG_DFL);
		signal(SIGINT, SIG_DFL);
		signal(SIGALRM, SIG_DFL);
		open(ctty, 2);
		dup(0);
		dup(0);
		execl(shell, minus, (char *)0);
		exit(0);
	}
	sync();
	while(wait((int *)0) != pid)
		;
}

runcom()
{
	register pid, f;

	pid = fork();
	if(pid == 0) {
		open("/", 0);
		dup(0);
		dup(0);
		execl(shell, shell, runc, (char *)0);
		exit(0);
	}
	while(wait((int *)0) != pid)
		;
	f = open(wtmpf, 1);
	if (f >= 0)
	{
		lseek(f,0L,2);
		SCPYN(wtmp.ut_line, "~");
		SCPYN(wtmp.ut_name, "");
		time(&wtmp.ut_time);
		write(f, (char *)&wtmp, sizeof(wtmp));
		close(f);
	}
}
setmerge()
{

	mergflag = 1;
}


multiple()
{
	register struct tab *p;
	register pid;

loop:
	mergflag = 0;
	signal(SIGHUP, setmerge);
	for(EVER) {
		pid = wait((int *)0);
		if(mergflag)
		{
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
/*
	p->line[0] = 0;
*/
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
	for(i=0; i<LINSIZ; i++)
	{
		c = get();
		if(c <= 0)
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
	i = atoi(&line.line[3]);	/* get tty line number */
	ttlocl(i, (line.flag >> 1));	/* set local or remote */
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

#define FOUND	1
#define CHANGE	2

merge()
{
	register struct tab *p;
	register i;

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
	for(ALL)
	{
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

dfork(p)
struct tab *p;
{
	register pid;

	pid = fork();
	if(pid == 0) {
		signal(SIGHUP, SIG_DFL);
		signal(SIGINT, SIG_DFL);
		strcpy(tty, dev);
		strncat(tty, p->line, LINSIZ);
		chown(tty, 0, 0);
		chmod(tty, 0622);
		open(tty, 2);
		dup(0);
		dup(0);
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
