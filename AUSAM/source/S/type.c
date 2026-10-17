/*
 *	Program to help typists.  Uses backspace as erase character, and
 *	'@' as kill character. Uses raw-mode, and is best therefore on
 *	a lightly-loaded machine.  If the named file is already present,
 *	seeks to the end before adding to it.
 *	Tabs are assumed set on every 8 columns.
 *	Many of the features of this program were dictated by the terminals
 *	on which it is used - there is no easy way to clear the current
 *	line; the `DEL' key is too close to the `RETURN' key and thus had
 *	to be ignored; and the `CONTROL' key is too close to the `SHIFT'
 *	key and is similarly ignored (by default).
 *
 *					Kevin Hill  (Dec 1978)
 */


#include	<gtty.h>

struct sgttyb ttybuf;


#define	LSZ	40		/* default max. line size */
#define	MAX	6		/* default max chars before auto-adjust */

#define	CTRLE	'\05'		/* control-e */
#define	CTRLL	'\014'		/* control-l */

char line[256];			/* line buffer */
int fd;				/* file being written onto */
char peekc;			/* one character look-ahead */
char *name;			/* points to actual file name */
int longline;			/* when set by ^l, allows long line */
int punctuation;		/* indicates last character was .:;!? */
int lnsize	LSZ;		/* max. line size */
int max 	MAX;		/* max chars after lnsize before auto-adjust */
char aflag;			/* suppress auto-adjust when set */
char bflag;			/* rings bell for every char after lnsize */
char cflag;			/* allows entry of control characters */

char sttystrng[]	"stty";

main(ac, av)
register char *av[];
{
	int mexit();

	signal(3, 1);
	signal(14, mexit);
	if (gtty(1, &ttybuf) == -1 || gtty(0, &ttybuf) == -1)
		userr("redirection of input/output not allowed");
	while (--ac)
	{
		if (**++av == '-')
		switch((*av)[1])
		{
	    case 'a':	aflag++; continue;

	    case 'b':	bflag++; continue;

	    case 'c':	cflag++; continue;

	    case 'l':	if (--ac && (lnsize = getnum(*++av, 30, 140))) continue;
			userr("line size ?");

	    case 'm':	if (--ac && (max = getnum(*++av, 1, 20))) continue;
			userr("auto-adjust size ?");

	    default:	userr("illegal flag");
		}
		if (fd) userr("file already specified");
		if ((fd = open((name = *av), 1)) < 0)
		{
			if ((fd = creat(name, 0600)) < 0)
			{
				perror(name);
				return;
			}
		}
		else if (seek(fd, 0, 2) < 0) { perror("seek"); return; }
	}
	if (fd == 0) userr("file ?");
	if (ttybuf.erase != '\b' || ttybuf.kill != '@')
	{
		prints(2, "\007Warning: your erase and kill characters have been changed\n");
		prints(2, "They are now back-space (BS) and @ respectively.\n");
	}
	initty();
	grab();
}

grab()
{
	register count;			/* counts the printable chars on a line */
	register char *lp, *s;
	char c;

	count = 0;
	*(lp = line) = 0;
	for (;;)
	{
		switch(c = getch())
		{
	    case '.':
	    case ':':
	    case ';':
	    case '!':
	    case '?':	putchar(*++lp = c);
			if ((peekc = getch()) == ' ' || peekc == '\t') punctuation++;
			c = 0;
			goto DFLTP1;

	    case CTRLE:
	    case 004:	if (*lp) { writeline(lp, 1); putchar('\n'); }
			if (c == 004) mexit();
			lp = line; count = 0;
			edit();
			continue;

	    case '\t':	count = ((count + 8) & ~07) - 1;

	    case ' ':	if (count < lnsize || longline) goto DFLT;
			putchar('\n');

	    newline:
	    case '\n':	writeline(lp, ! longline);
			lp = line;
			count = 0;
			continue;

	    case '\b':	if (lp != line)
				if (*lp == '\t')
				{
					s = count - fixup(lp--, 0);
					count =- s;
					while (s--) putchar('\b');
				}
				else if (*lp == 0) count = fixup(lp--, 1);
				else if (*lp-- >= ' ')
				{
					prints(1, "\b \b");
					count--;
				}
	    case 0177:	continue;

	    case CTRLL:	longline++; continue;

	    case '@':	putchar('\r');
			while (count--) putchar(' ');
			putchar('\r');
			c = 0;
			longline = 0;
			goto DFLT;

	    case '\\':	putchar('\\');
			if ((c = getch()) == '@' || c == '\\') putchar('\b');
			else
			{
				*++lp = '\\';
				count++;
				peekc = c;
				continue;
			}

	    default:	if (c < ' ')		/* unprintable control char */
				if (! cflag)
				{
					putchar('\007');
					continue;
				}
				else count--;
			else if (punctuation && ! longline)
			{
				writeline(lp, 1);
				putchar('\n');
				lp = line;
				count = 0;
			}

	    DFLT:	*++lp = c;
	    DFLTP1:	if (++count > lnsize && bflag && ! longline) putchar('\007');
			if (! aflag && count > lnsize + max && ! longline)
			{
				s = lp;
				while (*--s != ' ' && *s != '\t' && *s);
				if (*s)
				{
					writeline(--s, 1);
					*++lp = 0;
					lp = count - fixup(++s, 0) - 1;
					for (count = 0; count < lp; count++) putchar('\b');
					for (count = 0; count < lp; count++) putchar(' ');
					lp = line;
					putchar('\n');
					while (*++lp = *++s) putchar(*s);
					lp--;
					continue;
				}
			}
			if (lp >= &line[sizeof line - 2])
			{
				prints(1, "\nLine too long\n");
				goto newline;
			}
		}
		if (c) putchar(c);
	}
}


getch()
{
	char c;

	if (c = peekc) { peekc = 0; return c; }
	if (read(0, &c, 1) != 1)
	{
		perror("read");
		mexit();
	}
	return c;
}


userr(s)
register char *s;
{
	prints(2, s);
	prints(2, "\n\007Usage: type [-a] [-b] [-c] [-l line-size] [-m auto-adjust size] file\n");
	exit(1);
}

mexit()
{
	if (stty(0, &ttybuf) < 0) perror(sttystrng);
	exit(0);
}


initty()
{
	register fred;

	fred = ttybuf.mode;
	ttybuf.mode =| RAW;
	ttybuf.mode =& ~ECHO;
	if (stty(0, &ttybuf) < 0) { perror(sttystrng); exit(); }
	ttybuf.mode = fred;
	prints(1, "\007Go ahead\n");	/* indicates the stty is finished */
}


writeline(lp, noblanks)
register char *lp;
register noblanks;
{
	register count;

	if (noblanks) while (*lp == ' ' || *lp == '\t') lp--;
	*++lp = '\n';
	count = 1;
	while (*--lp) count++;
	if (write(fd, ++lp, count) != count)
	{
		perror("write");
		mexit();
	}
	longline = 0;
	punctuation = 0;
}

getnum(s, lo, hi)
register char *s;
register hi;
{
	register n;

	n = 0;
	while (*s >= '0' && *s <= '9' && (n = n * 10 + *s - '0') <= hi) s++;
	if (*s == 0 && n >= lo) return n;
	return 0;
}

fixup(lp, print)
register char *lp;
{
	register count;
	register char *s;

	s = lp;
	count = 0;
	while (*--s);
	while (++s != lp)
	{
		if (print) putchar(*s);
		if (*s == '\t') count = (count + 8) & ~07;
		else if (*s >= ' ') count++;
	}
	return count;
}


edit()
{
	int pid;

	if (stty(0, &ttybuf) < 0) { perror(sttystrng); exit(1); }
	signal(2, 1);
	if ((pid = fork()) < 0) return perror("fork");
	if (pid == 0)		/* child */
	{
		execl("/bin/em", "em", "-e", name, 0);
		perror("em");
		exit(1);
	}
	waitx(&pid);
	seek(fd, 0, 2);
	initty();
}
