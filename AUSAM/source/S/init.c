#include	<local-system>

#define	TALK
/*
 *	ian johnstone 	july 76
 *
 *	allow	"/etc/rc" to  have tty8 as fd 0 1 2
 *	useful for communicating etc during startup
 */ 

/* (not	SINGLE_USER)
#define	SINGLE_USER
/*
 *	chris maltby		'75
 *
 *	for single user invoke getty - must still know the passwd
 */ 

#define	SHUTDOWN
/*
 *	ian johnstone 		aug 77
 *
 *	on receiving a signal 14 init will not restart
 *	terminating shells.  The editor on receiving a signal 14
 *	will write current temp file to	"saved.file".
 *	see shtdwn() for more details.
 */ 

#define	AUSAM
/*
 *	Piers Lauder	oct '77
 *
 *	write more comprehensive info. into wtmp for
 *	each process collected here. Uses new wait for proc 1
 *	which returns process "zombie" & "limits" structures.
 *
 *	Piers Lauder	Feb '78 - May '78
 *
 *	The whole AUSAM works ...
 */ 

#define	SYS_SUP_PROCS
/*
 *	Piers Lauder	May '78
 *
 *	start up known system support processes
 *	i.e.: update, cron ...
 */ 

/* (not	DEBUG)
#define	DEBUG
/*
 *	Piers Lauder	Apr '78
 *
 *	Use "prs" to complain about problems ...
 */ 

/* (not	TEST)
#define	TEST
/*
 *	give alternate names for programs in test mode
 */ 

/* (not	TRACE)
#define	TRACE
/*
 **	trace bugs (needs DEBUG)
 */ 

#define	TTY_GROUP
/*
 **	Ian Johnstone	Sep '78
 **
 **	Each line in /etc/ttys is at least 4 chars long.
 **	The fourth char designates the terminal group to
 **	which a terminal belongs. Needs getty to have
 **	TTY_GROUP defined as well for full control
 **	to be exercised.
 */ 



#ifdef	AUSAM

#define	FORKTRYS 2	/* maximum fork attempts to be made */

#include	<wtmp.h>
struct stmp wtmp;

#include	<defines.h>
#include	<param.h>
#include	<passwd.h>
#include	<lnode.h>
#include	<stat16.h>
struct pwent pe;

#include	<pzomb.h>
struct corpse
{
	struct pzomb ppzomb;
	struct lnode limits;
} waitbuf;

char	homed[]		"/";
char	tmpd[]		"/tmp";
char	rmtree[]	"/etc/rmtree";
#define	rmtree_args	rmtree+5, "-f"
char	chkptf[]	"/etc/lnodes.chkpt";

int resetflg;

#endif	AUSAM

#ifdef	SYS_SUP_PROCS
/*
 *	system support processes
 */ 
char *sysprocs[]
{
	"/etc/update",
	"/etc/cron",
	0
};
#endif	SYS_SUP_PROCS

#define	SYSPRI	-90	/* run system processes at high priority */
#define	USERPRI	0	/* but not users */

#define	tabsize	NTTYS
#define	all	p = &itab[0]; p < &itab[tabsize]; p++
#define	ever	;;
#define	SINGLE	0173030
#define	REBOOT	0173040

long time();

char	shell[]	"/bin/sh";
char	getty[]	"/etc/getty";
char	runc[]	"/etc/rc";
char	init[]	"/etc/init";
char	ifile[]	"/etc/ttys";
char	utmpf[]	"/etc/utmp";
char	wtmpf[]	"/usr/adm/wtmp";
char	ctty[]	"/dev/tty8";

#ifdef	SINGLE_USER | AUSAM
#ifdef	BASSER
#define	CTTYS	"l"	/* console type for getty */
#define	CTTYC	'l'	/* console type for getty */
#else	BASSER
#define	CTTYS	"d"	/* console type for getty */
#define	CTTYC	'd'	/* console type for getty */
#endif	BASSER
#endif	SINGLE_USER | AUSAM

#ifndef	SINGLE_USER
char	minus[]	"-";
#endif

int fi;
struct
{
	int flag;
	char lins[2];
	char coms[2];
#	ifdef	TTY_GROUP
	char tgroup[2];	/* terminal group */ 
#	endif	TTY_GROUP
} line;

struct tab
{
	int pid;
	int line;
	int comn;
#	ifdef	TTY_GROUP
	int tgp;
#	endif	TTY_GROUP
} itab[tabsize];

#ifndef	AUSAM
struct
{
	char name[8];
	char tty;
	char fill;
	long time;
	int wfill;
} wtmp;

int status;
#endif	AUSAM

/*
 * the main init:	run start-up file "rc"
 *			fork off "getty" for each valid tty
 *			loop, catching dead shells and restarting "getty"
 */ 
main()
{
	register i;
	register struct tab *p, *q;
#ifdef	SHUTDOWN
	int shtdwn();
#endif	SHUTDOWN
#ifdef	AUSAM
	extern char *initwait();
#else
	int reset();
#endif	AUSAM

	nice(SYSPRI);
#ifdef	AUSAM
Reboot:
	/*
	 * write system boot time to wtmp
	 */ 
	wtmp.s_type = S_TYPE;
	wtmp.s_boottime = time();
	wrwtmp();
#endif	AUSAM
	/*
	 * if not single user,
	 * run shell sequence
	 */ 
	if(getcsw() != SINGLE)
	{
#ifndef	AUSAM
		if((i = open(wtmpf, 1)) >= 0)
		{
			seek(i, 0, 2);
			wtmp.tty = '~';
			wtmp.time = time();
			write(i, &wtmp, (sizeof wtmp));
			close(i);
		}
#endif	AUSAM
#ifdef	AUSAML
		/*
		 * if system crashed - do checkpointing
		 */ 
		{
			struct statbuf statb;

			if(stat(chkptf, &statb) != -1 && statb.sb_size1)
			{
				struct lnode ln;

				i = open(chkptf, 0);
				while(read(i, &ln, (sizeof ln)) == (sizeof ln))
				{
					pe.pw_uid = ln.l_uid;
					if(getpwlog(&pe, 0, 0) >= 0)
					{
						pe.pw_usage = ln.l_usage;
						pe.pw_extime = wtmp.s_boottime;
						updtpwent(&pe);
					}
				}
				close(i);
				pwclose();
			}
		}
#endif	AUSAML
		/*
		 *	run shell start up file "rc".
#		ifdef	TALK
		 *
		 *  N.B. -- it should ask if you really
		 *	want to run it!
#		endif	TALK
		 */ 
		i = fork();
		if(i == 0)
		{
			nice(USERPRI);
#ifdef	TALK
			open(ctty, 2);
#else
			open("/", 0);
#endif	TALK
			dup(0);
			dup(1);
			execl(shell, shell+5, runc, 0);
#			ifdef	DEBUG
			prs("init: no ");
			prs(shell);
			prs("!\n");
#			endif	DEBUG
			exit(100);
		}
#ifndef	AUSAM
		while(waitx(&status) != i);
#else
		itab[0].pid = i;
		itab[0].line = ctty[8];
		while(initwait(1) != &itab[0]);
#endif	AUSAM
#ifdef	SYS_SUP_PROCS
		/*
		 * start up known system support processes
		 */ 
		{
			struct
			{
				char **cp;
			};

			for(p.cp = sysprocs; *p.cp; p.cp++)
			{
				switch(fork())
				{
				case 0:
					signal(2, 1);
					signal(3, 1);
					signal(14, 0);
					open(ctty, 2);
					dup(0);
					dup(1);
					execl(*p.cp, *p.cp+5, 0);
#					ifdef	DEBUG
						prs("init: no ");
						prs(*p.cp);
						prs("!\n");
#					endif
					exit(1);
				case -1:
#					ifdef	DEBUG
						prs("init: fork failed\n");
#					endif	DEBUG
					continue;
				default:
					sleep(1);
				}
			}
		}
#endif	SYS_SUP_PROCS
		close(creat(utmpf, 0604));	/* ensure utmp exists */ 
	}
	/*
	 * main loop for hangup signal
	 * close all files and
	 * check switches for magic values
	 */ 
#ifndef	AUSAM
	setexit();
	signal(1, reset);
#else
	signal(1, 1);
Reset:
	resetflg = 0;
#endif	AUSAM
#ifdef	SHUTDOWN
	if(getpid() == 1)
		signal(14, shtdwn);	/*  only process one (the real init) */ 
#endif	SHUTDOWN
	for(i = 0; i < 10; i++)
		close(i);
	switch(getcsw())
	{
	case SINGLE:
	error:
		termall();
#ifdef	!AUSAM | !SINGLE_USER
		i = fork();
		if(i == 0)
		{
			nice(USERPRI);
#ifdef	AUSAM
			signal(2, 1);
			signal(3, 1);
#endif	AUSAM
			open(ctty, 2);
			dup(0);
			dup(1);
#ifndef	SINGLE_USER
			execl(shell, minus, 0);
#else	SINGLE_USER
#			ifndef	TTY_GROUP
			execl(getty, getty+5, CTTYS, 0);
#			else	TTY_GROUP
			execl(getty, getty+5, CTTYS, p->tgroup, 0);
#			endif	TTY_GROUP
#endif	SINGLE_USER
			exit(100);
		}
#ifndef	AUSAM
		while(waitx(&status) != i);
#else
		itab[0].pid = i;
		itab[0].line = ctty[8];
		while(initwait(1) != &itab[0] && !resetflg);
		if(resetflg)
			goto Reset;
#endif	AUSAM
#else	!AUSAM | !SINGLE_USER
		itab[0].line = ctty[8];
		itab[0].comn = CTTYC;
		dfork(&itab[0]);
		while(initwait(1) != &itab[0] && !resetflg);
		if(resetflg)
			goto Reset;
#endif	!AUSAM | !SINGLE_USER

	case REBOOT:
		termall();
		execl(init, init+5, 0);
#ifndef	AUSAM
		reset();
#else
		goto Reboot;
#endif	AUSAM
	}
	/*
	 * open and merge in init file
	 */ 
	fi = open(ifile, 0);
	q = &itab[0];
	while(rline())
	{
		if(line.flag == '0')
			continue;
		for( all)
			if(p->line == line.line || p->line == 0)
			{
				if(p >= q)
				{
					i = p->pid;
					p->pid = q->pid;
					q->pid = i;
					p->line = q->line;
					p->comn = q->comn;
#					ifdef	TTY_GROUP
					p->tgp = q->tgp;
#					endif	TTY_GROUP
					q->line = line.line;
					q->comn = line.comn;
#					ifdef	TTY_GROUP
					q->tgp = line.tgp;
#					endif	TTY_GROUP
					q++;
				}
				break;
			}
	}
	close(fi);
	if(q == &itab[0])
		goto error;
	for(; q < &itab[tabsize]; q++)
		term(q);
	for( all)
		if(p->line != 0 && p->pid == 0)
			dfork(p);
	for(ever)
	{
#ifndef	AUSAM
		i = waitx(&status);
		for( all)
			if(p->pid == i)
			{
				rmut(p);
				dfork(p);
			}
#else	AUSAM
		initwait(0);
		if(resetflg)
			goto Reset;
#endif	AUSAM
	}
}

/*
 * terminate all logged in users
 */ 
termall()
{
	register struct tab *p;

	for( all)
		term(p);
}

/*
 * terminate known process started by init
 */ 
term(ap)
struct tab *ap;
{
	register struct tab *p;

	p = ap;
	if(p->pid != 0)
	{
#ifndef	AUSAM
		rmut(p);
#endif	AUSAM
		kill(p->pid, 9);
#ifndef	AUSAM
		p->pid = 0;
#else
		p->comn = 0;	/* flag for wait to clear pid & line */ 
#endif	AUSAM
	}
#ifdef	AUSAM
	else
#endif	AUSAM
		p->line = 0;
}

/*
 * read a line from "ttys" file
 *	char 0 = '0' or '1' - tty valid flag
 *	char 1 = '0'-'9','a'-'z','A'-'Z' - tty id
 *	char 2 = tty type for getty
#ifdef	TTY_GROUP
 *	char 3 = tty group
#endif	TTY_GROUP
 *	char ... anything you like
 */ 
rline()
{
	static char c[4];

#	ifndef	TTY_GROUP
	if(read(fi, c, 3) != 3)
#	else	TTY_GROUP
	if(read(fi, c, 4) != 4)
#	endif	TTY_GROUP
		return(0);
	line.flag = c[0];
	line.lins[0] = c[1];
	line.coms[0] = c[2];
#	ifdef	TTY_GROUP
	line.tgroup[0] = c[3];
#	endif	TTY_GROUP
	while(read(fi, c, 1) == 1 && c[0] != '\n');
	return(c[0] == '\n');
}

/*
 * fork, open a tty file, and exec getty
 */ 
dfork(ap)
struct tab *ap;
{
	register i;
	register char *tty;
	register struct tab *p;

	p = ap;
	i = fork();
	if(i == 0)
#	ifdef	AUSAM
		newgetty(p);
#	else	AUSAM
	{
		nice(USERPRI);
#		ifdef	SHUTDOWN
		signal(14, 0);
#		endif	SHUTDOWN
		tty = "/dev/ttyx";
		tty[8] = p->lins[0];
		chown(tty, 0);
		chmod(tty, 0600);
		open(tty, 2);
		dup(0);
		dup(1);
#		ifndef	TTY_GROUP
		execl(getty, getty+5, p->coms, p->lins, 0);
#		else	TTY_GROUP
		execl(getty, getty+5, p->coms, p->lins, p->tgroup, 0);
#		endif	TTY_GROUP
#		ifdef	DEBUG
		prs("init: no getty!\n");
#		endif
		sleep(10);	/* avoid thrashing */ 
		exit(100);
	}
#	endif	AUSAM
	if(i == -1)	/* fork failed */ 
		i = 0;
	p->pid = i;
}

#ifdef	AUSAM
/*
 *	set up tty and exec getty
 */ 
newgetty(p)
register struct tab *p;
{
	register char *tty;
#	ifdef	DEBUG
	register i;
#	endif	DEBUG

	nice(USERPRI);
	signal(1, 0);
	signal(2, 1);
	signal(3, 1);
#	ifdef	SHUTDOWN
	signal(14, 0);
#	endif	SHUTDOWN
	tty = "/dev/ttyx";
	tty[8] = p->lins[0];
	chown(tty, 0);
	chmod(tty, 0600);
#	ifdef	DEBUG
	if((i = open(tty, 2)) == 0)
#	else	DEBUG
	if(open(tty, 2) == 0)
#	endif	DEBUG
	{
		dup(0);
		dup(1);
#		ifndef	TTY_GROUP
		execl(getty, getty+5, p->coms, p->lins, 0);
#		else	TTY_GROUP
		execl(getty, getty+5, p->coms, p->lins, p->tgroup, 0);
#		endif	TTY_GROUP
#		ifdef	DEBUG
		prs("init: no getty!\n");
#		endif	DEBUG
	}
#	ifdef	DEBUG
	else if(i > 0)
		prs("init: tty not fd 0!\n");
	else
		prs("init: tty open error!\n");
#	endif	DEBUG
	sleep(10);	/* avoid thrashing */ 
	exit(100);
}

/*
 * initwait collects dead procs and updates various files;
 *
 *	there are three types of procs and the action taken is as follows:
 *	1. tty shells		-> update utmp
 *				-> update pwent with connect time etc.
 *	2. async procs		-> update pwent cpu times etc.
 *	3. last proc of a user	-> update pwent with extime etc.
 *				-> clean out /tmp for this user
 *
 *	all except async root processes cause termination entries in wtmp.
 *
 * N.B. the hangup signal is enabled during "iwait".
 */ 
char *initwait(nofork)
#else	AUSAM
/*
 * rmut removes utmp entry and writes logoff time to wtmp
 */ 
rmut(p)
register struct tab *p;
#endif	AUSAM
{
	register i, f;
	static char zero[(sizeof wtmp)];
#ifdef	AUSAM
	register struct tab *p;
	long ttime, time();
	int reset();
#	ifdef	TRACE
	char tbuf[8];
#	endif	TRACE

	waitbuf.limits.l_refcount = 0;
	waitbuf.limits.l_uid = 0;
	signal(1, reset);
	i = iwait(&waitbuf);
	signal(1, 1);
	if(i == -1)
	{
		if(resetflg)
			return(0);
		else
			return(i);
	}
	for( all)
		if(p->pid == i)	/* tty shell */ 
		{
			p->pid = 0;
			goto out;
		}
	p = 0;	/* async proc */ 
	if(waitbuf.limits.l_uid == 0)	/* ignore async root processes */ 
		return(p);
out:
	if(!nofork)
	{
		/*
		 **	fork off a proc to do accounting and start new getty
		 */ 
		f = 0;
		while(i = fork())
		{
			/** Parent -- return to main **/ 
			if(i == -1)	/* fork failed */ 
			{
				if(++f < FORKTRYS)
				{
					sleep(10);
					continue;
				}
				else
					i = 0;
			}
			if(p && p->comn)
				p->pid = i;	/* mark new tty shell id */ 
			return(p);
		}
	}
	/*
	 **	This is (possibly) the child
	 */ 
#	ifdef	SHUTDOWN
	signal(14, 1);
#	endif	SHUTDOWN
	ttime = time();
#	ifdef	TRACE
		prs(ctime(ttime));
#	endif	TRACE
	if(waitbuf.limits.l_refcount)
		pe.pw_uid = waitbuf.limits.l_uid;
	else
		pe.pw_uid = waitbuf.ppzomb.p_uid;
	if(getpwlog(&pe, 0, 0) < 0)
	{
#		ifdef	DEBUG
		prs("getpwlog fails\n");
#		endif	DEBUG
		pe.pw_uid = 0;
		getpwlog(&pe, 0, 0);
	}
#endif	AUSAM
	/*
	 **	update utmp login record
	 */ 
	if(p && (f = open(utmpf, 2)) >= 0)
	{
		i = p->line;
		if(i >= '0' && i <= '9')
			i = i-'0';
		else if(i >= 'a' && i <= 'z')
			i = 10+i-'a';
		else if(i >= 'A' && i <= 'Z')
			i = 36+i-'A';
		else
			i =+ 62;
		i =* (sizeof wtmp);
#		ifdef	AUSAM
		seek(f, i, 0);
		read(f, &wtmp, (sizeof wtmp));	/* get login time */ 
#		ifdef	TRACE
			prs(ptime(pe.pw_contime, tbuf));
			prs(ctime(wtmp.u_logintime));
#		endif	TRACE
		if(wtmp.u_logintime)
			pe.pw_contime =+ (ttime-wtmp.u_logintime);
		else
		{
			if(pe.pw_uid == 0)
			{
				/* almost certainly a terminating getty */ 
				pwclose();
				close(f);
				goto skipac;
			}
		}
#		ifdef	TRACE
			prs(ptime(pe.pw_contime, tbuf));
			prs("\n");
#		endif	TRACE
#		endif	AUSAM
		seek(f, i, 0);
		write(f, zero, (sizeof wtmp));	/* clear login time */ 
		close(f);
	}
#ifdef	AUSAM
	/*
	 **	update cpu times
	 */ 
	pe.pw_cputime =+ waitbuf.ppzomb.pz_utime+waitbuf.ppzomb.pz_stime;
#	ifdef	AUSAML
	/*
	 **	clean up after user's last proc
	 */ 
	if(pe.pw_uid && waitbuf.limits.l_refcount == 1)
	{
		tmpclean();
		pe.pw_usage = waitbuf.limits.l_usage;
		pe.pw_extime = ttime;
	}
#	endif	AUSAML
#	ifdef	TRACE
	prs(ptime(pe.pw_contime, tbuf));
	prs(ctime(pe.pw_extime));
#	endif	TRACE
	updtpwent(&pe);
	pwclose();
	/*
	 **	Write account record to wtmp
	 */ 
	wtmp.w_type = p?O_TYPE:W_TYPE;
	wtmp.w_ttyid = p?p->line:0;
	wtmp.w_uid = pe.pw_uid;
	wtmp.w_finishtime = ttime;
	wtmp.w_usertime = waitbuf.ppzomb.pz_utime;
	wtmp.w_systime = waitbuf.ppzomb.pz_stime;
	wrwtmp();
skipac:
	if(p && !p->comn)
		p->line = 0;	/* this tty terminated */ 
	/*
	 **	if we didn't fork, return to init
	 */ 
	if(nofork)
		return(p);
	/*
	 **	otherwise, start up a getty if a shell just died
	 */ 
	if(p && p->line)
		newgetty(p);
	exit(0);
#else	AUSAM
	f = open(wtmpf, 1);
	if(f >= 0)
	{
		wtmp.tty = p->line;
		wtmp.time = time();
		seek(f, 0, 2);
		write(f, &wtmp, (sizeof wtmp));
		close(f);
	}
#endif	AUSAM
}

#ifdef	AUSAM
/*
 *	write wtmp entry
 */ 
wrwtmp()
{
	register f;

	if((f = open(wtmpf, 1)) >= 0)
	{
		seek(f, 0, 2);
		write(f, &wtmp, (sizeof wtmp));
		close(f);
	}
}
#ifdef	AUSAML

/*
 * tmpclean removes all files belonging to user in the temp directory
 *	if the user has created any directories, these are removed by
 *	forking off a process to execute "rmtree"(VIII).
 */ 
tmpclean()
{
	register fd;
	static struct
	{
		int f_ino;
		char f_name[14];
		char zero;
	} dirb;
	static struct statbuf statb;
	register char *name = dirb.f_name;
	register dcount = 0;
#	define	dotdot(n)	((n[1]==0||(n[2]==0&&n[1]=='.'))&&n[0]=='.')

	if(chdir(tmpd) < 0)
		return;
	if(fd = open(tmpd, 0) >= 0)
	{
		while(read(fd, &dirb, 16) == 16)
		{
			if(dirb.f_ino
			   && !dotdot(name)
			      && newstat(name, &statb) >= 0
				 && waitbuf.limits.l_uid == statb.sb_uid)
			{
				if((statb.sb_flags&IFTYP) == IFDIR)
				{
					switch(fork())
					{
					case -1:
#						ifdef	DEBUG
							prs("fork failed\n");
#						endif	DEBUG
						continue;
					case 0:
						close(fd);
						pwclose();
						open(ctty, 2);
						dup(0);
						dup(1);
						execl(rmtree, rmtree_args, name, 0);
#						ifdef	DEBUG
						prs("init: no rmtree!\n");
#						endif	DEBUG
						exit(1);
					default:
						waitx(&dirb);
					}
				}
				else
				{
					unlink(name);
				}
			}
		}
		close(fd);
	}
	chdir(homed);
}
#endif	AUSAML

reset()
{
	signal(1, reset);
	resetflg++;
}
#endif	AUSAM

#ifdef	SHUTDOWN
shtdwn()
{
	register struct tab *p;
	register i;

	signal(14, 1);
#ifndef	AUSAM
	for(ever)
	{
		if((i = waitx(&status)) == -1)
			break;
		for( all)
			if(p->pid == i)
				rmut(p);
	}
#else
	resetflg = 0;
	while(initwait(1) != -1)
		if(resetflg)
		{
			for(i = 0; i < 15; i++)
				close(i);
			termall();
			execl(init, init+5, 0);
			sync();
#ifdef	DEBUG
			prs("init: no init!\n");
#endif	DEBUG
			exit(-1);
		}
#endif	AUSAM
	for(i = 0; i < 15; i++)
		close(i);
	sync();
	prs("\n\nall shells dead - shutdown complete\n\n");
	exit(0);
}
#endif	SHUTDOWN

#ifdef	SHUTDOWN | DEBUG
int fd;
/*
 *	WARNING - do not use in process 1
 *		  as this will attach init to the console tty !
 */ 
prs(s)
register char *s;
{
	fd = open(ctty, 1);
	while(*s)
		putc(*s++);
	close(fd);
}

putc(c)
{
	write(fd, &c, 1);
}
#endif	SHUTDOWN | DEBUG
