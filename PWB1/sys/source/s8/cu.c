/*
 *	cu telno [-t] [-s speed] [-l line] [-a acu]
 *
 *	-t is for dial-out to terminal.
 *	speeds are: 110, 134, 150, 300, 1200. 300 is default.
 *
 *	Escape with `~' at beginning of line.
 *	Ordinary diversions are ~<, ~> and ~>>.
 *	Silent output diversions are ~>: and ~>>:.
 *	Terminate output diversion with ~> alone.
 *	Quit is ~. and ~! gives local command or shell.
 *	Also ~$ for canned procedure pumping remote.
 *	~%put from [to]  and  ~%take from [to] invoke builtins
 */

#include "stat.h"
#define EVEN  0200
#define ODD   0100
#define RAW   0040
#define CRMOD 0020
#define ECHO  0010
#define HUPCL 0001

#define CRLF "\r\0\0\0\n\0\0\0",8

#define wrc(ds) write(ds,&c,1)
#define spd(n)	(n<<8)|n


char	*devcul	"/dev/cul0";
char	*devcua	"/dev/cua0";
char	*lspeed	"300";

int	ln;	/* fd for comm line */
char	tkill, terase;	/* current input kill & erase */
char	c;

char	*connmsg[] {
	"",
	"line busy",
	"call dropped",
	"no carrier",
	"can't fork",
	"acu access",
	"tty access",
	"tty hung",
	"usage: cu telno [-t] [-s speed] [-l line] [-a acu]"
};

rdc(ds) {
	ds=read(ds,&c,1); 
	c=& 0177; 
	return (ds);
}

int intr;

sig2()
{
	signal(2,1); 
	intr=1;
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
	signal(14,sig14); 
	if (set14) alarm(1);
}

int nhup,dout;

/*
 *	main: get connection, set speed for line.
 *	spawn child to invoke rd to read from line, output to fd 1
 *	main line invokes wr to read tty, write to line
 */
main(ac,av)
char *av[];
{
	int fk,v[3];
	int speed;
	char *telno;
	signal(14,sig14);
	if (ac < 2) {
		prf(connmsg[8]);
		exit(8);
	}
	telno = av[1];
	av =+ 2;
	ac =- 2;
	for (; ac > 0; av++) {
		if (equal(*av, "-t")) {
			dout = 1;
			--ac;
			continue;
		}
		if (ac < 2)
			break;
		if (equal(*av, "-s"))
			lspeed = *++av;
		else if (equal(*av, "-l"))
			devcul = *++av;
		else if (equal(*av, "-a"))
			devcua = *++av;
		else
			break;
		ac =- 2;
	}
	if (!exists(devcua) || !exists(devcul))
		exit(9);
	ln = conn(devcul, devcua, telno);
	if (ln < 0) {
		prf("Connect failed: %s",connmsg[-ln]);
		exit(-ln);
	}
	switch(atoi(lspeed)) {
	case 110:
		speed = spd(3);break;
	case 134:
		speed = spd(4);break;
	case 150:
		speed = spd(5);break;
	default:
	case 300:
		speed = spd(7);break;
	case 1200:
		speed = spd(9);break;
	}
	v[0] = speed;
	v[2]=EVEN|ODD|HUPCL;
	if (!dout) v[2]=| RAW;
	stty(ln, v);
	prf("Connected");
	if (dout) fk=(-1); 
	else
		fk = dofork();
	nhup=signal(2,1);
	if (fk == 0) {
		rd();
		prf("\007Lost carrier");
		exit(3);
	}
	mode(1);
	wr();
	mode(0);
	kill(fk,9);
	wait(v);
	v[0]=0;
	v[2]=HUPCL;
	stty(ln, v);
	prf("Disconnected");
	exit(0);
}

/*
 *	conn: establish dial-out connection.
 *	Example:  fd = conn("/dev/ttyh","/dev/dn1","4500");
 *	Returns descriptor open to tty for reading and writing.
 *	Negative values (-1...-7) denote errors in connmsg.
 *	Uses alarm and fork/wait; requires sig14 handler.
 *	Be sure to disconnect tty when done, via HUPCL or stty 0.
 */

conn(dev,acu,telno)
char *dev,*acu,*telno;
{
	extern errno;
	char *p,*q,b[30];
	int er,fk,dn,dh,t,v[3];
	er=0; 
	fk=(-1);
	if ((dn=open(acu,1))<0) {
		er=(errno == 6? 1:5); 
		goto X;
	}
	if ((fk=fork()) == (-1)) {
		er=4; 
		goto X;
	}
	if (fk == 0) {
		open(dev,2); 
		for (;;) pause();
	}
	xsleep(2);
	/*
	 *	copy phone #, assure -= at end
	 */
	p=b; 
	q=telno;
	while (*p++=(*q++));
	p--;
	if (*(p-1)!='=') {
		if (*(p-1)!='-') *p++='-';
		*p++='=';
	}
	t=p-b;
	xalarm(5*t);
	t=write(dn,b,t);
	xalarm(0);
	if (t<0) {
		er=2; 
		goto X;
	}
	close(dn);
	xalarm(10);		/* was 5; sometimes missed carrier */
	dh = open(dev,2);
	xalarm(0);
	if (dh<0) {
		er=(errno == 4? 3:6); 
		goto X;
	}
	gtty(dh,v);
	v[2] = (v[2]|HUPCL) & ~ECHO;
	xalarm(10);
	t=stty(dh,v);
	xalarm(0);
	if (t<0) er=(errno == 4? 7:6);
X: 
	if (er) close(dn);
	if (fk!=(-1)) {
		kill(fk,9);
		xalarm(10);
		while ((t=wait(v))!=(-1) && t!=fk);
		xalarm(0);
	}
	return (er? -er:dh);
}

/*
 *	wr: write to remote: 0 -> line.
 *	~.	terminate
 *	~<file	send file
 *	~!	local login-style shell
 *	~!cmd	execute cmd locally
 *	~$proc	execute proc locally, send output to line
 *	~%cmd	execute builtin cmd (put and take)
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
			if (c == 0) c=0177;
			if (!lcl) {
				if (wrc(ln) == 0) {
					prf("line gone"); return;
				}
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
			if (c == tkill || c == 0177 || c == '\r' || c == '\n') p=b;
		}
		return;
A: 
		if (!dout) echo("");
		*p=0;
		switch (b[1]) {
		case '.':
		case '\004':
			return;
		case '!':
		case '$':
			fk = dofork();
			if (fk == 0) {
				close(1);
				dup(b[1] == '$'? ln:2);
				close(ln);
				mode(0);
				if (!nhup) signal(2,0);
				if (b[2] == 0) execl("/bin/sh","-",0);
				else execl("/bin/sh","sh","-c",b+2,0);
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
			if (!nhup) signal(2,sig2);
			while (!intr && rdc(ds) == 1) {
				if (wrc(ln) == 0) {
					x=1; 
					break;
				}
			}
			signal(2,1);
			close(ds);
			mode(1);
			if (x) return;
			if (dout) echo("<");
			break;
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
	int fk, v;
	int rcount, wcount, wrcret;
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
		wrln("echo '~>:'");
		wrln(args[2]);
		wrln(";tee /dev/null <");
		wrln(args[1]);
		wrln(";echo '~>'\n");
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
			signal(2, sig2);
		mode(2);
		wcount = rcount = 0;
		while(!intr && rdc(f) == 1) {
			rcount++;
			if (c == tkill || c == terase)
				wrln("\\");
			if ((wrcret = wrc(ln)) == 1)
				wcount++;
			else {
				xsleep(2);
				if (wrc(ln) == 1)
					wcount++;
				else {
					prf("character missed");
					intr = 1;
					break;
				}
			}
		}
		signal(2, 1);
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
	prf("~%%%s unknown\n",args[0]);
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

/*
 *	rd: read from remote: line -> 1
 *	catch:
 *	~>[>][:][file]
 *	stuff from file...
 *	~>	(ends diversion)
 */

rd()
{
	int ds,slnt;
	char *p,*q,b[600];
	p=b;
	ds=(-1);
	while (rdc(ln) == 1) {
		if (ds<0) slnt=0;
		if (!slnt) wrc(1);
		*p++=c;
		if (c!='\n') continue;
		q=p; 
		p=b;
		if (b[0]!='~' || b[1]!='>') {
			if (*(q-2) == '\r') {
				q--; 
				*(q-1)=(*q);
			}
			if (ds>=0) write(ds,b,q-b);
			continue;
		}
		if (ds>=0) close(ds);
		if (slnt) {
			write(1, b, q - b);
			write(1, CRLF);
		}
		if (*(q-2) == '\r') q--;
		*(q-1)=0;
		slnt=0;
		q=b+2;
		if (*q == '>') q++;
		if (*q == ':') {
			slnt=1; 
			q++;
		}
		if (*q == 0) {
			ds=(-1); 
			continue;
		}
		if (b[2]!='>' || (ds=open(q,1))<0) ds=creat(q,0644);
		seek(ds,0,2);
		if (ds<0) prf("Can't divert %s",b+1);
	}
}

struct {char lobyte; char hibyte;};
mode(f)
{
	int v[3];
	if (dout) return;
	gtty(0,v);
	tkill = v[1].hibyte;
	terase = v[1].lobyte;
	if (f == 0) {
		v[2]=& ~RAW;
		v[2]=| (ECHO|CRMOD);
	}
	if (f == 1) {
		v[2]=| RAW;
		v[2]=& ~(ECHO|CRMOD);
	}
	if (f == 2) {
		v[2]=& ~RAW;
		v[2]=& ~(ECHO|CRMOD);
	}
	stty(0,v);
}

echo(s)
char *s;
{
	char *p;
	for (p=s;*p;p++);
	if (p>s) write(0,s,p-s);
	write(0,CRLF);
}

dofork()
{
	register fk, i;
	for (i = 10; i < 60; i =+ 10) {
		if ((fk = fork()) != -1)
			return(fk);
	}
	prf(connmsg[4]);
	return(-1);
}

prf(a0,a1,a2,a3)
{
	extern int fout;
	fout=2;
	printf(a0,a1,a2,a3);
	write(2,CRLF);
}

exists(devname)
char *devname;
{
	struct  statb exstat;
	if (stat(devname, &exstat) == 0)
		return(1);
	prf("%s does not exist",devname);
	return(0);
}
