/*
 * PWB/UNIX shell
 * copyright 1973 Bell Telephone Laboratries inc.
 * 2.44 of 5/26/77
 *
 * Modified at UNSW by Chris Maltby Oct '78 to
 *	Improve glob
 *	Code for AUSAM
 *	Make accounting optional
 *	Implement $$1 etc.
 *	Allow cd %root
 *	Include code for 'kill'
 *	Timeout terminals
 *	Print out statistics at log-off
 *	Echoall & echonone supplied
 *	Root default path is "/etc/bin" "/etc" "/bin" ""
 *	Execute pascal (px) and spitbol files.
 *
 * Further modified at UNSW by Greg Rose Jan '79.
 *	remembers last person looked up with '%' or '@' (tend to be repetitive)
 *	'$^' is last command line
 *	'$^n' is nth item of previous command line
 *	'$$^n' is nth n+1th ... items of previous command line.
 *
 * Non-AUSAM code corrected by Chris Doney CSU Mar '79.
 *	knows about old style password file.
 *	'wait' command changed to allow to wait
 *		for specific proc.
 *	'wait' is also rub-outable.
 *
 * Above fix fixed so that AUSAM part still works Chris Maltby Apr '79.
 *
 */

#include	<local-system>
#include	<errnos.h>
#include	<stat.h>
#ifdef	AUSAM
#include	<passwd.h>
#endif	AUSAM
#define NOFILE	15	/* max open files per process */

#define	INTR	2
#define	QUIT	3
#define	SIGPIPE	13
#define LINSIZ 512
#define ARGSIZ 100
#define TRESIZ 100

#define QUOTE 0200

#define FAND 0001	/* This tree is '&'ed */
#define FCAT 0002	/* The '>' is really a '>>' */
#define FPIN 0004	/* A pipe is input */
#define FPOU 0010	/* A pipe is output */
#define FPAR 0020	/* The end of a tree in '()' */
#define FINT 0040	/* The tree is interrupt protected */
#define FPRS 0100	/* Print proc ids of '&'s */

#define TCOM 1		/* A command */
#define TPAR 2		/* A parenthesized command tree */
#define TFIL 3		/* DLEF is piped to DRIT */
#define TLST 4		/* A command list */
#define TOR  5		/* Execute DRIT if not DLEF */
#define TAND 6		/* Execute DRIT if DLEF */

#define DTYP 0
#define DFLG 1
#define DLEF 2
#define DRIT 3
#define DSPR 4
#define DCOM 5

#define N 'n'-'a'
#define P 'p'-'a'
#define R 'r'-'a'
#define S 's'-'a'
#define T 't'-'a'
#define W 'w'-'a'
#define Z 'z'-'a'
#define DOLREPL 1
#define DOLREPQ	2

extern errno;
/*	BSIZ = buffer for tty, BSIZFIL = size for file, 1 = size for pipe */
#define BSIZ 64
#define BSIZFIL	512
struct
{
	long start;	/* absolute file addr of char in buf[0] */
	long linloc;	/* loc of begin of line for while processing */
	int gotten;	/* number of bytes read in last actual read */
	int nleft;
	char *nextp;
} b;
#ifndef	AUSAM

#define	uid_pos	2
#define	dir_pos	5
struct
	{
	char	u_logname[8],
		u_logdir[22],
		u_logtty[2];
	}
	user;
	char	*skip_colon();
#define	logname()	user.u_logname
#define	logdir()	user.u_logdir
#define	logtty()	user.u_logtty
char	pwbuff[128];
#endif	AUSAM

char	bstate;	/* primary state var for control of buffering */
		/* 0 ==> BNOW right; real I/O ptr set at BEND (internal I/O) */
		/* 1 ==> real ptr has been set from BNOW (pre-exec) */
		/* 2 ==> post-exec - internals must be reset */
		/* pre-exec: 0->1; post-exec: 1->2; int cmd: 2->0 */
		/* pump begin: 1->0 */
char	b1char;	/* input buffer: 1 at a time */
char	*bbuf	&b1char;	/* addr of actual buffer */
#define	BNOW	(b.start + (b.gotten - b.nleft))
#define	BEND	(b.start + b.gotten)
int bnread BSIZ;	/* must change to 1 if found to be in pipe */

#define ACNAME	"/etc/sha"

char	*dolp;
int	idolp	-1;
int	oldfil0;	/* fildes for original file 0 */
char	pidp[6], argstr[6];
int	wide 5; /* glitch for 5char pid # */
char	devtty[10];
int	ldivr;
char	**dolv;
int	dolc;
char	**wordp;
int	prevc;
int	nwords;
char	*promp, *opromp, *optpromp;
char	*linep;
char	*elinep;
char	**argp;
char	**eargp;
int	*treep;
int	*treeend;
char	peekc;
char	secondpeek;
char	gflg;
char	error;
#ifdef	ACCOUNTING
char	acctf;	/*	<= 0 ==> no acctg at all, >0 ==> acctg */
char	acctfi;	/*	= 0 ==> no acctg for internal cmds */
#endif	ACCOUNTING
char	setintr;
int	uid;
char	*arginp;
char	onelflg;
char	rflg;
char	verbflg;	/* is this a sh -c or -t? */
int	exitcode;
char	exitstr[6] ;
char	*seta[26];
char	*endcore, *endptr;
int	oldintr;	/* save INTR state existing at start */
int	wasintr;
char	gointr[32];
int	catchintr();
int	timeout();
char	proflag;	/* 1 ==> inside .profile */
			/* 2 ==> interrupt in .profile */
char	redirf;	/* 1 ==> I/O redirection of input; controls pump rebuffering */
char	optfv;		/* 0 ==> +v, 1 ==> -v; -v ==> print commands */
char	ttyflag;	/* 1 ==> a login shell */


struct statb sb;

/*	commands performed internally */
char	*comint[]
{
	"chdir",	/* 0 */
	"shift",	/* 1 */
	"cd",		/* 2 */
	"wait",		/* 3 */
	":",		/* 4 */
	"onintr",	/* 5 */
	"=",		/* 6 */
	"next",		/* 7 */
	"if",		/* 8 */
	"else",		/* 9 */
	"endif",	/* 10 */
	"goto",		/* 11 */
	"switch",	/* 12 */
	"break",	/* 13 */
	"endsw",	/* 14 */
	"exit",		/* 15 */
	"while",	/* 16 */
	"end",		/* 17 */
	"continue",	/* 18 */
	"breaksw",	/* 19 */
	"opt",		/* 20 */
	"times",	/* 21 */
	"echoall",	/* 22 */
	"echonone",	/* 23 */
#ifdef	AUSAM
	"kill",		/* 24 */
#else
	"login",	/* 24 */
	"test",		/* 25 */
#endif	AUSAM
	0
};

/* defines for internal commands */
#define	ZCHDIR		0
#define	ZSHIFT		1
#define ZCD		2
#define	ZWAIT		3
#define	ZLABEL		4
#define	ZONINTR		5
#define	ZEQUALS		6
#define	ZNEXT		7
#define	ZIF		8
#define	ZELSE		9
#define	ZENDIF		10
#define	ZGOTO		11
#define	ZSWITCH		12
#define	ZBREAK		13
#define	ZENDSW		14
#define	ZEXIT		15
#define	ZWHILE		16
#define	ZEND		17
#define	ZCONTINUE	18
#define	ZBREAKSW	19
#define	ZOPT		20
#define	ZTIMES		21
#define	ZECHOALL	22
#define	ZECHONONE	23
#ifdef	AUSAM
#define ZKILL		24
#else
#define ZLOGIN		24
#define	ZTEST		25
#endif	AUSAM
#define	ONINTR	comint[ZONINTR]
#define	IF	comint[ZIF]
#define	ENDIF	comint[ZENDIF]
#define	ENDSW	comint[ZENDSW]
#define	END	comint[ZEND]

/*	other keywords */
char	THEN[]	"then";
char	*ARG0, *ARG1;	/* for diagnostics & internal commands */
int	COMTYPE;	/* code for command, -1, or one of Z* */

char	*mesg[]
{
	0,
	"Hangup",
	0,
	"Quit",
	"Illegal instruction",
	"Trace/BPT trap",
	"IOT trap",
	"EMT trap",
	"Floating exception",
	"Killed",
	"Bus error",
	"Segmentation violation",
	"Bad system call",
	"Broken pipe",
	"System termination",
	"Real time limit",
	"CPU time limit",
	"Memory parity",
	"Signal 18",
	"Signal 19",
	0,
};

/*	messages */
char	*SYNTAX	"syntax error: ";
char	*MISS	"missing ";
char	*MISSL	"missing label: ";
char	*NONUM	"non-numeric arg - ";

char	*ARGCNT	"arg count";
char	*ARGLNG "arg list too long";
char	*BADARG	"bad arg: ";
char	*CANTEX	"cannot execute";
char	*CANTOP	"cannot open: ";
char	*NOTPER	"not permitted";
char	*NOTWHI	"used outside loop";
char	*EQERR	"`=' error";
char	*BADDIR	"bad directory";

struct stime
{
	long procu, procs, childu, childs, curtim;
} timeb;
long starttime;
long time();
#define	HZ	50	/* This is AUSTRALIA mate */

/* pascal interpreter 'px' */
char	PX[]		"/bin/px";

#ifdef	ACCOUNTING
struct
{
	char cname[8];
	char lname[6];
	char shtty;
	int tuid;
	long datet;
	long realt;
	long bcput;
	long bsyst;
} tbuf;
#endif	ACCOUNTING

long tell();
char *nxtarg(), *sname();

/*	following items implement while -- end stack of WDEEP levels */
#define	WDEEP	3
#define INWHILE	(wtop < WDEEP)
struct
{
	long sloc;	/* starting loc = addr of while */
	long eloc;	/* ending loc = addr of line AFTER end */
} wstk[WDEEP];
int	wtop	WDEEP;	/* top of stack */

main(c, av)
int c;
char **av;
{
	register f;
	register char *p, **v;
#ifdef	AUSAM
	struct pwent pe[1];
	char pwbuf[SSIZ];
	char tname[2];
#endif

	starttime = time();
	copy(itoa(getpid()), pidp);
	copy("/dev/tty",devtty);
	setxcod(0);

	if (c >= 2 && (eq(av[1], "-v") || eq(av[1], "-x")))
	{
		optfv = 1;
		av[1] = av[0];
		av++;
		--c;
	}
	if (( f=dup(1)) != 2)
		close(f);	/* force open fd 2 if not open already */
	f = c-2;
	f = f<0 ? 0 : f;
#ifdef	AUSAM
	uid = getefvt();
	pe->pw_uid = uid;
	if(getpwlog(pe, pwbuf, sizeof pwbuf) < 0)
	{
		static char *unknown = "unknown";

		pe->pw_strings[LNAME] = unknown;
		pe->pw_strings[DIRPATH] = unknown;
	}
	pwclose();
	tname[0] = ttyn(2); tname[1] = 0;
#define	logname()	pe->pw_strings[LNAME]
#define	logdir()	pe->pw_strings[DIRPATH]
#define	logtty()	tname

#else
	accfix();		/* fix to loginfo */
	uid = getuid();
#endif	AUSAM
	copy(itoa(f), argstr); seta[N] = argstr;
	seta[T] = logtty();
#ifdef	ACCOUNTING
	copyn(logname(), tbuf.lname, sizeof tbuf.lname);
	tbuf.shtty = devtty[8] = *seta[T];
#else
	devtty[8] = *seta[T];
#endif	ACCOUNTING
	v = av;
	p = sname(*v);
	if (eq(p,"-rsh") || eq(p,"rsh"))
				rflg++;
	if (*p == '-')
		proflag++,ttyflag++;
	for (f = 3; f < NOFILE; f++)
		close(f);
	oldfil0 = dup(0);	/* save for pipe into, .profile */
	promp = "% ";
	if(c > 1)
	{
		promp = 0;
#ifdef	ACCOUNTING
		tbuf.shtty = 'x';
#endif	ACCOUNTING
		signal(INTR, oldintr = signal(INTR, 1));
		if (*v[1]=='-')
		{
			*p = '-';
			if (v[1][1]=='c' && c>2)
			{
				arginp = v[2];
				verbflg++;
			}
			else if (v[1][1]=='t')
			{
				onelflg = 2;
				verbflg++;
			}
		}
		else
		{
			close(0);
			f = open(v[1], 0);
			if(f < 0)
			{
				prs(CANTOP);
				err(v[1]);
				exit(1);
			}
			bnread = BSIZFIL;
		}
	}

	if (onelflg || arginp || (seek(0, 0, 1) == -1 && errno == ESPIPE))
		bnread = 1;	/* no lookahead, no buffering */
	else
		bbuf = sbrk(bnread);
	endptr = endcore = sbrk(0);

	seta[S] = logdir();
	setwhere();
	if (promp)
	{
		signal(15, timeout);
		if(uid == 0)
			promp = "# ";
	}
	pexinit();
#ifdef	ACCOUNTING
	initacct();
#endif	ACCOUNTING
	if (proflag)
	{
		if (uid != 0 && (f = open(".profile.",0)) >= 0)
		{
			close(0);
			dup(f);
			close(f);
			promp = 0;
			signal(INTR, catchintr);
		}
		else
			proflag = 0;
	}
	if(*p == '-' && !proflag)
	{
		setintr++;
		signal(QUIT, 1);
		signal(INTR, 1);
	}
	dolv = v+1;
	dolc = c-1;

loop:
	if (promp)
		prints(1, optpromp ? optpromp : promp);
	main1();
	if(wasintr)
	{
		wasintr = 0;
		if(eq(gointr, "-")) goto loop;
		if (proflag > 1)
		{
			b.nleft = 0;
			goto loop;	/* will cause faked `next' */
		}
		wtop = WDEEP;		/* clear out while stack */
		ARG0 = ONINTR;
		ARG1 = gointr;
		search(COMTYPE = ZGOTO, 0);
		setxcod(1);
		signal(INTR, 0);
	}
	goto loop;
}


char line[LINSIZ];
char	lastline[LINSIZ];
char	*lastargs[ARGSIZ];

main1()
{
	char *args[ARGSIZ];
	int trebuf[TRESIZ];
	register char c, *cp;
	register *t;

	argp = args;
	eargp = args+ARGSIZ-5;
	linep = line;
	elinep = line+LINSIZ-5;
	error = 0;
	gflg = 0;
	b.linloc = BNOW;	/* find where we are, in case while */
	COMTYPE = -1;	/* avoid bad diagnostic from readc */
	do
	{
		cp = linep;
		word();
		if (optfv)
		{
			prints(1, " ");
			prints(1, cp);
		}
	} while(*cp != '\n');

	setlast(args);
	treep = trebuf;
	treeend = &trebuf[TRESIZ];
	if(gflg == 0)
	{
		if(error == 0)
		{
			setexit();
			if (error)
				return;
			t = syntax(args, argp);
		}
		if(error != 0) 
		{
			err("syntax error"); 
		}
		else
		{
			redirf = 0;	/* no redirect of fildes 0 yet */
			execute(t);
			bsynch(0);	/* adjust internal to real */
		}
	}
}
setlast(args)
char	**args;
{
	register char	**pp;
	register char	*cp, *cp1;
	char		**pp1;

	if(argp == args+1)
		return;
	pp1 = lastargs;		cp1 = lastline;
	for( pp = args; pp < argp; pp++)
	{
		*pp1++ = cp1;
		for(cp = *pp; *cp1++ = *cp++; );
	}
	prevc = argp - args - 1;	/* because of the newline */
}



word()
{
	register char c, c1;
	register dolflag;

	*argp++ = linep;

loop:
	switch(c = getc(DOLREPL))
	{

	case ' ':
	case '\t':
		goto loop;

	case '\'':	/* '...' : what you see is what you get */
	case '"':	/* "..." : \", \$, $ substitution */
		c1 = c;
		dolflag = (c == '"' && !dolp) ? DOLREPQ : !DOLREPL;
		while((c=getc(dolflag)) != c1)
		{
			if(c == '\n')
			{
				error++;
				peekc = c;
				return;
			}
			if (c1 == '"' && c == '\\' &&
				((peekc = getc(!DOLREPL)) == '$' ||
				peekc == '"'))
				{
					c = peekc;
					peekc = 0;
			}
			*linep++ = c|QUOTE;
		}
		goto pack;

	case '&':
	case '|':
		*linep++ = c;
		if((peekc=getc(DOLREPL)) == c)
			peekc = 0;
		else
			linep--;
	case ';':
	case '<':
	case '>':
	case '(':
	case ')':
	case '^':
	case '\n':
		*linep++ = c;
		*linep++ = '\0';
		return;
	case '\\':
		if ((c=getc(!DOLREPL))=='\n') goto loop;
		else
		{
			c =| QUOTE;
			break;
		}
	}

	peekc = c;

pack:
	for(;;)
	{
		if ((c = getc(DOLREPL))=='\\')
		{
			if ((c=getc(!DOLREPL))=='\n') c = ' ';
			else c =| QUOTE;
		}
		if(any(c, " '\"\t;&<>()|^\n"))
		{
			peekc = c;
			if(any(c, "\"'"))
				goto loop;
			*linep++ = '\0';
			return;
		}
		*linep++ = c;
	}
}


tree(n)
int n;
{
	register *t;

	t = treep;
	treep =+ n;
	if (treep>treeend)
	{
		prs("Command line overflow\n");
		error++;
		reset();
	}
	return(t);
}

char	subchar	'$';	/* variable marker, may be changed by pump */

/*	flag: !DOLREPL ==> no substitution, DOLREPL ==> substitute,
	DOLREPQ ==> quoted substitution: "$1" = value of $1 for sure */
getc(flag)
register flag;
{
	register char c;

	if(peekc)
	{
		c = peekc;
		peekc = 0;
		return(c);
	}
	if(argp > eargp)
	{
		argp =- 10;
		while((c=getc(!DOLREPL)) != '\n');
		argp =+ 10;
		err("Too many args");
		gflg++;
		return(c);
	}
	if(linep > elinep)
	{
		linep =- 10;
		while((c=getc(!DOLREPL)) != '\n');
		linep =+ 10;
		err("Too many characters");
		gflg++;
		return(c);
	}
getd:
	if(dolp)
	{
		if (c = *dolp++)
		{
			if (flag == DOLREPQ)
				c =| QUOTE;
			return c;
		}
		if (idolp != -1 && ++idolp < nwords)
		{
			dolp = wordp[idolp];
			return(' ');
		}
		dolp = 0;
		idolp = -1;
	}
	if(secondpeek)
	{
		c = secondpeek;
		secondpeek = 0;
		return c;
	}
	c = readc();
	if(c == subchar && flag)
	{
		c = readc();
		if(c>='0' && c<='9')
		{
			if(c-'0' < dolc)
				dolp = dolv[c-'0'];
			goto getd;
		}
		else if(c>='a' && c<='z')
		{
			dolp = seta[c-'a'];
			goto getd;
		}
		else if(c == '^')
			/* $^ is whole of previous line,
			 * $^n is nth "word" of previous line,
			 * $$^n is words from nth on of the previous line.
			 * Note the ambiguity of $^ or $^0, which requires
			 * a second peek character
			 */
		{
			c = readc();
			if(c >= '0' && c <= '9')
			{
				if((c =- '0') < prevc)
					dolp = lastargs[c];
				goto getd;
			}
			else
			{
				secondpeek = c;     /* remember for after */
				wordp = lastargs;
				idolp = 0;
				nwords = prevc;
				dolp = *lastargs;
				goto getd;
			}
		}
		else if(c == subchar)
		{
			c = readc();
			if(c >= '0' && c <= '9')	/* $$n = $n $n+1 .... */
			{
				if((c =- '0') < dolc)
				{
					idolp = c;
					dolp = dolv[c];
					nwords = dolc;
					wordp = dolv;
				}
				goto getd;
			}
			else if(c == '^')
			{
				c = readc();
				if(c < '0' || c > '9') /* same as $^ */
				{
					secondpeek = c;
					c = '0';
				}
				if((c =- '0') < prevc)
				{
					idolp = c;
					nwords = prevc;
					dolp = lastargs[c];
					wordp = lastargs;
				}
				goto getd;
			}
			else if(c == subchar)	/* $$$ is pid */
			{
				dolp = pidp;
				goto getd;
			}
			else if(c != '\n')
				c = readc();
		}
		/* $* = $1 $2 .... */
		else if (c == '*')
		{
			if (dolc > 1)
			{
				idolp = 1;
				nwords = dolc;
				wordp = dolv;
				dolp = dolv[1];
			}
			goto getd;
		}
		else if(c != '\n')
			c = readc();
	}
	return(c&0177);
}



/*
 * syntax
 *	empty
 *	syn1
 */

syntax(p1, p2)
register char **p1, **p2;
{

	while(p1 != p2)
	{
		if(any(**p1, ";&\n"))
			p1++;
		else
			return(syn1(p1, p2));
	}
	return(0);
}


/*
 * syn1
 *	syn1a
 *	syn1a & syntax
 *	syn1a ; syntax
 */

syn1(p1, p2)
char **p1, **p2;
{
	register char **p;
	register *t, *t1;
	int l;

	l = 0;
	for(p=p1; p!=p2; p++)
	switch(**p)
	{

	case '(':
		l++;
		continue;

	case ')':
		l--;
		if(l < 0)
			error++;
		continue;

	case '&':
		if((*p)[1] == '&')
			continue;	/* and, not asynch */
	case ';':
	case '\n':
		if(l == 0)
		{
			l = **p;
			t = tree(4);
			t[DTYP] = TLST;
			t[DLEF] = syn1a(p1, p);
			t[DFLG] = 0;
			if(l == '&')
			{
				t1 = t[DLEF];
				t1[DFLG] =| FAND|FPRS|FINT;
			}
			t[DRIT] = syntax(p+1, p2);
			return(t);
		}
	}
	if(l == 0)
		return(syn1a(p1, p2));
	error++;
}


/*
 * syn1a
 *	syn1b
 *	syn1b || syn1a
 */

syn1a(p1,p2)
char **p1, **p2;
{
	register char **p;
	register int l, *t;

	l = 0;
	for(p=p1; p!=p2; p++)
	switch(**p)
	{

	case '(':
		l++;
		continue;

	case ')':
		l--;
		continue;

	case '|':
		if((*p)[1] == '\0')
			continue;	/* a pipe not an or */
		if(l == 0)
		{
			t = tree(4);
			t[DTYP] = TOR;
			t[DLEF] = syn1b(p1, p);
			t[DRIT] = syn1a(p+1, p2);
			t[DFLG] = 0;
			return(t);
		}
	}
	return(syn1b(p1, p2));
}


/*
 * syn1b
 *	syn2
 *	syn2 && syn1b
 */

syn1b(p1,p2)
char **p1, **p2;
{
	register char **p;
	register int l, *t;

	l = 0;
	for(p=p1; p!=p2; p++)
	switch(**p)
	{

	case '(':
		l++;
		continue;

	case ')':
		l--;
		continue;

	case '&':
		if(l == 0)
		{
			t = tree(4);
			t[DTYP] = TAND;
			t[DLEF] = syn2(p1, p);
			t[DRIT] = syn1b(p+1, p2);
			t[DFLG] = 0;
			return(t);
		}
	}
	return(syn2(p1, p2));
}


/*
 * syn2
 *	syn3
 *	syn3 | syn2
 */

syn2(p1, p2)
char **p1, **p2;
{
	register char **p;
	register int l, *t;

	l = 0;
	for(p=p1; p!=p2; p++)
	switch(**p)
	{

	case '(':
		l++;
		continue;

	case ')':
		l--;
		continue;

	case '|':
	case '^':
		if(l == 0)
		{
			t = tree(4);
			t[DTYP] = TFIL;
			t[DLEF] = syn3(p1, p);
			t[DRIT] = syn2(p+1, p2);
			t[DFLG] = 0;
			return(t);
		}
	}
	return(syn3(p1, p2));
}


/*
 * syn3
 *	( syn1 ) [ < in  ] [ > out ]
 *	word word* [ < in ] [ > out ]
 */

syn3(p1, p2)
char **p1, **p2;
{
	register char **p;
	char **lp, **rp;
	register *t;
	int n, l, i, o, c, flg;

	flg = 0;
	if(**p2 == ')')
		flg =| FPAR;
	lp = 0;
	rp = 0;
	i = 0;
	o = 0;
	n = 0;
	l = 0;
	for(p=p1; p!=p2; p++)
	switch(c = **p)
	{

	case '(':
		if(l == 0)
		{
			if(lp != 0)
				error++;
			lp = p+1;
		}
		l++;
		continue;

	case ')':
		l--;
		if(l == 0)
			rp = p;
		continue;

	case '>':
		p++;
		if (p != p2 && **p == '>')
		{
			if (l == 0)
				flg =| FCAT;
		}
		else p--;

	case '<':
		if(l == 0)
		{
			p++;
			if(p == p2)
			{
				error++;
				p--;
				continue;
			}
			if(any(**p, "<>("))
			{
				error++;
				continue;
			}
			if(c == '<')
			{
				if(i != 0)
					error++;
				i = *p;
				continue;
			}
			if(o != 0)
				error++;
			o = *p;
		}
		continue;

	default:
		if(l == 0)
			p1[n++] = *p;
	}
	if(lp != 0)
	{
		if(n != 0)
			error++;
		t = tree(5);
		t[DTYP] = TPAR;
		t[DSPR] = syn1(lp, rp);
	}
	else
	{
		if(n == 0)
			error++;
		p1[n++] = 0;
		t = tree(n+5);
		t[DTYP] = TCOM;
		for(l=0; l<n; l++)
			t[l+DCOM] = p1[l];
	}
	t[DFLG] = flg;
	t[DLEF] = i;
	t[DRIT] = o;
	return(t);
}


scan(at, f)
int *at;
int (*f)();
{
	register char *p;
	register *t;

	t = at+DCOM;
	while(p = *t++)
	{
		if(*p == '%' || *p == '@')
			gflg = 1;
		(*f)(p);
	}
}


tglob(s)
char *s;
{
	register char *p, c;

	for (p=s; c = *p++;)
		if(any(c, "[?*"))
			gflg = 1;
}


trim(s)
char *s;
{
	register char *p;

	for (p=s; *p++ =& 0177; );
	return(s);
}
int	ap, ac;		/* arg pointer & count for if & related cmds */
char	**av;		/* av[0] = t[DCOM] */
int	*savdlef;	/* for cmd piped into, has &t for cmd on other end */

execute(t, pf1, pf2)
int *t, *pf1, *pf2;
{
	int i, f, pv[2], wt;
	register *t1;
	register char *cp1, *cp2;
	int unnext();

tryagain:	/* if expr command may come back here to do command */
	if(t != 0)
	switch(t[DTYP])
	{

	case TCOM:
		ARG0 = cp1 = t[DCOM];
		ARG1 = cp2 = t[DCOM+1];
		if((COMTYPE = lookup(cp1)) < 0)
			goto notinternal;
#ifdef	ACCOUNTING
		if (acctfi)
		{
			times(&timeb);
			timeb.curtim = time();
		}
#endif	ACCOUNTING
		setxcod(0);	/* assume will be good */
		av = &t[DCOM];
		ap = 1;
		for(ac = 1; av[ac]; ac++);
		if (cp2)
			trim(cp2);
		if (COMTYPE != ZEQUALS)	/* = takes care of self,can't do now */
			bsynch(0);	/* make sure internal ptr right */
		switch (COMTYPE)
		{
		case ZCHDIR:
		case ZCD:
			if (rflg)
			{
				die(NOTPER, 0);
				break;
			}
			if(cp2 == 0)
			{
				if( chdir(seta[S]) < 0)
					die("Can't find home directory", 0);
			}
			else
			{
				gflg = 0;
				scan(t, &tglob);
				if(gflg)
				{
					etcglob(t + DCOM, 1);
				}
				else
				{
					if(chdir(cp2) < 0)
						die(BADDIR, 0);
				}
			}
			break;

		case ZSHIFT:
			if(dolc < 1)
			{
				die(ARGCNT, 0);
				break;
			}
			if(cp2==0)
			{
				dolv[1] = dolv[0];
				dolv++;
				dolc--;
				break;
			}
			if((i = atoi(cp2)) < 0 || i > dolc)
			{
				die(BADARG, cp2);
				break;
			}
			for(; i < dolc; i++)
				dolv[i] = dolv[i+1];
			dolc--;
			break;

		case ZWAIT:
			if(cp2)
			{
				i = atoi(cp2);
				if(i == 0)
					return;
			}
			else
			{
				i = -1;
			}
			if(setintr)
				signal(INTR,unnext);
			pwait(i,0);
			if(setintr)
				signal(INTR,1);
			return;
			break;

		case ZLABEL:
		case ZENDIF:
		case ZENDSW:
			break;

		case ZONINTR:
			/* suppress onintr if interactive shell, or if
			 * noninteractive fired up immune to INTR.
			 */
			if (promp || oldintr)
				break;
			if(!cp2)
			{
				signal(INTR, 0);
				break;
			}
			copy(cp2, gointr);
			wasintr = 0;
			signal(INTR, catchintr);
			break;

		case ZEQUALS:
			if(t[DFLG]&FPIN) close(pf1[1]);
			i = *cp2 - 'a';
			if(t[DCOM+3] != 0 && eq(t[DCOM+2], ""))
			{
				t[DCOM+2] = t[DCOM+3];
				setxcod(1);
			}
				/* 3rd exists & null 2nd ==> use 3rd instead */
			if(i>25 || i<0 || (i==P && rflg))
				err(EQERR);
			else
			{
				f = t[DFLG]&FPIN;
				if ((seta[i] = rdval((f ? pf1[0] : 0),
					t[DLEF], t[DCOM+2]))==0)
					err(EQERR);
				if (f)	/* piped, must assure synch */
				{
					pwait(savdlef[DSPR], savdlef);
					bsynch(2);
				}
			}
			break;

		case ZNEXT:
			opromp = promp;
			if(!cp2)
			{
				f = dup(oldfil0);
				promp = uid? "% ": "# ";
				bnread = BSIZ;
				proflag = 0;
			}
			else
			{
				if (rflg)
				{
					die(NOTPER, 0);
					break;
				}
				promp = 0;
				f = open(cp2, 0);
			}
			if (f<0)
			{
				promp = opromp;
				die(CANTOP, cp2);
			}
			else
			{
				close(0);
				dup(f);
				close(f);
				signal(INTR, promp?1:0);
				signal(QUIT, promp?1:0);
				setintr = promp?1:0;
				signal(15, promp?timeout:0);
			}
			b.nleft = b.gotten = 0;
			b.start = 0;
			for (wtop = 0; INWHILE; wtop++)
			{
				wstk[wtop].sloc = 0;
				wstk[wtop].eloc = 0;
			}
			break;

		case ZEXIT:
			bflush();
			setxcod(cp2 ? atoi(cp2) : 0);
			break;

		case ZGOTO:
			/* following takes care of (unlikely) case in which
			 * goto exits from loop(s) whose end(s) are as
			 * yet unlocated. although not efficient, it is
			 * rigged to act in the intuitively proper way.
			 */
			i = wtop;
			while (INWHILE)
			{
				if (wstk[wtop].eloc == 0)
				{
					search(ZBREAK, 0);	/* find end */
					wstk[wtop].eloc = BNOW;
				}
				else bseek(wstk[wtop].eloc);
				wtop++;
			}
			wtop = i;

			search(ZGOTO, 0);

			b.linloc = BNOW;	/* effect of goto on while */
			while (INWHILE &&
				(b.linloc < wstk[wtop].sloc ||
				b.linloc > wstk[wtop].eloc))
				wtop++;	/* pop */
			break;

		case ZELSE:
			search(ZELSE, eq(cp2, IF) && eq(av[ac-1], THEN));
			break;

		case ZBREAK:
			if (INWHILE)
				toend();
			else die(NOTWHI, 0);
			break;

		case ZBREAKSW:
			ARG1 = 0;	/* ignore possible arg */

		case ZSWITCH:
			search(COMTYPE, 0);	/* ZBREAKSW or ZSWITCH */
			break;

#ifndef	AUSAM
		case ZTEST:
			i = exp();
			if(nxtarg() != 0)
				die(SYNTAX, av[ap-1]);
			else
				setxcod(!i);
			break;
#endif

		case ZIF:
			if (exp())
			{
				if (eq(nxtarg(), THEN))
					break;
				--ap;	/*	ap -> new cmd */
				/* shift args to eliminate if */
				for(i = DSPR; i >= 0; i--)
					av[--ap] = t[i];
				t = &av[ap];
#ifdef	ACCOUNTING
				enacct(cp1, 1);	/* acct for if part */
#endif	ACCOUNTING
				goto tryagain;
			}
			else if (eq(nxtarg(), THEN))
				search(ZIF, 0);
			break;

		case ZWHILE:
			if (ac > 2) /* expr like that of if */
			{
				i = exp();
				if (cp1 = nxtarg())
				{
					die(SYNTAX, cp1);
					break;
				}
			}
			else /* 0 args or 1 null arg = false, else true*/
				i = cp2 != 0 && *cp2 != '\0';
			if (wtop >= WDEEP || wstk[wtop].sloc != b.linloc)
			{
				/* not already in loop */
				if (--wtop >= 0)
				{
					if (wstk[wtop].sloc != b.linloc)
					{
						wstk[wtop].sloc = b.linloc;
						wstk[wtop].eloc = 0;
					}
				}
				else
				{
					die(">3 levels", 0);
					break;
				}
			}

			if (i)
				break;
			toend();	/* condition nomatch */
			break;
		case ZCONTINUE:
			if (INWHILE)
				bseek(wstk[wtop].sloc);
			else die(NOTWHI, 0);
			break;

		case ZEND:
			if (INWHILE) /* while active */
			{
				wstk[wtop].eloc = BNOW;
				bseek(wstk[wtop].sloc);
			}
			else die(NOTWHI, 0);
			break;

		case ZOPT:
			for(i = DCOM+1; cp2 = t[i]; i++)
			{
				if (eq(cp2, "-v") || eq(cp2, "-x"))
					optfv = 1;
				else if (eq(cp2, "+v") || eq(cp2, "+x"))
					optfv = 0;
				else if (eq(cp2, "-p"))
				{
					if (cp2 = t[++i])
					{
						optpromp = rdval(0, 0, cp2);
						if(any('#', optpromp) && uid)
							optpromp = 0;
					}
					else
						optpromp = 0;
				}
			}
			break;

		case ZTIMES:
			finish();
			break;

		case ZECHOALL:
			optfv = 1;
			break;

		case ZECHONONE:
			optfv = 0;
			break;

#ifdef	AUSAM
		case ZKILL:
			killer(t + DCOM + 1);
			break;
#else

		case ZLOGIN:
			if(promp != 0) {
				fclean();
				execv("/bin/login", t+DCOM);
			}
			die(CANTEX, 0);
			break;
#endif	AUSAM
		}
#ifdef	ACCOUNTING
		enacct(cp1, 1);		/* internal commands */
#endif	ACCOUNTING
		return;

	notinternal:
	case TPAR:
		bsynch(1);	/* set real ptr = BNOW */
		f = t[DFLG];
		i = 0;
		if((f&FPAR) == 0)
			if ((i = dofork()) == -1)
				return;	/* couldn't fork */
		if(i != 0) /* parent */
		{
			if((f&FPIN) != 0)
			{
				close(pf1[0]);
				close(pf1[1]);
				t[DLEF] = savdlef;	/* link back for pwait*/
			}
			else
				t[DLEF] = 0;	/* 1st or only in pipeline */
			t[DSPR] = i;	/* save proc-num for pwait */
			if ((f & FPRS) && (promp || verbflg))
			{
				prs(itoa(i));
				prs("\n");
			}
			if ((f & (FAND | FPOU)) == 0)
				pwait(i, t);
			bsynch(2);	/* mark possible read by cmd */
			return;
		}
		/*
		 * ignore interrupts for asynchronous cmds [code needed
		 * for sh proc only; ignore also for onintr -
		 */
		if (f & FPRS)
		{
			signal(INTR, 1);
			signal(QUIT, 1);
		}
		if (eq(gointr, "-"))
			signal(INTR, 1);
		if(t[DLEF] != 0)
		{
			close(0);
			/*	following for pipe into sh */
			trim(t[DLEF]);
			if (eq(t[DLEF], "--"))
				dup(oldfil0);
			else
			{
				i = open(t[DLEF], 0);
				if(i < 0)
					xdie(CANTOP, t[DLEF]);
			}
			redirf++;
		}
		if(t[DRIT] != 0)
		{
			if (rflg)
				xdie(">: ",NOTPER);
			if((f&FCAT) != 0)
			{
				i = open(trim(t[DRIT]), 1);
				if(i >= 0)
				{
					seek(i, 0, 2);
					goto f1;
				}
			}
			i = creat(trim(t[DRIT]), 0600);
			if(i < 0)
			{
#ifdef	AUSAM
				if(errno == EDISKLIM)
				{
					xdie("Disk limit reached, cannot create: ",
						t[DRIT]);
				}
				else
#endif	AUSAM
					xdie("cannot create: ",t[DRIT]);
			}
		f1:
			close(1);
			dup(i);
			close(i);
		}
		if((f&FPIN) != 0)
		{
			close(0);
			dup(pf1[0]);
			close(pf1[0]);
			close(pf1[1]);
			redirf++;
		}
		if((f&FPOU) != 0)
		{
			close(1);
			dup(pf2[1]);
			close(pf2[0]);
			close(pf2[1]);
		}
		if((f&FAND)!=0 && t[DLEF]==0 && (f&FPIN)==0)
		{
			close(0);
			open("/dev/null", 0);
		}
		if((f&FINT) == 0 && setintr)
		{
			signal(INTR, 0);
			signal(QUIT, 0);
		}
		if(t[DTYP] == TPAR)
		{
			if(t1 = t[DSPR])
				t1[DFLG] =| f&FINT;
			execute(t1);
			exit(exitcode);
		}
		fclean();

		if (rflg && any('/', trim(t[DCOM])))
			xdie(NOTPER, 0);	/* no / in rsh commands */
		gflg = 0;
		scan(t, &tglob);
		if (gflg)
		{
			/* old glob stuff
			t[DSPR] = seta[P];
			execv("/etc/glob", t+DSPR);
			xdie("glob: ",CANTEX);
			*/
			etcglob(t + DCOM, 0);
		}
		scan(t, &trim);

		if (eq(cp1, "pump"))
			dopump(&t[DCOM+1], redirf);
		texec(t[DCOM], t);

	case TFIL:
		f = t[DFLG];
		pipe(pv);
		t1 = t[DLEF];
		t1[DFLG] =| FPOU | (f&(FPIN|FINT|FPRS));
		execute(t1, pf1, pv);
		t1 = t[DRIT];
		t1[DFLG] =| FPIN | (f&(FPOU|FINT|FAND|FPRS));
		savdlef = t[DLEF];	/* save so can link pipe together */
		execute(t1, pv, pf2);
		return;

	case TLST:
		f = t[DFLG]&FINT;
		if(t1 = t[DLEF])
			t1[DFLG] =| f;
		execute(t1);
		if(t1 = t[DRIT])
			t1[DFLG] =| f;
		execute(t1);
		return;


	case TOR:
	case TAND:
		f = t[DFLG]&FINT;
		t1 = t[DLEF];
		t1[DFLG] =| f;
		execute(t1);
		i = atoi(seta[R]);
		if ((i == 0) == (t[DTYP] == TAND))
		{
			t1 = t[DRIT];
			t1[DFLG] =| f;
			execute(t1);
		}
		return;

	}
}

toend()
{
	if (wstk[wtop].eloc == 0) /* need to find end */
	{
		ARG1 = 0;
		search(ZBREAK, 0);
	}
	else bseek(wstk[wtop].eloc);
	wtop++;		/* pop 1 level */
	return;
}


lookup(p)
register char *p;
{
	register char *q;
	register i;
	for (i = 0; q = comint[i]; i++)
		if (eq(p, q))
			return(i);
	return -1;
}

dofork()
{
	register wt, i;
	for(wt = 10;; wt =+ 10)
	{
		if ((i = fork()) != -1)
			break;
#ifdef	AUSAM
		if(errno == EPROCLIM)
		{
			err("Process limit reached");
			break;
		}
#endif	AUSAM
		if (promp == 0 && wt < 60)
			sleep(wt);
		else
		{
			err("cannot fork;try again");
			break;
		}
	}
	return i;
}

fclean()
{
#ifdef	ACCOUNTING
	if (acctf)
		close(acctf);
#endif	ACCOUNTING
	if (oldfil0)
		close(oldfil0);
	return;
}

texec(f, t)
register *t;
{
	register char *cp;
	char tline[256];
	int txtbsy;	/* kludge cntr for ETXTBSY fix */
	int pcfd;	/* file desc for suspected 'pi' obj */

	txtbsy = 0;
	cp = seta[P];	/* normal case -- search */
	if (any('/', f))
	{
		if (rflg)
			xdie(NOTPER, 0);	/* no / by rsh */
		cp = "";	/* sh: exec only cmd name as given */
	}
	do
	{
		cp = pcat(cp, f, tline, sizeof tline);
	retry:
		execv(tline, t+DCOM);
		switch (errno)
		{
		case ENOEXEC:
			t[DCOM] = tline;
			/*
			 * check for pascal object file
			 * magic number is 0404
			 */
			if((pcfd = open(t[DCOM], 0)) >= 0)
			{
				register int pcread;
				int pcmagic;

				pcread = read(pcfd, &pcmagic, 2);
				close(pcfd);
				if(pcread == 2)
				{
					if(pcmagic == 0404)
					{
						t[DSPR] = PX + 9;
						execv(PX + 4, t + DSPR);
						execv(PX    , t + DSPR);
						xdie("No px!", 0);
					}
				}
			}
			/*
			 * Must be a command file
			 */
			t[DSPR] = seta[Z];
			execv(t[DSPR], t+DSPR);
			xdie(seta[Z], " No shell!");
		case EACCES:
			xdie("not executable", 0); /* missing x (probably) */
		case ENOMEM:
			xdie("too large", 0);
		case EPERM:
			xdie("permission denied", 0);
		case E2BIG:
			xdie(ARGLNG,0);
		case ETXTBSY:
			if ((txtbsy =+ 10) > 60)
				xdie("text busy", 0);
			sleep(txtbsy);
			goto retry;
		}
	} while(cp);
	xdie("not found", 0);
}

char	pipebomb;	/* 1 ==> SIGPIPE caught */
catchpipe()
{
	if (promp != 0)
		exit(1);
	pipebomb++;
	return;
}

/*	dopump: pump command:
 *	pump [+] [-[subchar]] [eofstr]
 *	default is to substitute using $
 *	-	suppresses substitution
 *	-subchar uses subchar instead of $ for substitution
 *	+	causes leading tabs in input to be thrown away
 *	eofstr defaults to !
 *	eofstr may not begin with `+', which is reserved for future flags.
 *	acts like filter, although actually copy of sh
 *	in input, \subchar = subchar, no other escaping performed
 *	if interactive, ignores interrupts & quits, but dies if other end
 *	of pipe dies; if non-interactive, reads to eofstr or real eof
 *	even if other end does die.
 *	NOTE: line is used as workarea, both for eofstr and input line
 *	eofstr is safe because initial arg can't be at beginning
 */

dopump(t)
register char **t;
{
#define	eofstr	line
#define	pumpwk	(line+96)
	register char c, *p;
	char tabeat;
	tabeat = 0;
	if (redirf)
	{
		b.nleft = b.gotten = 0;	/* clear buffer values */
		bnread = 1;	/* be careful with pipes & files */
	}
	else
	{
		if (promp == 0)
			bsynch(0);	/* use existing b., if any */
		else
		{
			signal(INTR, 1);	/* interactive - ignore */
			signal(QUIT, 1);	/* for sake of pump -|ed */
		}
	}
	if (proflag)
	{
		signal(INTR, 0);
		proflag = 0;
	}
	signal(SIGPIPE, catchpipe);
	setxcod(0);		/* in case real eof */
	copy("!", eofstr);
	for(; *t; t++)
		switch (**t)
		{
		case '-':
			subchar = *(*t + 1);	/* \0 turns subst off */
			break;
		case '+':
			tabeat++;
			break;
		default:
			copy(*t, eofstr);
		}

	while(1)	/* real eof will cause exit in readc */
	{
		p = pumpwk;
		if (tabeat)
		{
			while((c = getc(DOLREPL)) == '\t');
			peekc = c;
		}
		do
		{
			c = getc(DOLREPL);
			if (c == '\\' && (peekc = getc(!DOLREPL)) == subchar)
			{
				c = peekc;
				peekc = 0;
			}
			*p++ = c;
		} while (c != '\n');
		*--p = 0;		/* put 0 for \n */
		if (eq(pumpwk, eofstr))
			break;
		*p = '\n';
		if (!pipebomb)
			write(1, pumpwk, p - pumpwk + 1);
	}
	bsynch(1);	/* in case file */
	exit(pipebomb);
}

atoi(s)
char *s;
{

	register char *sp;
	register i, neg;

	sp = s;
	i = neg = 0;

	if(*sp == '-')
	{
		++neg;
		++sp;
	}

	while(*sp)
	{
		if(*sp < '0' || *sp > '9')
		{
			die(NONUM, s);
			return 0;
		}
		i = i * 10 + (*sp - '0');
		++sp;
	}
	return neg? -i: i;
}

xdie(str1, str2)
char *str1, *str2;
{
	die(str1, str2);
	exit(1);
}

die(str1, str2)
char *str1, *str2;
{
	prs(ARG0);
	prs(": ");
	prs(str1);
	err(str2);
	return;
}

/*
 *	err: emit error message, flush input by seeking to EOF (but only
 *	if reading from 0 in unrestricted way), exit.
 */
err(s)
char *s;
{

	prs(s);
	prs("\n");
	if (onelflg == 0 && arginp == 0)
		bflush();
	setxcod(1);
}


prs(s)
register char *s;
{

	if (s == 0)
		return;
	prints(2, s);
}


putc(c)
{

	write(2, &c, 1);
}


any(c, s)
register char c, *s;
{

	while(*s)
		if(*s++ == c)
			return(1);
	return(0);
}


eq(s1, s2)
register char *s1, *s2;
{

	if (s1 == 0 || s2 == 0)
		return 0;
	while(*s1++ == *s2)
		if(*s2++ == '\0')
			return(1);
	return(0);
}


#ifdef	ACCOUNTING
/*	initacct: initialize acctg according to state of ACNAME file:
	if cannot open ACNAME, no acctg done (acctf == 0)
	group permissions of ACNAME are used otherwise:
	0	cmd level, externals only	(acctf > 0, acctfi == 0)
	4	cmd level, add internal cmds	(acctf > 0, acctfi > 0)
	2	shell proc, externals only	(acctf > 0, acctfi == 0)
	1	shell proc, add internal cmds	(acctf > 0, acctfi > 0)
	NOTE: THIS IS KLUDGE INTENDED FOR OBLIVION AFTER EITHER NEW
	MH ACCTG GETS INSTALLED AND/OR INTERRUPT-HANDLING CHANGE
	PERMITS BETTER INTERNAL CMD HANDLING.
 */

initacct()
{
	register f;
	if ((acctf = open(ACNAME, 1)) < 0)
		acctf = 0;	/* no acctg at all */
	else
	{
		fstat(acctf, &sb);
		f = sb.i_mode;
		if (tbuf.shtty == 'x')	/* shell proc */
		{
			if (f & 020)
			{
				if (f & 010)
					acctfi++;	/* ext + int */
			}
			else
			{
				close(acctf);	/* no acct at all */
				acctf = 0;
			}
		}
		else if (f & 040)
			acctfi++;	/* cmd level, ext + int */
	}
	return;
}
#endif	ACCOUNTING

pwait(i, t)
int i, *t;
{
	register p, e, *t1;
	int nprocs;
	int s;

	nprocs = 0;
	t1 = t;
	while(t1)	/* count number of procs in current pipeline */
	{
		if (t1[DSPR])
			nprocs++;
		t1 = t1[DLEF];
	}

	if(i != 0)
	do
	{
#ifdef	ACCOUNTING
		if (acctf)
		{
			times(&timeb);
			timeb.curtim = time();
		}
#endif	ACCOUNTING
		p = waitx(&s);
		if(wasintr) p = waitx(&s);
		if(p == -1)
			break;
		e = s&0177;
		exitcode = (s>>8)&0377;
		if(e==INTR && wasintr)  /* process returned intr */
			e = 0;
		if(mesg[e] != 0)
		{
			if(p != i)
			{
				prs(itoa(p));
				prs(": ");
			}
			prs(mesg[e]);
			if(s&0200)
				prs(" -- Core dumped");
		}
		if (p == i)
			setxcod(exitcode);
		if(e != 0)
			err("");
		for(t1 = t; t1; t1 = t1[DLEF]) /* hunt proc; elims most **gok*/
		{
			if (t1[DSPR] == p)
			{
				nprocs--;
				break;
			}
		}
#ifdef	ACCOUNTING
		acct(t1);
#endif	ACCOUNTING
	} while (nprocs || (t == 0 && p != i));
}


#ifdef	ACCOUNTING
acct(t)
int *t;
{
	if(t == 0)
		enacct("**gok", 0);
	else if(*t == TPAR)
		enacct("()", 0);
	else
		enacct(t[DCOM], 0);
}


enacct(as, acctype)
char *as;
int acctype;	/* 0 ==> child process, 1 ==> internal cmd */
{
	struct stime timbuf;
	register i;
	register char *np;

	if (uid == 0)
		return;
	if (acctf <= 0)	/* if no accounting active, elim sys calls */
		return;
	if (acctype && acctfi <= 0)
		return;	/* internal cmd, but no internal acctg */
	times(&timbuf);
	timbuf.curtim = time();
	tbuf.realt = timbuf.curtim - timeb.curtim;
	if (acctype)
	{
		tbuf.bcput = timbuf.procu - timeb.procu;
		tbuf.bsyst = timbuf.procs - timeb.procs;
	}
	else
	{
		tbuf.bcput = timbuf.childu - timeb.childu;
		tbuf.bsyst = timbuf.childs - timeb.childs;
	}
	np = sname(as);
	for (i=0; i<8; i++)
	{
		tbuf.cname[i] = *np;
		if (*np)
			np++;
	}
	tbuf.datet = timbuf.curtim;
	tbuf.tuid = uid;
	seek(acctf, 0, 2);
	write(acctf, &tbuf, sizeof(tbuf));
}
#endif	ACCOUNTING


rdval(pipef, lef, na)
char *na;
{
	register char *st, *np;
	char c;

	st = endptr;
	np = na;
	if(!pipef && lef)
	{
		pipef = eq(lef, "--") ? dup(oldfil0) : open(lef, 0);
		if(pipef<0)  return 0;
	}
	for(;;)
	{
		if(endptr >= endcore-10)
			if((endcore=sbrk(64))<0) return 0;
		if(!na)
		{
			if(read(pipef, &c, 1) <= 0)
			{
				setxcod(1);	/* EOF indicator */
				break;
			}
		}
		else c = *np++ & 0177;
		*endptr++ = c;
		if(c=='\n' || c=='\0') break;
	}
	if(c=='\n') --endptr;
	*endptr++ = '\0';
	if(pipef || lef) close(pipef);
	return st;
}


catchintr()
{
	if (proflag)
		proflag++;	/* in .profile, make sure come out ok */
	wasintr++;
	signal(INTR, 1);
}


pcat(so1, so2, si, sz)
register char *so1, *so2;
char *si;
int sz;
{
	register char *s;

	s = si;
	while(*so1 != ':' && *so1 != '\0' && --sz) *s++ = *so1++;
	if(si != s && --sz > 0) *s++ = '/';
	while(*so2 && --sz > 0) *s++ = *so2++;
	if (--sz < 0)
	{
			*si = '\0';
			die("cmd line overflow", 0);
	}
	else *s = '\0';
	return *so1 ? ++so1 : 0;
}


setxcod(code)
int code;
{
	copy(itoa(code), exitstr);
	seta[R] = exitstr;
	return;
}

copy(source, sink)
register char *source, *sink;
{
	 while(*sink++ = *source++ & 0177);
}

/*
 *	copyn: copy at most n bytes from source to sink.
 */
copyn(source, sink, n)
register char *source, *sink;
int	n;
{
	register i;
	for (i = 0; i < n; i++)
		if (!(*sink++ = *source++))
			break;
}

itoa(n)
{
	register i, j;
	register char *cp;
	static char *str[12];

	j = n;
	for(cp = &str[4]; ;--cp)
	{
		if(wide) --wide;
		j = ldiv(0, j, 10);
		*cp = ldivr+'0';
		if((j | wide) == 0)
			return cp;
	}
}

char *nxtarg()
{
	register iap;

	if ((iap = ap++) > ac || av[iap] == 0)
		return(0);
	return trim(av[iap]);
}

exp()
{
	int p1;

	p1 = e1();
	if (eq(nxtarg(), "-o")) return(p1 | exp());
	ap--;
	return(p1);
}

e1()
{
	int p1;

	p1 = e2();
	if (eq(nxtarg(), "-a")) return (p1 & e1());
	ap--;
	return(p1);
}

e2()
{
	if (eq(nxtarg(), "!"))
		return(!e3());
	ap--;
	return(e3());
}

e3()
{
	int ccode;
	int nap;
	int int1, int2;
	register char *a, *p1, *p2;

	ccode = 0;
	a = nxtarg();
	if(eq(a, "("))
	{
		ccode = exp();
		if(!eq(nxtarg(), ")")) goto erre3;
		return(ccode);
	}

	if(eq(a, "{")) /* execute a command for exit code */
	{
		nap = ap;	/*	save 1st arg ptr */
		while (ap < ac && !eq(av[ap], "}")) trim(av[ap++]);
		if (ap >= ac || nap == ap)
		{
			die(SYNTAX, av[nap]);
			return 0;
		}
		av[ap++] = 0;	/*	change } to 0 for pexec */
		if ((int1 = dofork()) > 0)
			waitx(&ccode);	/* parent */
		else if (int1 == 0)	/* child */
			texec(av[nap], &av[nap-DCOM]);
		return(ccode? 0 : 1);
	}

	p1 = nxtarg();

	/* file predicates */
	if(eq(a, "-r"))
		return(tio(p1, 0));

	if(eq(a, "-w"))
		return(tio(p1, 1));
	if (eq(a, "-f"))
	{
		if (stat(p1, &sb) == -1)
			return 0;
		return ((sb.i_mode & IFMT) == 0);
	}
	if (eq(a, "-d"))
	{
		if (stat(p1, &sb) == -1)
			return 0;
		return ((sb.i_mode & IFMT) == IFDIR);
	}
	if (eq(a, "-s"))
	{
		if (stat(p1, &sb) == -1)
			return 0;
		return (sb.i_size0 || sb.i_size1);
	}

	/* string predicates */
	if (eq(a, "-n"))
		return(p1 && *p1 != '\0');
	if (eq(a, "-z"))
		return(p1 == 0 || *p1 == '\0');
	if ((p2 = nxtarg()) == 0)
		goto erre3;
	if(eq(p1, "="))
		return(eq(a, p2));

	if(eq(p1, "!="))
		return(!eq(a, p2));

	int1 = atoi(a);
	int2 = atoi(p2);
	if(eq(p1, "-eq"))
		return( int1 == int2 );
	if(eq(p1, "-ne"))
		return( int1 != int2 );
	if(eq(p1, "-gt"))
		return( int1 > int2 );
	if(eq(p1, "-lt"))
		return( int1 < int2 );
	if(eq(p1, "-ge"))
		return( int1 >= int2 );
	if(eq(p1, "-le"))
		return( int1 <= int2 );

erre3:
	die(SYNTAX, p1);
}

tio(a, f)
char *a; int f;
{
	register int fil;

	if ((fil = open(a, f)) >= 0)
	{
		close(fil);
		return(1);
	}
	return(0);
}


/*	search: forward search for required token, type =
 *	ZGOTO: : label
 *	ZSWITCH: : label, : default, or unmatched ENDSW
 *	ZBREAKSW: unmatched endsw
 *	ZIF: unmatched else or endif (or }) (this is failed if-then)
 *	ZELSE: unmatched endif (or })
 *	ZBREAK: unmatched end (break, failed while or end)
 *	levinit = 0, except when called from else if ... then, when it is 1
 */
search(type, levinit)
int type, levinit;
{
	register int level, t;
	register char *aword;
	char fifel, fbr, fswex;
	char wordbuf[128];
	if (type == ZGOTO)
		bseek(0L);
	fifel = type == ZIF || type == ZELSE;
	fbr = type == ZBREAK;
	fswex = type == ZSWITCH || type == ZBREAKSW;
	aword = wordbuf;
	level = levinit;

	do
	{
		*aword = '\0';
		getword(aword);
		if ((t = lookup(aword)) >= 0)
			switch(t)
			{
			case ZELSE:
				if (level == 0 && type == ZIF)
					return;	/* leave rest of else line */
				continue;
			case ZIF:
				while(getword(aword));
				if (fifel && eq(aword, THEN))
					level++;
				break;
			case ZENDIF:
				if (fifel)
					level--;
				break;
			case ZWHILE:
				if (fbr)
					level++;
				break;
			case ZEND:
				if (fbr)
					level--;
				break;
			case ZSWITCH:
				if (fswex)
					level++;
				break;
			case ZENDSW:
				if (fswex)
					level--;
				break;
			case ZLABEL:
				if (getword(aword))
				{
					if (type == ZGOTO)
					{
						if (eq(aword, ARG1))
							level = -1;
					}
					else if (type == ZSWITCH &&
						level == 0 &&
						(eq(aword, "default") ||
						match(ARG1, aword)))
						level = -1;
				}
				break;
			}
		getword(0);	/* gobble rest of line, if any */
	} while (level >= 0 && proflag < 2);
	return;
}

/*	getword: return next word from input (if any):
	aword != 0 : get next word (if any, return 1),
			if none, return 0, do not distturb *aword.
	aword == 0 : consume rest of line thru (nl).
	return 1 if word, 0 if only newline left.
 */

getword(aword)
char *aword;
{
	register int found;	/* 1 ==> found word, 0 ==> not */
	register char c, *wp;
	wp = aword;
	found = 0;
	c = peekc ? peekc : readc();
	peekc = '\0';

	do
	{
		while(c == ' ' || c == '\t')
			c = readc();
		if (c == '\n')
		{
			if (wp)
				break;
			else
				return 0;	/* newline only thing left */
		}
		found++;
		do
		{
			if (wp)
				*wp++ = c;
			c = readc();
			if (c == '\\' && (c = readc()) == '\n')
				c = ' ';
		} while (c != ' ' && c != '\t' && c != '\n');
	} while (!wp);
	peekc = c;
	if (found)
		*wp = '\0';
	return(found);
}

readc()
{
	register c;

	if (arginp)
	{
		if (arginp == 1)
			goto ONLYEXIT;
		if ((c = *arginp++) == 0)
		{
			arginp = 1;
			c = '\n';
		}
		return(c);
	}
	if (onelflg==1)
		goto ONLYEXIT;
	if (b.nleft == 0)
	{
		b.nextp = bbuf;
		b.start = b.start + b.gotten;	/* increm by last gotten */
#ifdef	AGSM|EECF
		if(promp && *seta[T] != '8')
#else
		if(promp)
#endif	AGSM|EECF
		{
			clktim(5*60);
		}
		if ((b.nleft = read(0, b.nextp, bnread)) <= 0 || proflag > 1)
		{
			if(promp)
				clktim(0);	/* cancel timeout */
			if (proflag)	/* still inside .profile */
			{
				b.nleft = 5;	/* fake next */
				b.nextp = "next\n";
			}
			else
			{
				eoferr();
				if( promp && ttyflag )
					finish();
			ONLYEXIT:
				exit(atoi(seta[R]));
			}
		}
		else if(promp)
			clktim(0);
		b.gotten = b.nleft;
	}
	b.nleft--;
	c = *b.nextp++;
	if (c=='\n' && onelflg)
		onelflg--;
	return(c);
}

timeout()
{
	if(promp)
		signal(15,timeout);
}

/*	eoferr: issue error message if cmd was in middle of search */
eoferr()
{
	switch (COMTYPE)
	{
	case ZGOTO:
		die(MISSL, ARG1);
		break;
	case ZIF:
	case ZELSE:
		die(MISS, ENDIF);
		break;
	case ZSWITCH:
	case ZBREAKSW:
		die(MISS, ENDSW);
		break;
	case ZWHILE:
	case ZBREAK:
		die(MISS, END);
		break;
	}
	bsynch(1);
	return;
}

/*	bflush: complete input flush */
bflush()
{
	seek(0, 0, 2);
	b.nleft = b.gotten = 0;
	return;
}

/*	bsynch: synchronize internal buffering & outside world */
/*	btarg gives nominal target value of bstate */
bsynch(btarg)
register btarg;
{
	register obstate;
	long f;
	if (btarg == bstate || promp || bnread == 1 || redirf)
		return;	/* no seeking in any of these cases */
	obstate = bstate;
	bstate = btarg;
	if (btarg == 2)
		return;	/* just remember this; do nothing now */
	switch (obstate)
	{
	case 0:		/* 0 --> 1 only possible */
		if (b.nleft > 0)
			lseek(0, BNOW, 0);	/* set real = internal */
		break;
	case 1:		/* 1 --> 0 (pump begin) */
		lseek(0, BEND, 0);	/* ptr where expected for read */
		break;
	case 2:		/* 2 --> 0; 2 --> 1, leave as 2, since 1 dominates */
		if (btarg == 0)
		{
			f = tell(0);
			if (f >= 0)
				bseek(f);
		}
		else
			bstate = 2;
		break;
	}
}

/*	bseek: seek, staying within current input buffer b, if possible */
bseek(where)
long where;
{
	long bend;
	if (promp || bnread == 1 || redirf)
		return;
	bend = BEND;
	if (where >= b.start && where < bend)
	{
		b.nleft = bend - where;	/* 1 <= b.nleft <= b.gotten */
		b.nextp = bbuf + b.gotten - b.nleft;
		lseek(0, bend, 0);	/* really at end ofbuffer */
	}
	else
	{
		b.nleft = b.gotten = 0;
		b.start = where;
		lseek(0, where, 0);
	}
	return;
}

/*	sname: simple name (last component) of file name */
char *sname(s)
register char *s;
{
	register char *p;
	for(p = s; *p;)
		if (*p++ == '/' && *p != '\0')
			s = p;
	return(s);
}

/*	setwhere: set up wherev for $w (1st component of pathname) */
char	wherev[10];
setwhere()
{
	register char *s, *w;
	register i;
	s = seta[S];
	seta[W] = w = wherev;
	i = 0;
	do
	{
		*w++ = *s++;
	} while (++i < 9 && *s != '/');
}

/*
 *	pexinit: fills in pathstr (seta[P]) and shellnam (seta[Z]).
 *	may be invoked before fork to avoid unnecessary .path opening.
 *	returns 0 if OK, -1 if any error.
 */
pexinit()
{
	char pathbuf[128 + 16];
	register n, f;
	char *newpath, *newshell;
	char *p;

	newshell = "/bin/sh";
	pcat(seta[S], ".path", pathbuf, sizeof pathbuf);
	if ((f = open(pathbuf, 0)) < 0)
		newpath = uid ? ":/bin:" : "/etc/bin:/etc:/bin:";
	else
	{
		n = read(f, pathbuf, sizeof pathbuf);
		close(f);
		if (n <= 0)
		{
			prs("cannot read .path\n");
			return(-1);
		}
		if (pexline(pathbuf, pathbuf + n, 128, &newpath, &p)
		|| pexline(p, pathbuf + n, 16, &newshell, &p))
			return(-1);
	}
	seta[P] = rdval(0, 0, newpath);
	seta[Z] = rdval(0, 0, newshell);
	return(0);
}

/*
 *	pexline: scan for a line (if any) beginning at ptr,
 *	ending at ptrlim -1, for line up to psize bytes long.
 *	convert it to string, return beginning addr in pret
 *	and addr of next line in pnext
 *	return 0 if OK (or not present), -1 if runs off end in middle of line
 *	or if line too long.
 */
pexline(ptr, ptrlim, psize, pret, pnext)
register char *ptr, *ptrlim;
int psize;
char **pret, **pnext;
{
	if (ptr >= ptrlim)
		return(0);
	*pret = ptr;
	if (ptrlim > ptr + psize)
		ptrlim = ptr + psize;
	for (; ptr < ptrlim; ptr++)
		if (*ptr == '\n')
		{
			*ptr++ = '\0';
			*pnext = ptr;
			return(0);
		}
	prs(".path too long\n");
	return(-1);
}
/*
 *	sh internal version of /etc/glob
 *	(from here thru function cat)
 *
 *	"@uname" | "%uname" expands to uname's login directory
 *	"*" in params matches r.e ".*"
 *	"?" in params matches r.e. "."
 *	"[...]" in params matches character class
 *	"[...a-z...]" in params matches a through z.
 *	if param does not contain "*", "[", or "?", use it as is
 *	if it does, find all files in current directory
 *	which match the param, sort them, and use them
 */

#define	STRSIZ	600
char	**avx;
char	*string;
char	*matstr;
char	*ablimit;
int	ncoll;
int 	fin;	/* used by getchar */
int	wflg 0;	/* set if any wild char */

etcglob(argv, cdflag)
char *argv[];
{
	char	ab[STRSIZ];		/* generated characters */
	char	*avxa[200+DCOM];		/* generated arguments */
	char	pscr[512];		/* intermediate results */

	avx = &avxa[DCOM];
	string = ab;
	matstr = pscr;
	ablimit = &ab[STRSIZ];
	*avx = *argv;
	ncoll = 0;
	while (*argv)
		expand(*argv++);
	if (ncoll == 0)
	{
		gerr("No match");
		if(cdflag)
			return;
		else
			exit(1);
	}
	*avx++ = 0;
	if(cdflag)
	{
		if(chdir(avxa[DCOM + 1]) < 0)
		{
			gerr(BADDIR);
		}
		return;
	}
	else
		texec(avxa[DCOM], avxa);
}

#ifdef	AUSAM
expand(as)
char *as;
{
	register char *s, *cs;
	register int c;
	int cf;
	char *proto, **oav, *ocs;

	cs = as;
	oav = avx;
	c = *cs++;
	cf = 0;
	if((c == '@') || (c == '%'))
	{
		static char uname[30];
		static char sbuf[SSIZ];
		static struct pwent pe[1];

		s = uname;
		c = sizeof uname;
		while(--c && (*s++ = *cs) && *cs++ != '/');
		*--s = 0;
		if(*cs == 0)
			cf++;
		/* if the name has been read recently, don't re-read it */
		if( !pe->pw_strings[LNAME] || compar(uname, pe->pw_strings[LNAME]) != 0 )
		{
			pe->pw_strings[LNAME] = uname;
			c = getpwuid(pe, sbuf, SSIZ);
			pwclose();
			if(c <= 0)
			{
				gerr("Who?");
				pe->pw_strings[LNAME] = 0;
				return;
			}
		}
		if(cf)
		{
			*avx++ = cat(pe->pw_strings[DIRPATH], "");
			ncoll++;
			return;
		}
		s = matstr;
		copy(pe->pw_strings[DIRPATH], s);
		while( *s++);
		proto = --s;
		ocs = cs;
		c = cs[-1];
		cf++;
	}
	else
	{
		proto = s = matstr;
		ocs = as;
		if(c == '/')
		{
			*s++ = c;
			ocs = cs;
			c = *cs++;
			proto = s;
		}
	}
	do
	{
		if(c == '/')
		{
			proto = s;
			ocs = cs;
		}
		else if(any(c,"*?["))
		{
			*proto = 0;
			dmatch(proto - matstr, ocs);
			sort(oav);
			return;
		}
		*s++ = c;
	}
	while(c = *cs++);
	*s = 0;
	*avx++ = cat(matstr, "");
	ncoll =+ cf;
}
#else
expand(as)
char *as;
{
	register char *s, *cs;
	register int c;
	int cf, backc, wild, mflg;
	char uname[16], *proto, **oav, *oas, *ocs, *oc, *os;

	cs = as;
	oav = avx;
	cf = 0;
	mflg = 0;
	wild = 1;
	c = *cs++;

	fin = open("/etc/passwd",0);
	if((c == '@') || (c == '%'))
	{
		if(fin == -1)
		{
			prints(2, "No passwd file\n");
			exit();
		}
		while((c = *cs) && c != '/')
		{
			cs++;
			if(c == '*' || c == '?' || c == '[')
			{
				wild = 0;
				wflg = 1;
			}
		}
		if(c)
			cf++;
		*cs++ = 0;
		s = uname;
		proto = as+1;
		seek(fin, 0, 0);
		/* buffered since fin>2 */ 
		while(c = getchar())
			if(c != ':')
				*s++ = c;
			else
			{
				backc = c;
				*s = 0;
				if(amatch(uname, proto))
				{
					mflg = 1;
					s = 0;
					do
						if((c = getchar()) == ':')
							s++;
					while(s < 4 && c);
					if(!c)
						break;	/* bad passwd file */ 
					s = matstr;
					while((c = getchar()) != ':' && c != '\n' && c)
						*s++ = c;
					backc = c;
					if(!c)
						break;
					*s = 0;
					if(cf)
					{
						os = s++;
						for(oc = cs; (c = *cs++) && c != '*' && c != '?' && c != '['; *s++ = c);
						if(!c)
						{
							*os = '/';
							cf = 0;
						}
						cs = oc;
						*s = 0;
						s = os;
					}
					if(cf)
						dmatch(s-matstr, cs);
					else
					{
						ncoll++;
						*avx++ = cat(matstr, "");
					}
					if(wild)
					{
						sort(oav);
						return;
					}
				}
				s = uname;
				if(backc != '\n')
				{
					while((c = getchar()) && c != '\n');
					if(!c)
						break;
				}
			}
		sort(oav);
		if(!mflg)
			wflg = 1;
		return;
	}
	ocs = as;
	proto = 0;
	s = matstr;
	if(c == '/')
	{
		*s++ = c;
		ocs = cs;
		c = *cs++;
		proto = s;
	}
	do
	{
		if(c == '/')
		{
			proto = s;
			ocs = cs;
		}
		else if(c == '*' || c == '?' || c == '[')
		{
			if(proto == 0)
			{
				s = matstr;
				*s++ = '.';
				proto = s;
			}
			*proto = 0;
			dmatch(proto-matstr, ocs);
			sort(oav);
			return;
		}
		*s++ = c;
	}
	while(c = *cs++);
	*avx++ = cat(as, "");
	close(fin);		/* close "/etc/passwd" .. */
}
#endif	AUSAM

dmatch(mof, ps)
int mof;
char *ps;
{
	register char *s;
	int chan, df, loc, ncos, nread;
	char *sp;
	struct statb f;
	struct dent
	{
		int d_ino;
		char d_name[14];
	} etab[32];

	df = 0;
	s = ps;
	if( mof == 0 || ( mof == 1 && matstr[0] == '/'))
		ncos = 0;
	else
		ncos = 1;
	if(stat(matstr, &f) == -1 || (f.i_mode & IFMT) != IFDIR)
		return;
	if((chan = open(matstr, 0)) == -1)
		return;
	while( *s && *s++ != '/');
	if(*s)
	{
		*--s = 0;
		sp = s;
	}
	else
		df++;
	loc = 0;
	while((nread = read(chan, etab, 512)) > 0)
	{
		register struct dent *entry;

		loc =+ nread;
		nread =>> 4;
		for(entry = &etab[0]; entry < &etab[nread]; entry++)
		{
			char ename[16];

			if(entry->d_ino == 0)
				continue;
			copyn(entry->d_name, ename, 14);
			ename[14] = 0;
			if(match(ename, ps))
			{
				register char *ep;

				s = &matstr[mof];
				ep = ename;
				if(ncos)
					*s++ = '/';
				while(*s++ = *ep++);
				if(chan == NOFILE - 1)
				{
					close(chan);
					chan = -1;
				}
				if(df)
				{
					ncoll++;
					*avx++ = cat(matstr, "");
				}
				else
					dmatch(s - matstr - 1, sp + 1);
				if(chan == -1)
				{
					chan = open(matstr, 0);
					seek(chan, loc, 0);
				}
			}
		}
	}
	close(chan);
	if(!df)
		*sp = '/';
}

sort(oavx)
char **oavx;
{
	register char **p1, **p2, **c;

	p1 = oavx;
	while (p1 < avx-1)
	{
		p2 = p1;
		while(++p2 < avx)
		{
			if (compar(*p1, *p2) > 0)
			{
				c = *p1;
				*p1 = *p2;
				*p2 = c;
			}
		}
		p1++;
	}
}


match(s, p)
char *s, *p;
{
	if (*s=='.' && *p!='.')
		return(0);
	return(amatch(s, p));
}

amatch(as, ap)
char *as, *ap;
{
	register char *s, *p;
	register scc;
	int c, cc, ok, lc;

	s = as;
	p = ap;
	if (scc = *s++)
		if ((scc =& 0177) == 0)
			scc = 0200;
	switch (c = *p++)
	{

	case '[':
		ok = 0;
		lc = 077777;
		while (cc = *p++)
		{
			if (cc==']')
			{
				if (ok)
					return(amatch(s, p));
				else
					return(0);
			}
			else if (cc=='-')
			{
				if (lc<=scc && scc<=(c = *p++))
					ok++;
			}
			else
				if (scc == (lc=cc))
					ok++;
		}
		return(0);

	default:
		if (c!=scc)
			return(0);

	case '?':
		if (scc)
			return(amatch(s, p));
		return(0);

	case '*':
		return(umatch(--s, p));

	case '\0':
		return(!scc);
	}
}

umatch(s, p)
char *s, *p;
{
	if(*p==0)
		return(1);
	while(*s)
		if (amatch(s++,p))
			return(1);
	return(0);
}

compar(as1, as2)
char *as1, *as2;
{
	register char *s1, *s2;

	s1 = as1;
	s2 = as2;
	while (*s1++ ==  *s2)
		if (*s2++ == 0)
			return(0);
	return (*--s1 - *s2);
}

cat(as1, as2)
char *as1, *as2;
{
	register char *s1, *s2;
	register int c;

	s2 = string;
	s1 = as1;
	while (c = *s1++)
	{
		if (s2 > ablimit)
			gdie(ARGLNG);
		c =& 0177;
		if (c==0)
		{
			*s2++ = '/';
			break;
		}
		*s2++ = c;
	}
	s1 = as2;
	do
	{
		if (s2 > ablimit)
			gdie(ARGLNG);
		*s2++ = c = *s1++;
	} while (c);
	s1 = string;
	string = s2;
	return(s1);
}

/*
 *	gdie: terminate with error message, but no seek to EOF
 *	purpose is to make internal glob work same as external one.
 *	something should be done about match/nomatch actions.
 */
gdie(str)
char *str;
{
	gerr(str);
	exit(1);
}

gerr(str)
char *str;
{
	prs(ARG0); prs(": ");
	prs(str); prs("\n");
}


int tpflag;

finish()
{
	struct stime now;

	times(&now);
	prs("\nconnect  time ");
	ptime((time() - starttime)*HZ);
	prs("\nuser cpu time ");
	ptime(now.procu + now.childu);
	prs("\nsys  cpu time ");
	ptime(now.procs + now.childs);
	prs("\n\n");
}

ptime(tics)
long tics;
{
	register long *t = &tics;
	struct
	{
		int hiint;
		unsigned loint;
	};

	tpflag = 0;
	tprintf("%d:%d:%d.%d",
	    (*t =/ 60, t)->loint,
	     (*t =/ 60, t)->loint % 60,
	      (*t =/ HZ, t)->loint % 60,
	       (t->loint % HZ) * 2);
}

tprintf(s, a)
register char *s;
int a;
{
	register *ap = &a;
	register char c;

	while(c = *s++)
		if(c == '%')
		{
			tprint1n((*ap)/10);
			if(*++s == '.')
				tpflag++;
			tprint1n((*ap++)%10);
		}
		else
			putc(tpflag ? c : ' ');
}

tprint1n(n)
char n;
{
	putc(((n && ++tpflag) || tpflag) ? (n+'0') : ' ');
}

#ifdef	AUSAM
killer(argv)
register char **argv;
{
	register unsigned pid,upper;
	int sig;

	sig = 9;
	while(*argv)
	{
		if(**argv == '-')
		{
			(*argv)++;
			sig = atoi(*argv);
			if(sig <= 0 || sig >= 20)
			{
				die("Bad signal", 0);
				return;
			}
		}
		else
		{
			pid = nscan(argv);
			if(**argv == '-')
			{
				(*argv)++;
				upper = nscan(argv);
				if(pid > upper)
				{
					die("Lower bound exceeds upper bound", 0);
					break;
				}
			}
			else
				upper = 0;
			if(**argv)
			{
				die(NONUM, *argv);
				break;
			}
			do
			{
				if( kill(pid, sig) < 0 && upper == 0)
				{
					die(itoa(pid), " - Not found");
				}
			}
			while(pid++ < upper);
		}
		argv++;
	}
}

nscan(s)
char **s;
{
	register char *c;
	register i;

	c = *s;
	i = 0;
	while(*c <= '9' && *c >= '0')
		i = i*10 + *c++ - '0';
	*s = c;
	return(i);
}
#endif	AUSAM

#ifndef	AUSAM
accfix()
{
	char	*p;		/* pointer to pwbuff */
	char	c;		/* scratch character */
	char	*cp;		/* pointer to character "c"  */
	char	low_uid;	/* low byte of getuid() call */
	int	uid;		/* user I*/
	int	x_uid;		/* user id from pwbuff */
	int	pwfd;		/* file descriptor for password file */
	int EOF;		/* end of file flag */

	EOF = 0;		/* set end of file flag */
	cp = &c;		/* set the address of "c" */
	low_uid = getuid();
	uid = low_uid;
	if ((pwfd = open("/etc/passwd",0)) < 0 ) 
	{
		/* error */
		prints(2,"no /etc/passwd\n");
		exit(1);	/* kill */
	}
	while(!EOF)
	{
		/* read the parameter file */
		p = &pwbuff[0];	/* set up pointer */
		do
		{
			if(!(read(pwfd, cp, 1)))
				/* end of file */
				EOF++;		/* set flag on */
			*p++ = c;	/* store the character */
		}
		while((c != '\n') && !EOF);
		x_uid = set_uid();
		if(uid == x_uid)
			/* user password entry found */
			 break;
	}
	if(EOF)
	{
		/* error */
		prints(2, "user search failed.\n");
		exit(1);	/* kill */
	}
	set_name();
	set_dir();
	set_tty();
	close(pwfd);	/* close passwd file descriptor */
}

set_uid()
{
	int x_uid;	/* returned user ID */
	char *pw;	/* pointer to pwbuff */

	pw = skip_colon(uid_pos);
	x_uid = fix_atoi(pw);
	return(x_uid);
}

set_name()
{
	char *pw;	/* pointer to pwbuff */
	int n;		/* scratch integer   */

	n = 0;		/* set index */
	pw = &pwbuff[0];
	while(*pw != ':')
	{
		/* read in login_name */
		user.u_logname[ n++ ] = *pw++;
	}
}

set_dir()
{
	int n;	/* scratch integer */
	char *pw;	/* pointer to pwbuff */

	n = 0;
	pw = skip_colon(dir_pos);
	while(*pw != ':')
	{
		/* read login directory name */
		user.u_logdir[ n++ ] = *pw++;
	}
}

char *skip_colon(pos)
int pos;	/* number of colons to be skipped */
{
	char *pw;	/* pointer to pwbuff */
	int n;		/* scratch integer   */

	n = 0;
	pw = &pwbuff[0];
	do
	{
		if(*pw == ':')
			/* add 1 to colon count */
			n++;
		pw++;
	}
	while(n < pos);
	return(pw);	/*indexintopwbuff*/
}

set_tty()
{
	user.u_logtty = ttyn(2);
}

fix_atoi(pw)
char *pw;
{

	char *sp;		/* string pointer */
	int i;		/* holds the integer result */

	sp = pw;	/* pass address */
	i = 0;
	while(1)
	{
		if((*sp < '0') || (*sp > '9'))
			/* end of integer string */
			break;
		i = i*10 + (*sp++ -'0');
	}
	return i;
}
#endif	AUSAM

unnext()
{
	seek(0,0,2);
	setexit(INTR,1);
}
