/*
   Jonathan Payne at Lincoln-Sudbury Regional High School 5-25-83
  
   jove_misc.c

   Miscellanious commands... */

#include "jove.h"
#include <signal.h>
#include <ctype.h>

ChrToOct()
{
	int	c = getch();

	ins_str(sprint("\\%03o", c));
}

char *
StrIndex(dir, buf, charpos, what)
char	*buf,
	what;
{
	char	*cp = &buf[charpos],
		c;

	if (dir > 0) {
		while (c = *cp++)
			if (c == what)
				return (cp - 1);
	} else {
		while (cp >= buf && (c = *cp--))
			if (c == what)
				return (cp + 1);
	}
	return 0;
}

StrLength()
{
	static char	inquotes[] = "Where are the quotes?";
	char	*first = StrIndex(-1, linebuf, curchar, '"');
	char	*last = StrIndex(1, linebuf, curchar, '"');
	char	c;
	int	numchars = 0;

	if (first == 0 || last == 0)
		complain(inquotes);
	first++;
	while (first < last) {
		c = *first++;
		if (c == '\\') {
			int	num;

			if (*first < '0' || *first > '9')
				++first;
			else {
				num = 3;
				while (num-- && (c = *first++) >= '0' &&
						c <= '9' && first < last)
					;
			}
		}
		numchars++;
	}
	s_mess("%d characters", numchars);
}

SplitWind()
{
	curwind = div_wind(curwind);
}

LINE *
lastline(lp)
register LINE	*lp;
{
	while (lp->l_next)
		lp = lp->l_next;
	return lp;
}

/* Transpos cur_char with cur_char - 1 */

TransChar()
{
	char	c;

	if (curchar == 0)
		complain((char *) 0);	/* BEEP */
	c = linebuf[curchar - 1];
	exp = 1;
	DelPChar();
	if (eolp()) {
		BackChar();
		Insert(c);
		ForChar();
	} else {
		ForChar();
		Insert(c);
		BackChar();
	}
}

SetLine(line)
register LINE	*line;
{
	DotTo(line, 0);
}

UpScroll()
{
	SetTop(curwind, next_line(curwind->w_top, exp));
	if (in_window(curwind, curline) == -1)
		SetLine(next_line(curwind->w_top, HALF(curwind)));
}

DownScroll()
{
	SetTop(curwind, prev_line(curwind->w_top, exp));
	if (in_window(curwind, curline) == -1)
		SetLine(next_line(curwind->w_top, HALF(curwind)));
}

Leave()
{
	BUFFER	*bp;

	if (RecDepth == 0)
		for (bp = world; bp; bp = bp->b_next)
			if ((bp->b_zero) && IsModified(bp) &&
					(bp->b_type != SCRATCHBUF)) {
				confirm("Modified buffers exist.  Leave anyway? ");
				break;
			}
	longjmp(mainjmp, QUIT);
}

KillEOL()
{
	LINE	*line2;
	int	char2;

	if (exp_p) {
		line2 = next_line(curline, exp);
		if (line2 == curline)
			char2 = length(curline);
		else
			char2 = 0;
	} else if (linebuf[curchar] == '\0') {
		line2 = next_line(curline, 1);
		if (line2 == curline)
			char2 = length(curline);
		else
			char2 = 0;
	} else {
		line2 = curline;
		char2 = length(curline);
	}
	reg_kill(curline, curchar, line2, char2, 0);
}

CtlxPrefix()
{
	struct function	*fp;
	int	c,
		invokingchar = LastKeyStruck;

	c = (*Getchar)();

	if (c == CTL(G)) {
		PrefAbort("Prefix-2");
		return;
	}
	fp = pref2map[c];
	if (fp == 0)
		Unknown(invokingchar, c);
	ExecFunc(fp, 0);
}

Yank()
{
	LINE	*line,
		*lp;
	BUFLOC	*dot;

	if (killbuf[killptr] == 0)
		complain("Nothing to yank!");
	lsave();
	this_cmd = YANKCMD;
	line = killbuf[killptr];
	lp = lastline(line);
	dot = DoYank(line, 0, lp, length(lp), curline, curchar, curbuf);
	SetMark();
	SetDot(dot);
}

static	NumArg = 1;

GetFour(Input)
int	(*Input)();
{
	register int	c;

	do {
		NumArg *= 4;
	} while ((c = (*Input)()) == CTL(U));
	return c;
}

GetArg(Input)
int	(*Input)();
{
	register int	c,
			i = 0;

	if (!isdigit(c = (*Input)()))
		return (c | 0200);

	do
		i = i * 10 + (c - '0');
	while ((c = getch()) >= '0' && c <= '9');
	NumArg = i;
	return c;
}

char *
jumpup(cp)
char	*cp;
{
	while (*cp && !isword(*cp))
		cp++;
	while (*cp && isword(*cp))
		cp++;
	return cp;
}

char *
backup(string, cp)
char	*string, *cp;
{
	while (cp > string && !isword(*(cp - 1)))
		cp--;
	while (cp > string && isword(*(cp - 1)))
		cp--;
	return cp;
}

#define ASKSIZE	132

char *
RunEdit(c, begin, cp, def, HowToRead)
register int	c;
register char	*begin,
		*cp;
char	*def;
int	(*HowToRead)();
{
	char	*tcp;

	switch (c) {
	case CTL(@):
		break;

	case CTL(A):
		cp = begin;
		break;

	case CTL(B):
		if (cp > begin)
			--cp;
		break;

	case CTL(D):
		if (!*cp)
			break;
		cp++;
		goto delchar;

	case CTL(E):
		while (*cp)
			cp++;
		break;

	case CTL(F):
		if (*cp)
			cp++;
		break;

	case CTL(G):
		return (char *) -1;

	case CTL(K):
		*cp = '\0';
		break;

	case CTL(N):
	case CTL(P):
		ArgIns(begin, c == CTL(N));
		cp = begin + strlen(begin);
		break;

	case '\177':
	case CTL(H):
delchar:
		if (cp == begin)
			break;
		strcpy(cp - 1, cp);
		cp--;
		break;

	case META(\177):	/* Delete previous word */
	case CTL(W):
	    {
		char	*tcp = backup(begin, cp);

		strcpy(tcp, cp);
		cp = tcp;
	    }
		break;

	case META(D):
	case META(d):
		strcpy(cp, jumpup(cp));
		break;		/* Pretty neat huh */

	case META(B):
	case META(b):
		cp = backup(begin, cp);
		break;

	case META(F):
	case META(f):
		cp = jumpup(cp);
		break;

	case CTL(R):
		if (!def || !*def) {
			rbell();
			break;
		}
		tcp = def;
		while (c = *tcp++) {
			insert(c, begin, cp - begin, 1, LBSIZE);
			cp++;
		}
		break;

	case CTL(^):
	case CTL(Q):
		c = (*HowToRead)();
		/* Fall into... */

	default:
		if (c & 0200)
			rbell();
		else {
			insert(c, begin, cp - begin, 1, ASKSIZE);
			cp++;
		}
	}
	return cp;
}

NoMacGetc()
{
	int	c;

	redisplay();
	if ((c = getchar()) == EOF)
		finish(SIGHUP);
	return c & 0177;
}

extern int	Interactive;

/* VARARGS2 */

char *
ask(def, fmt, args)
char	*def,
	*fmt;
{
	static char	string[ASKSIZE];
	char	*cp,
		*begin;	/* Beginning of real text */
	int	c;
	extern int	(*Getchar)();
	int	(*HowToRead)() = Interactive ? NoMacGetc : Getchar;

	format(string, fmt, &args);
	message(string);
	Asking = strlen(string);	/* Entirely for redisplay */
	begin = string + Asking;
	cp = begin;
	*cp = '\0';

	while ((c = (*HowToRead)()) != '\r' && c != '\n') {
		NumArg = 1;	/* If not otherwise specified */
		if (c == '\033')	/* Maybe a number */
			c = GetArg(HowToRead);
		else if (c == CTL(U))
			c = GetFour(HowToRead);
		if (c == '\033')	/* Again */
			c = (*HowToRead)() | 0200;
		while (NumArg-- > 0)
			if ((int)(cp = RunEdit(c, begin, cp, def, HowToRead)) == -1)
				complain("Aborted");

		/* Show the change */
		message(string);
		Asking = cp - string;
	}
	Asking = 0;
	if (cp == begin && *cp == '\0')	/* Nothing was typed */
		if (def == 0)
			complain("No default");
		else
			return def;
	return begin;
}

YankPop()
{
	LINE	*line,
		*last;
	MARK	*mp = CurMark();
	BUFLOC	*dot;

	if (last_cmd != YANKCMD)
		complain("Yank something first!");

	lfreelist(reg_delete(mp->m_line, mp->m_char, curline, curchar));

	/* Now must find a recently killed region. */

	killptr--;
	for (;;) {
		if (killptr < 0)
			killptr = NUMKILLS - 1;
		if (killbuf[killptr])
			break;
		killptr--;
	}

	this_cmd = YANKCMD;

	line = killbuf[killptr];
	last = lastline(line);
	dot = DoYank(line, 0, last, length(last), curline, curchar, curbuf);
	MarkSet(CurMark(), curline, curchar);
	SetDot(dot);
}

WtModBuf()
{
	int	askp = exp_p;
	BUFFER	*oldb = curbuf,	
		*bp;		
	char	*yorn,
		name[100];

	for (bp = world; bp; bp = bp->b_next) {
		if (bp->b_zero == 0 || !IsModified(bp) ||
					bp->b_type == SCRATCHBUF)
			continue;
		SetBuf(bp);	/* Make this current buffer */
		if (Crashing) {
			strcpy(name, curbuf->b_name);
			insert('_', name, 0, 1, 100);
		} else {
			if (curbuf->b_fname == 0)
				setfname(curbuf, ask((char *) 0, "No file.  File to use: "));
			strcpy(name, curbuf->b_fname);
		}
		if (askp) {
			yorn = ask((char *) 0, "Write %s? ", curbuf->b_fname);
			if (*yorn != 'Y' && *yorn != 'y')
				continue;
		}
		file_write(name, 0);
		SetUnmodified(curbuf);
	}
	SetBuf(oldb);
}

NotModified()
{
	SetUnmodified(curbuf);
}

ArgIns(at, next)
char	*at;
{
	int	inc = next ? 1 : -1;

	if (*(argvp + inc) == 0)
		inc = 0;
	argvp += inc;
	strcpy(at, *argvp);
}

blnkp(buf)
register char	*buf;
{
	register char	c;

	while ((c = *buf++) && (c == ' ' || c == '\t'))
		;
	return c == 0;	/* It's zero if we got to the end of the line */
}

DelBlnkLines()
{
	exp = 1;
	if (!blnkp(linebuf) && !eolp())
		return;
	while (blnkp(linebuf) && curline->l_prev)
		SetLine(curline->l_prev);

	Eol();
	DelWtSpace();
	NextLine();
	while (blnkp(linebuf) && !eobp()) {
		DelWtSpace();
		DelNChar();
	}
	DelWtSpace();
}
