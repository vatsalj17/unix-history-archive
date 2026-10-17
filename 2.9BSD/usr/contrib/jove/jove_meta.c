/*
   Jonathan Payne at Lincoln-Sudbury Regional High School 5-25-83
  
   jove_meta.c

   Various commands that are (by default) invoked by ESC-key.
   Not a very good name since the ESC key can be rebound...  */

#include "jove.h"
#include <ctype.h>

PrefAbort(str)
char	*str;
{
	s_mess("%s aborted", str);
	rbell();
}

Unknown(pref, c)
char	pref,
	c;
{
	s_mess("%c-%c unbound", pref, c);
	rbell();
}

Upper(c)
register int	c;
{
	return (islower(c) ? toupper(c) : c);
}

FourTime()
{
	exp_p = 1;
	exp *= 4;
	this_cmd = ARG_CMD;
}

EscPrefix()
{
	register int	c, i, sign;
	struct function	*mp;
	int	invokingchar = LastKeyStruck;

	c = (*Getchar)();

	if (c == CTL(G)) {
		PrefAbort("Prefix-1");
		return;
	}

	if (c == '-' || (c >= '0' && c <= '9')) {
		sign = c == '-' ? -1 : 1;
		if (sign == 1)
			i = c - '0';
		else
			i = 0;
		for (;;) {
			c = (*Getchar)();
			if (c >= '0' && c <= '9')
				i = i * 10 + (c - '0');
			else {
				exp_p = 1;
				exp = i * sign;
				peekc = c;
				this_cmd = ARG_CMD;
				return;
			}
		}
	}

	mp = pref1map[c];
	if (mp == 0)
		Unknown(invokingchar, c);
	ExecFunc(mp, 0);
}

/*
 * Save a region.  A pretend kill.
 */

CopyRegion()
{
	LINE	*nl;
	MARK	*mp;
	int	status,
		mod = curbuf->b_modified;

	mp = CurMark();

	if (mp->m_line == curline && mp->m_char == curchar)
		complain((char *) 0);

	killptr = ((killptr + 1) % NUMKILLS);
	if (killbuf[killptr])
		lfreelist(killbuf[killptr]);
	nl = killbuf[killptr] = nbufline();
	nl->l_dline = putline("");
	nl->l_next = nl->l_prev = 0;

	status = inorder(mp->m_line, mp->m_char, curline, curchar);
	if (status == -1)
		return;

	if (status)
		ignore(DoYank(mp->m_line, mp->m_char, curline, curchar,
				nl, 0, (BUFFER *) 0));
	else
		ignore(DoYank(curline, curchar, mp->m_line, mp->m_char,
				nl, 0, (BUFFER *) 0));
	curbuf->b_modified = mod;
}

DelNWord()
{
	dword(1);
}

DelPWord()
{
	dword(0);
}

dword(forward)
{
	BUFLOC	savedot;

	DOTsave(&savedot);
	forward ? ForWord() : BackWord();
	reg_kill(savedot.p_line, savedot.p_char, curline, curchar, !forward);
}

UppWord()
{
	case_word(1);
}

LowWord()
{
	case_word(0);
}

ToIndent()
{
	register char	*cp, c;

	for (cp = linebuf; c = *cp; cp++)
		if (c != ' ' && c != '\t')
			break;
	curchar = cp - linebuf;
}

GoLine()
{
	if (exp_p == 0)
		return;
	SetLine(next_line(curbuf->b_zero, exp - 1));
}

VtKeys()
{
	int	c = getch();

	switch (c) {
	case 'A':
		PrevLine();
		break;

	case 'B':
		NextLine();
		break;

	case 'D':
		BackChar();
		break;

	case 'C':
		ForChar();
		break;

	default:
		complain("Unknown command ESC-[-%c", c);
	}
}

int	RMargin = RMARGIN;

Justify()
{
	BUFLOC	*bp;
	LINE	*start,
		*end;
	char	*ParaStr = "^\\.\\|^$";

	bp = dosearch(ParaStr, -1, 1);	/* Beginning of paragraph */
	if (bp)		/* Not the tab case */
		start = bp->p_line->l_next;
	else
		start = curbuf->b_zero;
	bp = dosearch(ParaStr, 1, 1);		/* End of paragraph */
	if (bp)
		end = bp->p_line;
	else
		end = curbuf->b_dol;
	DoJustify(start, end);
}

extern int	diffnum;

DoJustify(l1, l2)		/* Justify text */
LINE	*l1,
	*l2;
{
	int	d1 = 0,
		d2 = 1,
		goal,
		nlines,			/* Number of lines to justify
					 * so we know when we are done.
					 */
		curcol;
	char	*cp;
	MARK	*savedot = MakeMark(curline, curchar);

	fixorder(&l1, &d1, &l2, &d2);	/* l1/c1 will be before l2/c2 */
	nlines = diffnum;
	SetLine(l1);

	if (lastp(l2) && !blnkp(getline(l2->l_dline, genbuf)))
		nlines++;

	while (nlines) {	/* One line at a time */
		goal = RMargin - (RMargin / 10);
		Eol();	/* End of line */
		if ((curcol = calc_pos(linebuf, curchar)) == 0) {
			SetLine(curline->l_next);	/* Skip blank lines */
			nlines--;
		} else if (curcol < goal) {
			if (nlines == 1)
				break;
			DelWtSpace();	/* No white space at the end of the line */
			DelNChar();	/* Delete carraige return */
			nlines--;
			Insert(' ');
		} else if (curcol > RMargin) {
			do {
				/* curchar - 2 because curchar is the null,
				 * curchar - 1 is the space (if there is
				 * one there.
				 */
				cp = StrIndex(-1, linebuf, curchar - 2, ' ');
				if (cp == 0)
					break;
				curchar = cp - linebuf;
				curcol = calc_pos(linebuf, curchar);
			} while (curcol > RMargin);
			if (cp) {
				DelWtSpace();
				LineInsert();
			} else {
				SetLine(curline->l_next);
				nlines--;
			}
		} else {
			SetLine(curline->l_next);
			nlines--;
		}
	}
	ToMark(savedot);	/* Back to where we were */
	DelMark(savedot);	/* Free up the mark */
	this_cmd = 0;		/* So everything is under control */
}
