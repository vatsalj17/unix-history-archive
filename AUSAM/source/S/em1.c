#include "em.h"



main(argc, argv)
char **argv;
{
	register char *p1, *p2;
#ifdef	ERRORS
	register eflag;
#endif	ERRORS
	extern int onintr();
	extern int terminate();

#ifdef	AUTOW
	extern int catchclock();

	signal(SIGCLOK, catchclock);
#endif	AUTOW
	onquit = signal(SIGQUIT, 1);
	signal(SIGTERM, terminate);
	onhup = signal(SIGHUP, 0);
	if ((*argv)[1] == 'm') vflag = 1;
	elfic = 0;
	argv++;
	if (argc > 1 && **argv=='-')
	{
		p1 = &(*argv)[1];
		while (*p1 != '\0')
			switch (*p1++)
			{

		    case 'e':	elfic = 1;
				break;

		    case 'l':	lflag++;
				break;

#ifndef	ERRORS
		    case 's':	vflag = -1;
#else
		    case 's':	sflag++;
#endif	ERRORS
				break;
			}
		if ((*argv)[1] == '\0') vflag = -1;
		argv++;
		argc--;
	}
	if (argc>1)
	{
		p1 = *argv;
		p2 = savedfile;
		while (*p2++ = *p1++);
		breaks(p1 - 3);
		globp = "r";
	}
	fendcore = sbrk(0);
	if (vflag > 0) putnls("Editor");
#ifdef	ERRORS
	istty = (gtty(0, linebuf) != -1);	/* flogs linebuf */
	eflag = 0;
	setexit();		/* catch init errors */
	if (eflag) exit(1);
	eflag = 1;
#endif	ERRORS
	margin = LBSIZE - 40;
#ifdef	N_LINES
	lines = LINES;
#endif
#ifdef	ERRORS
	linenr = 1;
#endif
	firstime = 1;
	tfile = -1;
	init();
	if ((signal(SIGINTR, 1) & 01) == 0) signal(SIGINTR, onintr);
	setexit();
	commands(vflag);
	unlink(tfname);
	exit(0);
}

terminate()
{
	signal(SIGTERM, 1);
	if (dol != zero) writeout("saved.file");
	putnls("System going down - tmp file written to \"saved.file\""); 
	unlink(tfname);
	exit(1);
}

#ifdef	AUTOW
catchclock()
{
	register int	ad1, ad2, oldintr;
	long	csav;
	char	bell	BELL;

	if (delaywrite) { writewaiting++; return; }
	signal(SIGCLOK, catchclock);
	justwrit++;
	if (dol != zero)		/* empty file */
	{
		oldintr = signal(SIGINTR, 1);
		ad1 = addr1;
		ad2 = addr2;
		addr2 = 0;
		csav = count;
		write(2, &bell, 1);
		writeout(-3);
		write(2, &bell, 1);
		addr1 = ad1;
		addr2 = ad2;
		count = csav;
		signal(SIGINTR, oldintr);
	}
	clktim(writetime);
}
#endif	AUTOW

commands(prompt)
{
	int getfile(), gettty();
	register *a1, c;
	register char *p;
	int r, n;
	int fd;

	for (;;)
	{

#ifdef	ERRORS
		if (prompt != -2) currlnnr = linenr;	/* i.e., not global */
#endif	ERRORS

		if (pflag)
		{
			pflag = 0;
			addr1 = addr2 = dot;
			goto print;
		}
		if (prompt>0 && globp==0) putch(PROMPT);
		addr1 = addr2 = 0;
		xflag = 0;
#ifdef	AUTOW
		if (writewaiting)
		{
			delaywriting = writewaiting = 0;
			catchclock();
			justwrit = 0;
		}
#endif	AUTOW
#ifndef	ADDRESS
		do
		{
			addr1 = addr2;
			if ((a1 = address())==0) { c = getchar(); break; }
			addr2 = a1;
			if ((c=getchar()) == ';') { c = ','; dot = a1; }
		} while (c==',');
#else
		a1 = address();
		if ((c = getchar()) == ';' && a1)
		{ dot = addr1 = addr2 = a1; a1 = address(); c = getchar(); }
		if (c == ',')
		{
			if (a1 == 0)
				if (dol == zero) error(30);
				else addr1 = zero + 1;
			else addr1 = a1;
			if ((a1 = address()) == 0) addr2 = dol;
			else addr2 = a1;
			c = getchar();
		}
		else addr2 = a1;
#endif	ADDRESS
		if (addr1==0) addr1 = addr2;
		if (c>='A' && c<='Z') c =| 040;
		switch(c)
		{
	
	    case 'a':	setdot();
			newline();
			append(gettty, addr2);
			continue;
	
	    case 'b':	c = peekc = getchar();
			if (c=='+' || c=='-') peekc = 0;
			else if (c != '\n') error(20);
			newline();
			margin = c == '-' ? LBSIZE - 40 : LENGTH - 20;
			continue;
	
#ifndef	CHDIR
	    case 'c':	setdot();
#else
	    case 'c':	if (addr2 == 0 && (peekc = getchar()) == 'd')
			{
				peekc = 0; if (getchar() != ' ') error(20);
				do c = getchar(); while (c == ' ');
				if (c == '\n') error(20);
				p = linebuf;
				do *p++ = c; while ((c = getchar()) != '\n');
				*p = '\0';
				if (chdir(linebuf)) error(8);
				continue;
			}
			setdot();
#endif	CHDIR
			newline();
			fd = *addr1;		/* borrow 'fd' */
			delete();
			if (append(gettty, addr1-1) > 0) retag(*addr1, fd);
			continue;
	
	    case 'd':	setdot();
			newline();
			delete();
			continue;
	
	    case 'e':	if (elfic) error(22);
			setnoaddr();
			if ((peekc = getchar()) != ' ') error(20);
			savedfile[0] = 0;
			init();
			addr2 = zero;
			goto caseread;
	
#ifndef	MISC
	    case 'f':	if (elfic) error(22);
			setnoaddr();
#else
	    case 'f':	setnoaddr();
#endif
			if ((c = getchar()) != '\n')
			{
				peekc = c;
#ifdef	MISC
				if (elfic && *savedfile) error(22);
#endif
#ifndef	SEARCH_STRING
				savedfile[0] = 0;
#endif	SEARCH_STRING
				filename(1);
			}
			putnls(savedfile);
			continue;
	
	    case 'g':	global(1);
			continue;
	
	    case 'h':	newline();
			if ((fd = open("/usr/lib/emhelp", 0)) < 0)
			{
				putnls("/usr/lib/emhelp not found");
				continue;
			}
			while ((n = read(fd, linebuf, 512)) > 0) write(1, linebuf, n);
			close(fd);
			continue;
	
	    case 'i':	setdot();
			nonzero();
			newline();
			append(gettty, addr2-1);
			continue;
	
#ifdef	JOIN
	    case 'j':	if (addr2 == 0) addr2 = (addr1 = dot) + 1;
			if (addr1 > addr2) error(23);
			if (addr1 == addr2) addr2++;
			nonzero();
			nocmpress = 1;
			if ((peekc = getchar()) == 'c') { nocmpress--; peekc = 0; }
			newline();
			join();
			continue;
#endif	JOIN
	
	    case 'k':	if ((c = getchar()) < 'a' || c > 'z') error(24);
			newline();
			setdot();
			nonzero();
			names[c-'a'] = *addr2 | 01;
			continue;
	
	    case 'm':	move(0);
			continue;
	
	    case '\n':	if (addr2==0) addr2 = dot+1;
			addr1 = addr2;
			if (lflag) listf++;
			goto print;

	    case 'l':	listf++;
	    case 'p':	newline();
	    print:	setdot();
			nonzero();
			a1 = addr1;
			do putnls(getline(*a1++)); while (a1 <= addr2);
			dot = addr2;
			listf = 0;
			continue;
	
	    case 'o':	setdot();
#ifdef	AUTOW
			delaywrite++;
#endif	AUTOW
			op(globp);
#ifdef	AUTOW
			delaywrite = 0;
#endif	AUTOW
			continue;
	
	    case 'q':	setnoaddr();
			newline();
#ifndef	MISC
			if (elfic) { firstime = 1; writeout(0); }
#else
			if (elfic && dol != zero) writeout(0);
#endif
			unlink(tfname);
			exit(0);
	
	    case 'r':
	    caseread:	filename(0);
#ifdef	AUTOW
			delaywrite++;
#endif	AUTOW
			if ((io = open(file, 0)) < 0) { lastc = '\n'; error(0); }
			setall();
			ninbuf = 0;
			append(getfile, addr2);
			exfile();
#ifdef	NULLS
			if (listf && vflag >= 0)		/* borrows 'listf' */
			{
				putchar('(');
				if ((count = listf) > 0) putd();
				else puts("many");
				putnls(" null(s) stripped from file)");
			}
			listf = 0;
#endif	NULLS
#ifdef	AUTOW
			delaywrite = 0;
#endif	AUTOW
			continue;
	
	    case 'x':	xflag = 1;
	    case 's':	setdot();
			nonzero();
			substitute(globp);
			xflag = 0;
			continue;
	
#ifndef	AUTOW
	    case 't':
#else
	    case 't':	if ((peekc = getchar()) == 'i')		/* timeout */
			{
				fd = 0; peekc = 0;
				if (*savedfile == 0) error(25);
				while (((c = getchar()) >= '0') && c <= '9')
					fd = fd * 10 + c - '0';
				if (c != '\n') error(20);
				writetime = fd * 60;
				clktim(writetime);
				continue;
			}
#endif	AUTOW
			move(1);
			continue;

	    case 'v':	global(0);
			continue;
	
	    case 'w':	if (elfic) error(22);
#ifdef	AUTOW
			delaywrite++;
#endif	AUTOW
#ifndef	DRM
			writeout(0);
#else
			if ((peekc = getchar()) == 'a') { peekc = 0; writeout(-1); }
#ifdef	MISC
			else if (peekc == 'x') { peekc = 0; writeout(-2); }
#endif	MISC
			else writeout(0);
#endif	DRM
#ifdef	AUTOW
			delaywrite = 0;
#endif	AUTOW
			continue;
	
#ifdef	PRINT_STRING
	    case 'z':	setnoaddr();
			newline();
			printstrings();
			continue;
#endif	PRINT_STRING

	    case '"':	setdot();
#ifdef	N_LINES
			fetchnr();
#endif	N_LINES
			newline();
			if (addr1 < zero) error(23);
			dot = addr1;
			if (dot == dol) error(23);
			addr1 = dot + 1;
#ifndef	N_LINES
			addr2 = dot + LINES - 1;
#else
			addr2 = dot + lines - 1;
#endif	N_LINES
			if (addr2 > dol) addr2 = dol;
	    outlines:	pflag = listf;
			for (a1 = addr1; a1 <= addr2; a1++) putnls(getline(*a1));
			dot = addr2;
			pflag = listf = 0;
			continue;
	
	    case '&':	setdot();
#ifdef	N_LINES
			fetchnr();
#endif	N_LINES
			newline();
			nonzero();
			dot = addr1;
#ifndef	N_LINES
			addr1 = dot - (LINES-2);
#else
			addr1 = dot - (lines-2);
#endif	N_LINES
			addr2 = dot;
			if (addr1 <= zero) addr1 = zero + 1;
			goto outlines;
	
	    case '%':
#ifdef	N_LINES
			fetchnr();
#endif	N_LINES
			newline();
			pflag = listf; listf = 0;
			setdot();
			nonzero();
			dot = addr1;
#ifndef	N_LINES
			addr1 = dot - (LINES/2 - 2);
			addr2 = dot + (LINES/2 - 2);
#else
			addr1 = dot - (lines/2 - 2);
			addr2 = dot + (lines/2 - 2);
#endif	N_LINES
			if (addr1 <= zero) addr1 = zero + 1;
			if (addr2 > dol) addr2 = dol;
			listf = pflag;
			for (a1 = addr1; a1 <= addr2; a1++)
			{
				if (a1 == dot) screensplit();
				putnls(getline(*a1));
				if (a1 == dot) screensplit();
			}
			pflag = listf = 0;
			continue;
	
	    case '>':	if (vflag > 0) vflag = 0;
			newline();
			reset();
	
	    case '<':	vflag = 1;
			newline();
			reset();
	
	    case '=':	setall();
			newline();
			count = (addr2-zero) & 077777;
			putd();
			putchar('\n');
			continue;
	
	    case '!':	setnoaddr();
			unix();
			continue;
	
#ifndef	ERRORS
	    case EOF:	if (prompt == -2 || ttyn(0) == 'x') return;
#else
	    case EOF:	if (prompt == -2 || ! istty) return;
#endif	ERRORS
			continue;
	
		}
		error(20);
	}
}

writeout(filen)
char *filen;
{
	register char *p1, *p2;

#ifdef	MISC
	extern write();
#endif
#ifdef	DRM
	register flag 0;
#ifndef	MISC

	if (filen == -1) { flag++; filen = 0; }
#else
	if (filen == -1 || filen == -2) { flag = filen; filen = 0; }
#endif	MISC
#endif	DRM

	setall();
	nonzero();
	if (filen == 0)
	{
		if (elfic)
		{
			p1 = savedfile;
			if (*p1 == 0) error(25);
			p2 = file;
			while (*p2++ = *p1++);
		}
		else filename(0);
		filen = file;
	}
#ifndef	DRM
	if ((io = creat(filen, 0600)) < 0) error(1);
#else
	if (flag == -1 && (io = open(filen, 1)) >= 0) seek(io, 0, 2);
#ifndef	MISC
	else if ((io = creat(filen, 0600)) < 0) error(1);
#else
#ifdef	AUTOW
	else if (filen == -3)
	{
		if ((io = creat(savedfile, 0600)) < 0)
		{
			putnls("Auto-write - cannot write file ?");
			return;
		}
	}
#endif
	else if ((io = creat(filen, flag == -2 ? 0700 : 0600)) < 0) error(1);
#endif	MISC
#endif	DRM
#ifdef	MISC
	if (iblock >= 0 && ichanged) { blkio(iblock, ibuff, write); ichanged = 0; }
	if (oblock >= 0) blkio(oblock, obuff, write);
#endif
	putfile();
	exfile();
}

address()
{
	register *a1, minus, c;
	int n;
	int relerr;

	minus = 0;
	a1 = 0;
	for (;;)
	{
		c = getchar();
		if ('0'<=c && c<='9')
		{
			n = 0;
			do
			{
				n =* 10;
				n =+ c - '0';
			} while ((c = getchar())>='0' && c<='9');
			peekc = c;
			if (a1==0) a1 = zero;
			if (minus<0) n = -n;
			a1 =+ n;
			minus = 0;
			continue;
		}
		relerr = 0;
		if (a1 || minus) relerr++;
		switch(c)
		{

	    case ' ':
	    case '\t':	continue;
	
	    case '+':	minus++;
			if (a1==0) a1 = dot;
			continue;

	    case '-':
	    case '^':	minus--;
			if (a1==0) a1 = dot;
			continue;
	
	    case '?':
	    case '/':	compile(c);
			a1 = dot;
			for (;;)
			{
				if (c=='/') { a1++; if (a1 > dol) a1 = zero; }
				else if (--a1 < zero) a1 = dol;
				if (execute(0, a1)) break;
				if (a1==dot) error(21);
			}
			break;
	
	    case '$':	a1 = dol;
			break;
	
	    case '.':	a1 = dot;
			break;

	    case '\'':	if ((c = getchar()) < 'a' || c > 'z') error(24);
			c =- 'a';
			for (a1 = zero+1; a1 <= dol; a1++)
				if (names[c] == (*a1|01)) break;
			break;
	
	    default:	peekc = c;
			if (a1==0) return(0);
			a1 =+ minus;
			if (a1<zero || a1>dol) error(23);
			return(a1);
		}
		if (relerr) error(20);
	}
}

setdot()
{
	if (addr2 == 0) addr1 = addr2 = dot;
	if (addr1 > addr2) error(23);
}

setall()
{
	if (addr2==0)
	{
		addr1 = zero+1;
		addr2 = dol;
		if (dol==zero) addr1 = zero;
	}
	setdot();
}

setnoaddr()
{
	if (addr2) error(20);
}

nonzero()
{
	if (addr1<=zero || addr2>dol) error(23);
}

newline()
{
	register c;

	if ((c = getchar()) == '\n') return;
	if (c=='p' || c=='l' || c=='P' || c=='L')
	{
		pflag++;
		if (c=='l' || c=='L') listf++;
		if (getchar() == '\n') return;
	}
	error(20);
}

#ifdef	N_LINES
fetchnr()
{
	register int n;
	register char c;

	if ((c = getchar()) >= '0' && c <= '9')
	{
		n = 0;
		do
		{
			n =* 10;
			n =+ c - '0';
		} while ((c = getchar()) >= '0' && c <= '9');
		if (n < 4) error(20);	/* min. is 4 lines */
		lines = n;
	}
	peekc = c;
}
#endif	N_LINES

retag (newlabel, oldlabel)
{
	register int	ol, nl;
	register int	*ip;

	ol = oldlabel | 01;
	nl = newlabel | 01;
	for (ip = &names[0]; ip < &names[26]; ip++)
		if (*ip == ol) *ip = nl;
}

filename(flag)
{
	register char *p1, *p2;
	register c;

#ifdef	MISC
	chkflg = 1;
#endif	MISC
	count = 0;
	c = getchar();
	if (c=='\n' || c==EOF)
	{
		if (elfic && !firstime) error(22); else firstime = 0;
		p1 = savedfile;
		if (*p1==0) error(25);
		p2 = file;
		while (*p2++ = *p1++);
#ifdef	MISC
		chkflg = -1;
#endif	MISC
		return;
	}
	if (c!=' ') error(20);
	while ((c = getchar()) == ' ');
	if (c=='\n') error(20);
	p1 = file;
	do *p1++ = c; while ((c = getchar()) != '\n');
	*p1++ = 0;
#ifndef	SEARCH_STRING
	if (savedfile[0]==0)
#else
	if (flag || savedfile[0]==0)
#endif	SEARCH_STRING
	{
		p1 = savedfile;
		p2 = file;
		while (*p1++ = *p2++);
		breaks(p1 - 3);
	}
}

breaks(p)
register char *p;
{
	if (*p++ == '.' && (*p == 'r' || *p == 'n')) margin = LENGTH - 20;
}

exfile()
{
	close(io);
	io = -1;
#ifndef	AUTOW
	if (vflag >= 0)
	{
#else
	if (vflag>=0 && !justwrit)
	{
#endif	AUTOW
		putd();
		putchar('\n');
	}
}

onintr()
{
	signal(SIGINTR, onintr);
	putchar('\n');
	lastc = '\n';
	error(26);
}

error(type)
{
	register c;

#ifdef	ERRORS

	register char *sp;

static char *sysmsgs[]		/* system-type failures */
{
	"Open",			/* type  0 == file non-existant		*/
	"Create",		/* type  1 == creat fails		*/
	"Write",		/* type  2 == write error		*/
	"Gtty",			/* type  3 == gtty failed for std input	*/
	"Fork",			/* type  4 == fork failed 		*/
	"Temp i/o",		/* type  5 == i/o on temp file failed   */
	"Create temp",		/* type  6 == creat temp file failed 	*/
	"Open temp",		/* type  7 == open temp file failed	*/
	"Chdir",		/* type  8 == chdir failed		*/
};

static char *msgs[]		/* editor-type failures */
{
	"Syntax",		/* type 20 == syntax (+ others)		*/
	"No match",		/* type 21 == match fail		*/
	"Elfic",		/* type 22 == not allowed in elfic 	*/
	"Address",		/* type 23 == illegal address(s)	*/
	"Reg name",		/* type 24 == invalid register name	*/
	"File",			/* type 25 == no file specified		*/
	"Interrupt",		/* type 26 == interrupted		*/
	"Line size",		/* type 27 == line too big		*/
	"Too many lines",	/* type 28 == out of core		*/
	"Temp size",		/* type 29 == out of temp space		*/
	"Buffer empty",		/* type 30 == null file			*/
	"Command too long",	/* type 31 == UNIX command too long	*/
	"&, %, or subexpression undefined",
				/* type 32 == ...in the strings		*/
};

extern	char *sys_errlst[];
extern	int   sys_nerr;
extern	int   errno;

#endif	ERRORS

	listf = count = 0;

#ifndef	ERRORS
	if (type==21) putnls("??"); else putnls("?");
	seek(0, 0, 2);
#else
	pflag = (type != 0 && type != 21);	/* flog 'pflag' as cleared later anyway */
	if (! istty)
	{
		if (vflag >= 0 || pflag)
		{ count = currlnnr; putd(); putchar(' '); }
		if (pflag) seek(0,0,2);
	}
	if (sflag) { if (type == 21) putnls("??"); else putnls("?"); }
	else if (vflag >= 0 || pflag)
	{
		if (type == 23 && dol==zero) type = 30;
		if (type < 20) sp = sysmsgs[type]; else sp = msgs[type - 20];
		while (*sp) putchar(*sp++);
		if (type < 20 && errno < sys_nerr)
		{
			sp = sys_errlst[errno];
			puts(": ");
			while (*sp) putchar(*sp++);
		}
		putnls(" ?");
	}
#endif	ERRORS

	pflag = 0;
	if (globp) lastc = '\n';
	globp = 0;
	peekc = lastc;
	while ((c = getchar()) != '\n' && c != EOF);
	if (io > 0) { close(io); io = -1; }
	reset();
}

getchar()
{
#ifdef	AUTOW
	register count;

#endif
	if (lastc=peekc) { peekc = 0; return(lastc); }
	if (globp)
	{
		if ((lastc = *globp++) != 0) return(lastc);
		globp = 0;
		return(EOF);
	}
#ifndef	AUTOW
	if (read(0, &lastc, 1) <= 0) return(lastc = EOF);
#else
	while ((count = read(0, &lastc, 1)) <= 0)
		if (count < 0 && justwrit) justwrit = 0;
		else if (! istty) return(lastc = EOF);
#endif	AUTOW
	lastc =& 0177;
#ifdef	ERRORS
	if (! istty && lastc=='\n') linenr++;
#endif	ERRORS
	return(lastc);
}

gettty()
{
	register c, gf;
	register char *p;

	p = linebuf;
	gf = globp;
	while ((c = getchar()) != '\n')
	{
		if (c==EOF) { if (gf) peekc = c; return(c); }
		if ((c =& 0177) == 0) continue;
		*p++ = c;
		if (p >= &linebuf[LBSIZE-2]) error(27);
	}
	*p++ = 0;
	if (linebuf[0]=='.' && linebuf[1]==0) return(EOF);
	return(0);
}

#ifdef	MISC
checkfile(fstwd)
register fstwd;
{
	static list[]
	{
		01,				/* dec object */
		0404,				/* pascal obj */
		0407, 0410, 0411, 0412,		/* objects */
		017437,				/* packed */
		0177545, 0177555,		/* archives */
		0,				/* end-of-list */
	};
	register *lp list;

	while (*lp)
		if (*lp++ == fstwd)
		{
			putnls("Illegal file type");
			return 1;
		}
	return 0;
}
#endif	MISC

getfile()
{
	register c;
	register char *lp, *fp;

	lp = linebuf;
	fp = nextip;
	do
	{
		if (--ninbuf < 0)
		{
			if ((ninbuf = read(io, genbuf, LBSIZE)-1) < 0)
#ifndef	NULLS
				return(EOF);
#else
				if (lp == linebuf) return(EOF);
				else
				{
					if (vflag >= 0) putnls("No `\\n' on last line - `\\n' added.");
					if (lp >= &linebuf[LBSIZE]) { lastc = '\n'; error(27); }
					*lp = 0;
					return(0);
				}
#endif	NULLS
			fp = genbuf;
#ifdef	MISC
			if (chkflg)
			{
				if (checkfile(genbuf->integer))
				{
					if (chkflg < 0) savedfile[0] = 0;
					return EOF;
				}
				chkflg = 0;
			}
#endif	MISC
		}
		if (lp >= &linebuf[LBSIZE]) { lastc = '\n'; error(27); }
#ifndef	NULLS
		if ((*lp++ = c = *fp++ & 0177) == 0) { lp--; continue; }
#else
		if ((*lp++ = c = *fp++ & 0177) == 0)
		{
			lp--;
			if (listf >= 0) listf++;
			continue;
		}
#endif	NULLS
		count++;
	} while (c != '\n');
	*--lp = 0;
	nextip = fp;
	return(0);
}

putfile()
{
	int *a1;
	register char *fp, *lp;
	register nib;

	nib = 512;
	fp = genbuf;
	a1 = addr1;
	do
	{
		lp = getline(*a1++);
		for (;;)
		{
			if (--nib < 0)
			{
				nib = fp - genbuf;
				if (write(io, genbuf, nib) != nib) error(2);
				nib = 511;
				fp = genbuf;
			}
			count++;
			if ((*fp++ = *lp++) == 0) { fp[-1] = '\n'; break; }
		}
	} while (a1 <= addr2);
	nib = fp - genbuf;
	if (write(io, genbuf, nib) != nib) error(2);
}

append(f, a)
int (*f)();
{
	register *a1, *a2, *rdot;
	int tl;

	nline = 0;
	dot = a;
	while ((*f)() == 0)
	{
		if (dol >= endcore)
		{
			if (sbrk(1024) == -1) error(28);
			endcore.integer =+ 1024;
		}
		tl = putline(linebuf);
		nline++;
		a1 = ++dol;
		a2 = a1+1;
		rdot = ++dot;
		while (a1 > rdot) *--a2 = *--a1;
		*rdot = tl;
	}
	return(nline);
}

unix()
{
#ifndef UNIXCMD
	register savint, pid, rpid;
	int retcode;
	char c, *lp, *fp;
	char	workbuffer[UNIXBUFL];
#else
	int savint, pid, retcode;
	register rpid;
	register char *lp, *fp;
#endif

	pid = 0;
#ifndef UNIXCMD
	c = getchar();
	if (c != '!')
	{
		lp = unixbuffer;
		for (;;)
		{
			if (c == '\n')
				if (lp!=unixbuffer && lp[-1]==BACKSL) lp--;
				else break;
			*lp++ = c;
			c = getchar();
		}
		*lp = '\0';
	}
	else { pid = 1; while (getchar() != '\n'); }
	lp = workbuffer;
	fp = unixbuffer;
	while ((c = *fp++) != '\0')
	{
		if (c == '%')
			if (lp!=unixbuffer && lp[-1]==BACKSL) lp--;
			else
			{
				pid = 1;
				for (rpid = 0; savedfile[rpid] != '\0'; rpid++)
					*lp++ = savedfile[rpid];
				continue;
			}
		*lp++ = c;
	}
	*lp = '\0';
	if (pid) { putchar('!'); putnls(workbuffer); }
#else
	lp = linebuf;
	for (;;)
	{
		if ((*lp = getchar()) == '\n') break;
		if (*lp == '!')
		{
			fp = unixbuffer;
			while (*lp++ = *fp++)
				if (lp >= &linebuf[UNIXBUFL]) error(31);
			lp--;
			pid++;
			continue;
		}
		if (*lp == BACKSL)
			if ((fp = getchar()) == '\n' || fp == '!') *lp = fp;
			else *++lp = fp;
		if (++lp >= &linebuf[UNIXBUFL]) error(31);
	}
	*lp = '\0';
	fp = unixbuffer;
	lp = linebuf;
	retcode = 0;
	while (*fp++ = *lp++);
	fp = unixbuffer;
	lp = linebuf;
	while (*lp = *fp++)
		if (*lp == BACKSL)
		{
			if ((*++lp = *fp++) == '%') *--lp = '%';
			lp++;
		}
		else if (*lp++ == '%')
		{
			pid++; lp--;
			for (rpid = 0; *lp++ = savedfile[rpid]; rpid++)
				if (lp >= &linebuf[LBSIZE]) error(31);
			lp--;
		}
	if (pid) { putchar('!'); putnls(linebuf); }
#endif UNIXCMD
#ifdef	AUTOW
	clktim(0);
#endif
	if ((pid = fork()) == 0)
	{
		signal(SIGHUP, onhup);
		signal(SIGQUIT, onquit);
#ifndef UNIXCMD
		execl("/bin/sh", "sh", "-c", workbuffer, 0);
#else
		execl("/bin/sh", "sh", "-c", linebuf, 0);
#endif
		exit(1);
	}
#ifdef	ERRORS
	if (pid == -1) error(4);	/* fork failed */
#endif	ERRORS
	savint = signal(SIGINTR, 1);
#ifndef	AUTOW
	while ((rpid = wait(&retcode)) != pid && rpid != -1);
#else
	while ((rpid = waitx(&retcode)) != pid);
	clktim(writetime);
#endif	AUTOW
	signal(SIGINTR, savint);
	putnls("!");
}

#ifdef	JOIN
join()
{
	register char *bp, *lp;
	register nl;
	int tl, toggle, *lptr;

	lp = linebuf; toggle = 1;
	for (lptr = addr1; lptr <= addr2; lptr++)
	{
		tl = *lptr;
		bp = getblock(tl,READ); nl = nleft; tl =& ~0177;
		while (*lp = *bp++)
		{
			if (toggle || nocmpress) lp++;
			else if (*lp != ' ' && *lp != CTRLI) { toggle++; lp++; }
			if (--nl == 0) { bp = getblock(tl=+0200,READ); nl = nleft; }
			if (lp >= &linebuf[LBSIZE-2]) error(27);
		}
		if (lp >= &linebuf[LBSIZE-1]) error(27);
		while (--lp >= linebuf && (*lp==' ' || *lp==CTRLI) && ! nocmpress);
		*++lp = ' '; lp++; toggle = 0;
	}
	lp[-1] = 0;
	retag(*addr1 = putline(linebuf), *addr1);
	addr1++;
	if (addr2 == dol) delete(); else { delete(); dot--; }
}
#endif	JOIN

delete()
{
	register *a1, *a2, *a3;

	nonzero();
	a1 = addr1;
	a2 = addr2+1;
	a3 = dol;
	dol =- a2 - a1;
	do *a1++ = *a2++; while (a2 <= a3);
	a1 = addr1;
	if (a1 > dol) a1 = dol;
	dot = a1;
}

getline(tl)
{
	register char *bp, *lp;
	register nl;

	lp = linebuf;
	bp = getblock(tl, READ);
	nl = nleft;
	tl =& ~0177;
	while (*lp++ = *bp++)
		if (--nl == 0)
		{
			bp = getblock(tl=+0200, READ);
			nl = nleft;
		}
	return(linebuf);
}

putline(buffer)
char	buffer[];
{
	register char *bp, *buff;
	register nl;
	int tl;

	buff = buffer;
	tl = tline;
	bp = getblock(tl, WRITE);
	nl = nleft;
	tl =& ~0177;
	while (*bp = *buff++)
	{
		if (*bp++ == '\n')
		{
			*--bp = 0;
			linebp = buff;
			break;
		}
		if (--nl == 0)
		{
			bp = getblock(tl=+0200, WRITE);
			nl = nleft;
		}
	}
	nl = tline;
	tline =+ (((buff-buffer)+07)>>2) & 077776;
	return(nl);
}

/*	format of core words :		bbbbbbbbboooooog
 *	where :
 *		b is block nr. in temp. file;
 *		o is (offset in block) / 8;
 *		g is set on global matches
 */

getblock(atl, iof)
{
	extern read(), write();
	register bno, off;

	bno = (atl>>7)&0777;
	off = (atl<<2)&0770;
	if (bno >= 511) error(29);
	nleft = 512 - off;
	if (bno==iblock) { ichanged =| iof; return(ibuff+off); }
	if (bno==oblock) return(obuff+off);
	if (iof==READ)
	{
		if (ichanged) blkio(iblock, ibuff, write);
		ichanged = 0;
		iblock = bno;
		blkio(bno, ibuff, read);
		return(ibuff+off);
	}
	if (oblock>=0) blkio(oblock, obuff, write);
	oblock = bno;
	return(obuff+off);
}

blkio(b, buf, iofcn)
int (*iofcn)();
{
	seek(tfile, b, 3);
	if ((*iofcn)(tfile, buf, 512) != 512) error(5);
}

init()
{
	register char *p;
	register pid;

	close(tfile);
	tline = 0;
	iblock = -1;
	oblock = -1;
	tfname = "/tmp/exxxxx";
	ichanged = 0;
	pid = getpid();
	for (p = &tfname[11]; p > &tfname[6];)
	{
		*--p = (pid&07) + '0';
		pid =>> 3;
	}
	if ((tfile = creat(tfname, 0600)) < 0) error(6);
	close(tfile);
	if ((tfile = open(tfname, 2)) < 0) error(7);
#ifndef	ERRORS
	brk(fendcore);
	endcore = fendcore - 2;
#else
	brk(fendcore+INITLINES);
	endcore = fendcore - 2 + INITLINES;
#endif	ERRORS
	dot = zero = dol = fendcore;
}

global(k)
{
	register char *gp;
	register c;
	register int *a1;
	char globuf[GBSIZE];
#ifdef	ADDRESS
	int dflt 0;
#endif

	if (globp) error(20);
	setall();
	nonzero();
	if ((c=getchar())=='\n') error(20);
#ifndef	ADDRESS
	compile(c);
#else
	if (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z')
	{
		peekc = c;
		dflt++;
	}
	else compile(c);
#endif	ADDRESS
	gp = globuf;
	while ((c = getchar()) != '\n')
	{
		if (c==EOF) error(20);
		if (c==BACKSL)
		{
			c = getchar();
			if (c!='\n') *gp++ = BACKSL;
		}
		*gp++ = c;
		if (gp >= &globuf[GBSIZE-2]) error(27);
	}
	*gp++ = '\n';
	*gp++ = 0;
#ifdef	GLOBAL
	c = 0;
#endif	GLOBAL
	for (a1=zero; a1<=dol; a1++)
	{
		*a1 =& ~01;
#ifndef	ADDRESS
		if (a1>=addr1 && a1<=addr2 && execute(0, a1)==k)
#else
		if (a1>=addr1 && a1<=addr2 && (dflt || execute(0, a1))==k)
#endif	ADDRESS
#ifndef	GLOBAL
			*a1 =| 01;
#else
			{ *a1 =| 01; c++; }
#endif	GLOBAL
	}
#ifdef	GLOBAL
	if (c == 0) error(21);
#endif	GLOBAL
	for (a1=zero; a1<=dol; a1++)
	{
		if (*a1 & 01)
		{
			*a1 =& ~01;
			dot = a1;
			globp = globuf;
			commands(-2);
			a1 = zero;
		}
	}
}

substitute(inglob)
{
#ifdef	BETTER_SWAP
	register char *sp, *lp;
#endif	BETTER_SWAP
	register *a1;
	int gsubf;
	int nn;
	int nflag;
	int getsub();

	gsubf = compsub();
	nflag = gsubf > 1;
	nn = 0;
	gsubf =& 01;
	gsubf =| xflag;
	for (a1 = addr1; a1 <= addr2; a1++)
	{
		if (execute(0, a1)==0) continue;
		inglob =| 01;
#ifdef	BETTER_SWAP
		ssp = genbuf; slp = linebuf;
#endif	BETTER_SWAP
		if (confirmed()) { dosub(); nn++; }
		else donothing();
		if (gsubf)
		{
			while (*loc2)
			{
				if (execute(1)==0) break;
				if (confirmed()) { dosub(); nn++; }
				else donothing();
			}
		}
#ifdef	BETTER_SWAP
		sp = ssp; lp = slp;
		while (*sp++ = *lp++) if (sp >= &genbuf[LBSIZE]) error(27);
		retag(*a1 = putline(genbuf), *a1);
#else
		retag(*a1 = putline(linebuf), *a1);
#endif	BETTER_SWAP
		append(getsub, a1);
		a1 =+ nline;
		addr2 =+ nline;
	}
	if (inglob==0) error(21);
	if (nflag)
	{
		putchar(' ');
		count = nn;
		putd();
		putchar('\n');
	}
}
