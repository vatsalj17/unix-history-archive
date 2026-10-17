/* 
 * cu-- call Unix
 * Modified to use Vadic 3451A modem, escape-code autodialer 
 */
#define	DIALUP		/* line is a dialup, must tell init it's in use. */
/*
 *  If defining DIALUP, and you have job control, compile this with
 *	cc -n -O -o cu cu.c -lc -ljobs
 *  because only wait2 and not signal() should come from libjobs.a.
 */
#include <whoami.h>
#include <stdio.h>
#include <signal.h>
#include <sgtty.h>
#if	defined(SIGTSTP) && defined(DIALUP)
#include <wait.h>
#endif
/*
 * defs that come from uucp.h
 */
#define NAMESIZE 35
#define FAIL -1
#define SAME 0
#define SLCKTIME 5400	/* system/device timeout (LCK.. files) in seconds */
#define ASSERT(e, f, v) if (!(e)) {\
	fprintf(stderr, "AERROR - (%s) ", "e");\
	fprintf(stderr, f, v);\
	cleanup(FAIL);\
}

/*
 *	cu telno [-t] [-s speed] [-l line] [-a acu] [-p]
 *
 *	-t is for dial-out to terminal.
 *	speeds are: 110, 134, 150, 300, 1200, 2400. 300 is default.
 *
 *	-p says strip parity of characters transmitted.  (to compensate
 *	for c100's)
 *
 *	Escape with `~' at beginning of line.
 *	Ordinary diversions are ~<, ~> and ~>>.
 *	Silent output diversions are ~>: and ~>>:.
 *	Terminate output diversion with ~> alone.
 *	Quit is ~. and ~! gives local command or shell.
 *	Also ~$ for canned procedure pumping remote.
 *	~%put from [to]  and  ~%take from [to] invoke builtins
 */

#define CRLF "\r\n"
#define wrc(ds) write(ds,&c,1)


char	*devcul	= "/dev/ttyd0";
char	*devcua	= "/dev/dialout0"; /* same minor number + 200 set */
char	*lspeed = "1200";

int	ln;		/* fd for comm line */
char	tkill, terase;	/* current input kill & erase */
int	notabs;		/* terminal doesn't expand tabs */
int	efk;		/* process of id of listener  */
char	c;
char	oc;

char	*connmsg[] = {
	"",
	"line busy",
	"call dropped",
	"no carrier",
	"can't fork",
	"acu access",
	"tty access",
	"tty hung",
"usage: cu telno [-t] [-p] [-n] [-h] [-b] [-acu#] [-s speed] [-l line] [-a acu]",
	"lock failed: line busy"
};

rdc(ds) {

	ds=read(ds,&oc,1); 
	c = oc & 0177;
	return (ds);
}

int intr;

sig2()
{
	signal(SIGINT, SIG_IGN); 
	intr = 1;
}

int set14;

xsleep(n)
{
	xalarm(n);
	pause();
	xalarm(0);
}

xalarm(n)
{
	set14=n; 
	alarm(n);
}

sig14()
{
	signal(SIGALRM, sig14); 
	if (set14) alarm(1);
}

int	dout;
int	nhup;
int	dbflag;
int	nodial;		/* don't do autodial sequence on modem */
int	pflag;		/* strip parity on chars sent to remote */
int	hdplx;		/* set to emulate half-duplex terminal */
int	nullbrk;	/* turn breaks (nulls) into dels */
int	pipes[2] = { -1, -1 };
int	speed;
#ifdef	DIALUP
int	child;		/* not suid root, let parent do cleanup */
#endif

/*
 *	main: get connection, set speed for line.
 *	spawn child to invoke rd to read from line, output to fd 1
 *	main line invokes wr to read tty, write to line
 */
main(ac,av)
char *av[];
{
	int fk;
	char *telno = NULL;
	struct sgttyb stbuf;
	int cleanup();
#ifdef	DIALUP
#   ifdef	SIGTSTP
	union wait w;
#   else
	int	status;
#   endif
#endif

	signal(SIGALRM, sig14);
	nhup = (int)signal(SIGINT, cleanup);
	signal(SIGHUP, cleanup);
	signal(SIGQUIT, cleanup);
	if (ac < 2) {
		prf(connmsg[8]);
		exit(8);
	}
	for (; ac > 1; av++,ac--) {
		if (av[1][0] != '-')
			telno = av[1];
		else switch(av[1][1]) {
		case 't':
			dout = 1;
			continue;
		case 'b':
			nullbrk++;
			continue;
		case 'd':
			dbflag++;
			continue;
		case 'n':
			nodial++;
			continue;
		case 'h':
			hdplx++;
			continue;
		case 'p':
			pflag++;
			continue;
		case 's':
			lspeed = av[2]; ++av; --ac;
			break;
		case 'l':
			devcul = av[2]; ++av; --ac;
			break;
		case 'a':
			devcua = av[2]; ++av; --ac;
			break;
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			devcua[strlen(devcua)-1] = av[1][1];
			devcul[strlen(devcul)-1] = av[1][1];
			break;
		default:
			prf("Bad flag %s", av[1]);
			break;
		}
	}
	if (telno == NULL && !nodial) {
		prf(connmsg[8]);
		exit(8);
	}
	if (!exists(devcua) || !exists(devcul))
		exit(9);
	switch(atoi(lspeed)) {
	case 110:
		speed = B110;break;
	case 150:
		speed = B150;break;
	case 300:
		speed = B300;break;
	default:
	case 1200:
		speed = B1200;break;
	case 2400:
		speed = B2400;break;
	}
	ln = conn(devcul, devcua, telno);
	if (ln < 0) {
		prf("Connect failed: %s",connmsg[-ln]);
		cleanup(-ln);
	}
	ioctl(0, TIOCGETP, &stbuf);
	notabs = stbuf.sg_flags & XTABS;
	ioctl(ln, TIOCEXCL, (struct sgttyb *)NULL);
	ioctl(ln, TIOCHPCL, (struct sgttyb *)NULL);
	prf("Connected");
#ifdef	DIALUP
	if ((fk=fork()) == 0) {
		setuid(getuid());
		child++;
	} else {
		signal(SIGINT, SIG_IGN);
		signal(SIGHUP, SIG_IGN);
		signal(SIGQUIT, SIG_IGN);
		signal(SIGTERM,SIG_IGN);
#ifndef	SIGTSTP
		wait(&status);
		cleanup(status);
#else
		do {
			wait2(&w.w_status, WUNTRACED);
			if (WIFSTOPPED(w))
				kill(getpid(), SIGTSTP);
		} while (WIFSTOPPED(w));
		cleanup(w.w_status);
#endif
	}
#endif
	pipe(pipes);
	if (dout)
		fk = -1;
	else
		fk = fork();
	signal(SIGINT, SIG_IGN);
	if (fk == 0) {
		chwrsig();
		rd();
		prf("\007Lost carrier");
		cleanup(3);
	}
	mode(1);
	efk = fk;
	wr();
	mode(0);
	if (fk != -1) kill(fk, SIGKILL);
	wait((int *)NULL);
	stbuf.sg_ispeed = 0;
	stbuf.sg_ospeed = 0;
	ioctl(ln, TIOCSETP, &stbuf);
	prf("Disconnected");
	cleanup(0);
}

/*
 *	conn: establish dial-out connection.
 *	Example:  fd = conn("/dev/ttyh","/dev/dn1","4500");
 *	Returns descriptor open to tty for reading and writing.
 *	Negative values (-1...-7) denote errors in connmsg.
 *	Uses alarm and fork/wait; requires sig14 handler.
 *	Be sure to disconnect tty when done, via HUPCL or stty 0.
 */

#include <setjmp.h>
jmp_buf timbuf;
int dn;

conn(dev,acu,telno)
char *dev, *acu, *telno;
{
	struct sgttyb stbuf;
	int timeout();
	int (*old_sig14)();
	int (*old_sig2)();
	extern errno;
	char *ltail, *atail;
	char *s, buf[20];
	char *rindex();
	int er, timerr, dh, tryagain;

	er=0; 
	atail = rindex(acu, '/')+1;
	if (!nodial && mlock(atail) == FAIL) {
		er = 9;
		goto X;
	}
	ltail = rindex(dev, '/')+1;
#ifdef	DIALUP
	if (inuse(ltail)) {
		er = 9;
		goto X;
	}
	if ((mlock(ltail) == FAIL) || (untty(ltail) < 0)) {
#else
	if (mlock(ltail) == FAIL) {
#endif
		er = 9;
		delock(atail);
		goto X;
	}
	if ((dn=open(nodial? dev: acu,2))<0) {
		er=(errno == 6? 1:5); 
		goto X;
	}
	stbuf.sg_ispeed = speed;
	stbuf.sg_ospeed = speed;
	stbuf.sg_flags = EVENP|ODDP;
	if (!dout)
		stbuf.sg_flags |= RAW | TANDEM;
	stbuf.sg_erase = stbuf.sg_kill = 0377;
	ioctl(dn, TIOCSETP, &stbuf);
	old_sig14 = signal(SIGALRM,timeout);
	old_sig2 = signal(SIGINT,timeout);
	if (nodial) {
		dh = dn;
		goto X;
	}
	for (s=telno; *s; s++)
		if (*s == '-')
			*s = 'K';		/* delay char for 3451A */
	tryagain = 1;		/* allow second try to get autodial mode */
	intr = 0;
	if (setjmp(timbuf)) {
		if (!tryagain || intr) {
			er = timerr;
			goto X;
		}
		tryagain = 0;
	}
	timerr = 2;
	alarm(30);
	type(dn,"\005\r",2);			/* enter autodial mode */
	waitfor(dn,"READY");
	waitfor(dn,"*");
	tryagain = 0;
	alarm(35);
	type(dn,"D\r",2);			/* Dial */
	waitfor(dn,"NUMBER?\r\n");
	type(dn,telno,strlen(telno));		/* the number */
	type(dn,"\r",1);
	alarm(30);
	waitfor(dn,telno);
	waitfor(dn, "\r\n");
	write(dn,"\r",1);			/* and confirm it */
	timerr = 3;
	alarm(120);
	waitfor(dn, "DIALING:  ");
	for (s=buf; ; s++) {
		if (read(dn, s, 1) < 1)
			timeout(0);
		if (dbflag)
			write(1, s, 1);
		if (*s == '\n')
			break;
	}
	*s = 0;
	if (strncmp(buf, "ON LINE", 7)) {
		waitfor(dn, "*");
		timeout(0);
	}
	alarm(0);
	dh = open(dev,2);
	if (dh<0)
		er=6; 
X: 
	if (er) {
		ioctl(dn, TIOCHPCL, (struct sgttyb *)NULL);
		close(dn);
	}
	delock(atail);
	signal(SIGALRM,old_sig14);
	signal(SIGINT,old_sig2);
	return (er? -er:dh);
}
/*
 *  wait for string str from line fn.
 *  The caller must time out if we don't get it.
 */
waitfor(fn, str)
int fn;
char *str;
{
	int nextch = 0;
	char *rp = str;

	if (dbflag) {
		write(1, "expected ", 9);
		prf(str);
		write(1, "got ", 4);
	}
	while (*rp) {
		if (read(fn, &nextch, 1) < 1)
			timeout(0);
		if (dbflag)
			write(1,&nextch,1);
		if (*rp == (nextch & 0177))
			rp++;
		else
			rp = str;
	}
	if (dbflag)
		write(1,"\n",1);
}
timeout(sig){
	signal(sig, timeout);
	if (sig == SIGINT)
		intr++;
	type(dn, "I\r", 2);		/* try to get back to Idle mode */
	longjmp(timbuf,1);
}
/*
 *  Type a string to the modem.
 */
type(fd,s,ct)
char *s;
{
	for (; *s; s++) {
		if (*s == '\r')
			sleep(1);
		write(fd,s,1);
	}
}

/*
 *	wr: write to remote: 0 -> line.
 *	~.	terminate
 *	~<file	send file
 *	~!	local login-style shell
 *	~!cmd	execute cmd locally
 *	~$proc	execute proc locally, send output to line
 *	~%cmd	execute builtin cmd (put and take)
 *	~#	send 1-sec break
 *	~^Z	suspend cu process.
 */

wr()
{
	int ds,fk,lcl,x;
	char *p,b[600];
	for (;;) {
		p=b;
		while (rdc(0) == 1) {
			if (p == b) lcl=(c == '~');
			if (p == b+1 && b[0] == '~') lcl=(c!='~');
			if (nullbrk && c == 0) oc=c=0177; /* fake break kludge */
			if (!lcl) {
				if(!pflag)c = oc;
				if (wrc(ln) == 0) {
					prf("line gone"); return;
				}
				if (hdplx) wrc(0);
				c &= 0177;
			}
			if (lcl) {
				if (c == 0177) c=tkill;
				if (c == '\r' || c == '\n') goto A;
				if (!dout) wrc(0);
			}
			*p++=c;
			if (c == terase) {
				p=p-2; 
				if (p<b) p=b;
			}
			if (c == tkill || c == 0177 || c == '\4' || c == '\r' || c == '\n') p=b;
		}
		return;
A: 
		if (!dout) echo("");
		*p=0;
		switch (b[1]) {
		case '.':
		case '\004':
			return;
#ifdef	TIOCSBRK
		case '#':
			if (ioctl(ln, TIOCSBRK, 0) < 0)
				prf("can't send break");
			else {
				sleep(1);
				ioctl(ln, TIOCCBRK, 0);
				continue;
			}
#endif
		case '!':
		case '$':
			fk = fork();
			if (fk == 0) {
				char *getenv();
				char *shell = getenv("SHELL");
				if (shell == 0) shell = "/bin/sh";
				close(1);
				dup(b[1] == '$'? ln:2);
				close(ln);
				mode(0);
				if (!nhup) signal(SIGINT, SIG_DFL);
				if (b[2] == 0) execl(shell,shell,0);
				/* if (b[2] == 0) execl(shell,"-",0); */
				else execl(shell,"sh","-c",b+2,0);
				prf("Can't execute shell");
				exit(~0);
			}
			if (fk!=(-1)) {
				while (wait(&x)!=fk);
			}
			mode(1);
			if (b[1] == '!') echo("!");
			else {
				if (dout) echo("$");
			}
			break;
		case '<':
			if (b[2] == 0) break;
			if ((ds=open(b+2,0))<0) {
				prf("Can't divert %s",b+1); 
				break;
			}
			intr=x=0;
			mode(2);
			if (!nhup) signal(SIGINT, sig2);
			while (!intr && rdc(ds) == 1) {
				if (wrc(ln) == 0) {
					x=1; 
					break;
				}
			}
			signal(SIGINT, SIG_IGN);
			close(ds);
			mode(1);
			if (x) return;
			if (dout) echo("<");
			break;
		case '>':
		case ':':
			{
			register char *q;

			if(pipes[1]==-1) {
				prf("Can't tell other demon to divert");
				break;
			}
			q = b+1;
			if(*q=='>') q++;
			write(pipes[1],q,strlen(q)+1);
			if(dbflag) prf("msg to be delivered:"),prf(q);
			if (efk != -1) kill(efk,SIGEMT);
			}
			break;
#ifdef SIGTSTP
#define CTRLZ	26
		case CTRLZ:
			mode(0);
			kill(getpid(), SIGTSTP);
			mode(1);
			break;
#endif
		case '%':
			dopercen(&b[2]);
			break;
		default:
			prf("Use `~~' to start line with `~'");
		}
		continue;
	}
}

dopercen(line)
register char *line;
{
	char *args[10];
	register narg, f;
	int rcount;
	for (narg = 0; narg < 10;) {
		while(*line == ' ' || *line == '\t')
			line++;
		if (*line == '\0')
			break;
		args[narg++] = line;
		while(*line != '\0' && *line != ' ' && *line != '\t')
			line++;
		if (*line == '\0')
			break;
		*line++ = '\0';
	}
	if (equal(args[0], "take")) {
		if (narg < 2) {
			prf("usage: ~%%take from [to]");
			return;
		}
		if (narg < 3)
			args[2] = args[1];
		write(pipes[1], ">/dev/null",sizeof(">/dev/null"));
		if(dbflag) prf("sending take message");
		if (efk != -1) kill(efk,SIGEMT);
		xsleep(5);
		if (notabs)
			wrln("stty tabs;");
		wrln("echo '~>:");
		wrln(args[2]);
		wrln("'; tee /dev/null <");
		wrln(args[1]);
		wrln(";echo '~>'");
		if (notabs)
			wrln(";stty -tabs");
		wrln("\n");
		return;
	} else if (equal(args[0], "put")) {
		if (narg < 2) {
			prf("usage: ~%%put from [to]");
			return;
		}
		if (narg < 3)
			args[2] = args[1];
		if ((f = open(args[1], 0)) < 0) {
			prf("cannot open: %s", args[1]);
			return;
		}
		wrln("stty -echo;cat >");
		wrln(args[2]);
		wrln(";stty echo\n");
		xsleep(5);
		intr = 0;
		if (!nhup)
			signal(SIGINT, sig2);
		mode(2);
		rcount = 0;
		while(!intr && rdc(f) == 1) {
			rcount++;
			if (c == tkill || c == terase)
				wrln("\\");
			if (wrc(ln) != 1) {
				xsleep(2);
				if (wrc(ln) != 1) {
					prf("character missed");
					intr = 1;
					break;
				}
			}
		}
		signal(SIGINT, SIG_IGN);
		close(f);
		if (intr) {
			wrln("\n");
			prf("stopped after %d bytes", rcount);
		}
		wrln("\004");
		xsleep(5);
		mode(1);
		return;
	}
	prf("~%%%s unknown\n", args[0]);
}

equal(s1, s2)
register char *s1, *s2;
{
	while (*s1++ == *s2)
		if (*s2++ == '\0')
			return(1);
	return(0);
}

wrln(s)
register char *s;
{
	while (*s)
		write(ln, s++, 1);
}
/*	chwrsig:  Catch orders from wr process 
 *	to instigate diversion
 */
int whoami;
chwrsig(){
	int readmsg(); 
	whoami = getpid();
	signal(SIGEMT,readmsg);
}
int ds,slnt,taking;
int justrung;
readmsg(){
	static char dobuff[128], morejunk[256];
	int n;
	justrung = 1;
	signal(SIGEMT,readmsg);
	if(dbflag) {
		prf("About to read from pipe");
	}
	n = read(pipes[0],morejunk,256);
	if(dbflag) {
		prf("diversion mesg recieved is");
		prf(morejunk);
		prf(CRLF);
	}
	dodiver(morejunk);
}
dodiver(msg)
char *msg;
{
	register char *cp = msg; 

	if (*cp=='>') cp++;
	if (*cp==':') {
		cp++;
		if(*cp==0) {
			slnt ^= 1;
			return;
		} else  {
			slnt = 1;
		}
	}
	if (ds >= 0) close(ds);
	if (*cp==0) {
		slnt = 0;
		ds = -1;
		return;
	}
	if (*msg!='>' || (ds=open(cp,1))<0) ds=creat(cp,0644);
	lseek(ds, (long)0, 2);
	if(ds < 0) prf("Creat failed:"), prf(cp);
	if (ds<0) prf("Can't divert %s",cp+1);
}


/*
 *	rd: read from remote: line -> 1
 *	catch: diversion caught by interrupt routine
 */

#define ORDIN 0
#define SAWCR 1
#define EOL   2
#define SAWTL 3
#define DIVER 4

rd()
{
	extern int ds,slnt;
	char rb[600], lb[600], *rlim, *llim, c;
	register char *p,*q;
	int cnt, state = ORDIN, mustecho, oldslnt;

	ds=(-1);
	p = lb; llim = lb+600;
agin:
	while((cnt = read(ln,rb,600)) > 0) {
		if(!slnt) {
			if (pflag)
				for (q=rb, rlim = rb + cnt - 1; q <= rlim; )
					*q++ &= 0177;
			write(1,rb,cnt);
		}
		if(ds < 0) continue;
		oldslnt = slnt;
		for( q=rb, rlim = rb + cnt - 1; q <= rlim; ) {
			c = *q++ & 0177;
			if(p < llim) *p++ = c;
			switch(state) {
			case ORDIN:
				if(c=='\r') state = SAWCR;
				break;
			case SAWCR:
				if(c=='\n') {
					state = EOL;
					p--;
					p[-1] = '\n';
				} else state = ORDIN;
				break;
			case EOL:
				state = (c=='~' ? SAWTL : 
					 (c=='\r' ? SAWCR : ORDIN));
				break;
			case SAWTL:
				state = (c=='>' ? DIVER : 
					 (c=='\r' ? SAWCR : ORDIN));
				break;
			case DIVER:
				if(c=='\r') {
					p--;
				} else if (c=='\n') {
					state = ORDIN;
					p[-1] = 0;
					dodiver(lb+2);
					c = 0; p = lb;
				}
			}
			if(slnt==0 && oldslnt) {
				if(c=='\n') {
					write(1,lb,p-lb-1);
					write(1,CRLF,sizeof(CRLF));
				} else if(q==rlim) {
					write(1,lb,p-lb);
					c = '\n';  /*force flush to file*/
				}
			}
			if(c=='\n') {
				if(ds >= 0)
					write(ds,lb,p-lb);
				p = lb;
			}
		}
	}
	if(justrung) {
		justrung = 0;
		goto agin;
	}
}

struct {char lobyte; char hibyte;};

mode(f)
{
	struct sgttyb stbuf;
	if (dout) return;
	ioctl(0, TIOCGETP, &stbuf);
	tkill = stbuf.sg_kill;
	terase = stbuf.sg_erase;
	if (f == 0) {
		stbuf.sg_flags &= ~RAW;
		stbuf.sg_flags |= ECHO|CRMOD;
	}
	if (f == 1) {
		stbuf.sg_flags |= RAW;
		stbuf.sg_flags &= ~(ECHO|CRMOD);
	}
	if (f == 2) {
		stbuf.sg_flags &= ~RAW;
		stbuf.sg_flags &= ~(ECHO|CRMOD);
	}
	ioctl(0, TIOCSETN, &stbuf);
}

echo(s)
char *s;
{
	char *p;
	for (p=s;*p;p++);
	if (p>s) write(0,s,p-s);
	write(0,CRLF, sizeof(CRLF));
}

prf(f, s)
char *f;
char *s;
{
	fprintf(stderr, f, s);
	fprintf(stderr, CRLF);
}

exists(devname)
char *devname;
{
	if (access(devname, 0)==0)
		return(1);
	prf("%s does not exist", devname);
	return(0);
}

cleanup(code)
{
#ifdef	DIALUP
	/*
	 * Let the parent do the cleanup.
	 */
	if (child)
		exit(code);
#endif
	rmlock(NULL);
	ioctl(ln, TIOCNXCL, (struct sgttyb *)NULL);
	close(ln);
	close(dn);

#ifdef	DIALUP
	ttyrlse();
#endif
	exit(code);
}

/*
 * This code is taken directly from uucp and follows the same
 * conventions.  This is important since uucp and cu should
 * respect each others locks.
 */

	/*  ulockf 3.2  10/26/79  11:40:29  */
/* #include "uucp.h" */
#include <sys/types.h>
#include <sys/stat.h>



/*******
 *	ulockf(file, atime)
 *	char *file;
 *	time_t atime;
 *
 *	ulockf  -  this routine will create a lock file (file).
 *	If one already exists, the create time is checked for
 *	older than the age time (atime).
 *	If it is older, an attempt will be made to unlink it
 *	and create a new one.
 *
 *	return codes:  0  |  FAIL
 */

ulockf(file, atime)
char *file;
time_t atime;
{
	struct stat stbuf;
	time_t ptime;
	int ret;
	static int pid = -1;
	static char tempfile[NAMESIZE];

	if (pid < 0) {
		pid = getpid();
		sprintf(tempfile, "/usr/spool/uucp/LTMP.%d", pid);
	}
	if (onelock(pid, tempfile, file) == -1) {
		/* lock file exists */
		/* get status to check age of the lock file */
		ret = stat(file, &stbuf);
		if (ret != -1) {
			time(&ptime);
			if ((ptime - stbuf.st_ctime) < atime) {
				/* file not old enough to delete */
				return(FAIL);
			}
		}
		ret = unlink(file);
		ret = onelock(pid, tempfile, file);
		if (ret != 0)
			return(FAIL);
	}
	stlock(file);
	return(0);
}


#define MAXLOCKS 10	/* maximum number of lock files */
char *Lockfile[MAXLOCKS];
int Nlocks = 0;

/***
 *	stlock(name)	put name in list of lock files
 *	char *name;
 *
 *	return codes:  none
 */

stlock(name)
char *name;
{
	char *p;
	extern char *calloc();
	int i;

	for (i = 0; i < Nlocks; i++) {
		if (Lockfile[i] == NULL)
			break;
	}
	ASSERT(i < MAXLOCKS, "TOO MANY LOCKS %d", i);
	if (i >= Nlocks)
		i = Nlocks++;
	p = calloc(strlen(name) + 1, sizeof (char));
	ASSERT(p != NULL, "CAN NOT ALLOCATE FOR %s", name);
	strcpy(p, name);
	Lockfile[i] = p;
	return;
}


/***
 *	rmlock(name)	remove all lock files in list
 *	char *name;	or name
 *
 *	return codes: none
 */

rmlock(name)
char *name;
{
	int i;

	for (i = 0; i < Nlocks; i++) {
		if (Lockfile[i] == NULL)
			continue;
		if (name == NULL
		|| strcmp(name, Lockfile[i]) == SAME) {
			unlink(Lockfile[i]);
			free(Lockfile[i]);
			Lockfile[i] = NULL;
		}
	}
	return;
}


/*  this stuff from pjw  */
/*  /usr/pjw/bin/recover - check pids to remove unnecessary locks */
/*	isalock(name) returns 0 if the name is a lock */
/*	unlock(name)  unlocks name if it is a lock*/
/*	onelock(pid,tempfile,name) makes lock a name
	on behalf of pid.  Tempfile must be in the same
	file system as name. */
/*	lock(pid,tempfile,names) either locks all the
	names or none of them */
isalock(name) char *name;
{
	struct stat xstat;
	if(stat(name,&xstat)<0) return(0);
	if(xstat.st_size!=sizeof(int)) return(0);
	return(1);
}
unlock(name) char *name;
{
	if(isalock(name)) return(unlink(name));
	else return(-1);
}
onelock(pid,tempfile,name) char *tempfile,*name;
{	int fd;
	fd=creat(tempfile,0444);
	if(fd<0) return(-1);
	write(fd,(char *) &pid,sizeof(int));
	close(fd);
	if(link(tempfile,name)<0)
	{	unlink(tempfile);
		return(-1);
	}
	unlink(tempfile);
	return(0);
}
lock(pid,tempfile,names) char *tempfile,**names;
{	int i,j;
	for(i=0;names[i]!=0;i++)
	{	if(onelock(pid,tempfile,names[i])==0) continue;
		for(j=0;j<i;j++) unlink(names[j]);
		return(-1);
	}
	return(0);
}

#define LOCKPRE "/usr/spool/uucp/LCK."

/***
 *	delock(s)	remove a lock file
 *	char *s;
 *
 *	return codes:  0  |  FAIL
 */

delock(s)
char *s;
{
	char ln[30];

	sprintf(ln, "%s.%s", LOCKPRE, s);
	rmlock(ln);
}


/***
 *	mlock(sys)	create system lock
 *	char *sys;
 *
 *	return codes:  0  |  FAIL
 */

mlock(sys)
char *sys;
{
	char lname[30];
	sprintf(lname, "%s.%s", LOCKPRE, sys);
	return(ulockf(lname, (time_t) SLCKTIME ) < 0 ? FAIL : 0);
}



/***
 *	ultouch()	update access and modify times for lock files
 *
 *	return code - none
 */

ultouch()
{
	time_t time();
	int i;
	struct ut {
		time_t actime;
		time_t modtime;
	} ut;

	ut.actime = time(&ut.modtime);
	for (i = 0; i < Nlocks; i++) {
		if (Lockfile[i] == NULL)
			continue;
		utime(Lockfile[i], &ut);
	}
	return;
}

#ifdef	DIALUP
/*
 * Routines to allow cu to use a dialup line to call out.
 */
#include <utmp.h>
#define UTMP	"/etc/utmp"
#define TTYS	"/etc/ttys"
#define MAXLINE 80		/* maximum line length in TTYS */

static char	WasOn;		/* first char of line's entry in ttys */
static char	*tty;		/* last part of tty device pathname */
char	*rindex();

/*
 * Check whether anyone is logged in on devcul.
 */
inuse(devcul)
char *devcul;
{
	register fi;
	struct utmp utmpb;
	char *tmp;

	if ((tmp=rindex(devcul,'/')) != NULL)
		devcul = tmp+1;
	fi=open(UTMP,0);
	while(read(fi,&utmpb,sizeof(utmpb)) == sizeof(utmpb))
		if (strcmp(utmpb.ut_line,devcul) == 0) {
			if ( *utmpb.ut_name != 0 ) {
				fprintf(stderr,"%s is in use\n",devcul);
				return(-1);
			}
			break;
		}
	close(fi);
	return(0);
}

#ifdef	DIALUP
/*
 *  Remove devcul from TTYS file, notify init that line is
 *  no longer available.
 */
untty(devcul)
char *devcul;
{
	register fi, ret;
	char	line[MAXLINE];

	if ((tty=rindex(devcul,'/')) != NULL)
		tty++;
	else
		tty = devcul;
	fi = open(TTYS,2);
	if (fi < 0) {
		fprintf(stderr,"can't modify %s\n",TTYS);
		return (-1);
	}
	WasOn = '0';
	while (ret=getline(fi,line)) {
		if (equal(line+2,tty)) {
			WasOn = line[0];
			break;
		}
	}
	if (ret == 0)
		return(0);
	if (WasOn != '0') {
		lseek(fi,-(long)(ret+1),1);
		write(fi,"0",1);
	}
	close(fi);
	if (WasOn != '0') {
		kill(1,1);
		sleep(2);	/* wait for init/getty to go away */
	}
	return(0);
}

/*
 *  Restore line to dialup service (removed from service by untty).
 *  Uses pointer to name saved by untty.
 */
ttyrlse()
{
	register fi, ct;
	char *tmp;
	char	line[MAXLINE];

	if (tty == NULL || WasOn == '0')
		return;
	fi = open(TTYS,2);
	if (fi < 0) {
		fprintf(stderr,"can't modify %s\n",TTYS);
		return (-1);
	}
	while (ct=getline(fi,line)) {
		if (equal(line+2,tty)) {
			lseek(fi,-(long)(ct+1),1);
			write(fi,&WasOn,1);
			close(fi);
			kill(1,1);
			return(0);
		}
	}
	fprintf(stderr,"%s not in %s now\n",tty,TTYS);
	return(-1);
}
#endif	DIALUP

/*
 * Read a line from the ttys file.  Null terminate at the end
 * of the name.
 */
getline(fd,buf)
char *buf;
{
	char c;
	register char *p, *lim = buf+MAXLINE-1;
	register count = 0;

	p = buf;
	while (read(fd,&c,1) && c!='\n') {
		count++;
		if (p<lim)
			*p++ = c;
	}
	*p = 0;
	for (p=buf; *p; p++)
		if (*p==' ' || *p=='\t')
			break;
	*p = 0;
	return(count);
}
#endif
