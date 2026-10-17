/*
   Jonathan Payne at Lincoln-Sudbury Regional High School 5-25-83
  
   jov_move.c

   Commands that move the point around in the buffer. */

#include "jove.h"
#include <ctype.h>

int line_pos;

LINE *
next_line(line, num)
register LINE	*line;
register int	num;
{
	register int	i;

	if (line)
		for (i = 0; i < num && line->l_next; i++, line = line->l_next)
			;

	return line;
}

LINE *
prev_line(line, num)
register LINE	*line;
register int	num;
{
	register int	i;

	if (line)
		for (i = 0; i < num && line->l_prev; i++, line = line->l_prev)
			;

	return line;
}

ForChar()
{
	register int	num = exp;

	exp = 1;
	while (num--) {
		if (eolp()) {			/* Go to the next line */
			if (curline->l_next == 0)
				break;
			SetLine(curline->l_next);
		} else
			curchar++;
	}
}

BackChar()
{
	register int	num = exp;

	exp = 1;
	while (num--) {
		if (bolp()) {
			if (curline->l_prev == 0)
				break;
			SetLine(curline->l_prev);
			Eol();
		} else
			--curchar;
	}
}

NextLine()
{
	if (lastp(curline))
		OutOfBounds();
	down_line(1);
}

PrevLine()
{
	if (firstp(curline))
		OutOfBounds();
	down_line(0);
}

OutOfBounds()
{
	complain("");
}

down_line(down)
{
	LINE	*(*func)() = down ? next_line : prev_line;
	LINE	*line;

	line = (*func)(curline, exp);

	this_cmd = LINE_CMD;

	if (last_cmd != LINE_CMD)
		line_pos = calc_pos(linebuf, curchar);

	SetLine(line);		/* Curline is in linebuf now */
	curchar = how_far(curline, line_pos);
}

/* Returns what cur_char should be for that pos. */

how_far(line, ypos)
LINE	*line;
{
	register char	*pp;
	char	*base;
	register int	cur_char;
	char	c;
	register int	y;

	base = pp = getcptr(line, genbuf);

	cur_char = 0;
	y = 0;

	while (c = *pp++) {
		if (y >= ypos)
			return cur_char;
		if (c == 0177)
			y++;
		else if (c < ' ') {
			if (c == 011)
				y += ((tabstop - y % tabstop) - 1);
			else
				y++;
		}
		y++;
		cur_char++;
	}

	return pp - base - 1;
}

Bol()
{
	curchar = 0;
}

Eol()
{
	curchar += strlen(&linebuf[curchar]);
}

DotTo(line, col)
LINE	*line;
{
	BUFLOC	bp;

	bp.p_line = line;
	bp.p_char = col;
	SetDot(&bp);
}

/* If bp->p_line is != current line, then save current line.  Then set dot
   to bp->p_line, and if they weren't equal get that line into linebuf  */

SetDot(bp)
register BUFLOC	*bp;
{
	register int	notequal = bp->p_line != curline;

	if (notequal)
		lsave();
	curline = bp->p_line;
	curchar = bp->p_char;
	if (notequal)
		getDOT();
}

Eof()
{
	SetMark();
	SetLine(curbuf->b_dol);
	Eol();
}

Bof()
{
	SetMark();
	SetLine(curbuf->b_zero);
}

char	REsent[] = "[?.!]\"  *\\|[.?!]  *\\|[.?!][\"]*$";

Bos()
{
	int	num = exp;
	BUFLOC	*bp,
		save;

	exp = 1;
	while (num--) {
onceagain:
		bp = dosearch(REsent, -1, 1);
		DOTsave(&save);
		if (bp == 0) {
			Bof();
			break;
		}
		SetDot(bp);
		to_word(1);
		if (curline == save.p_line && curchar == save.p_char) {
			SetDot(bp);
			goto onceagain;
		}
	}
}

Eos()
{
	int	num = exp;
	BUFLOC	*bp;

	exp = 1;
	while (num-- && (bp = dosearch(REsent, 1, 1)))
		SetDot(bp);
	if (bp == 0)
		Eof();
	else
		to_word(1);
}

length(line)
register LINE	*line;
{
	register char	*base,
			*cp;

	base = cp = getcptr(line, genbuf);

	while (*cp++)
		;
	return cp - base - 1;
}

isword(c)
register char	c;
{
	return isalpha(c) || isdigit(c);
}

to_word(dir)
{
	register char	c;

	if (dir > 0) {
		while ((c = linebuf[curchar]) != 0 && !isword(c))
			curchar++;
		if (eolp()) {
			if (curline->l_next == 0)
				return;
			SetLine(curline->l_next);
			to_word(dir);
			return;
		}
	} else {
		while (!bolp() && (c = linebuf[curchar - 1], !isword(c)))
			--curchar;
		if (bolp()) {
			if (curline->l_prev == 0)
				return;
			SetLine(curline->l_prev);
			Eol();
			to_word(dir);
		}
	}
}

ForWord()
{
	register char	c;
	register int	num = exp;

	exp = 1;
	while (--num >= 0) {
		to_word(1);
		while ((c = linebuf[curchar]) != 0 && isword(c))
			curchar++;
		if (eobp())
			break;
	}
	this_cmd = 0;	/* Semi kludge to stop some unfavorable behavior */
}

BackWord()
{
	register int	num = exp;
	register char	c;

	exp = 1;
	while (--num >= 0) {
		to_word(-1);
		while (!bolp() && (c = linebuf[curchar - 1], isword(c)))
			--curchar;
		if (bobp())
			break;
	}
	this_cmd = 0;
}

/* End of window */

Eow()
{
	SetLine(next_line(curwind->w_top, SIZE(curwind) - 1 -
			min(SIZE(curwind) - 1, exp - 1)));
	if (!exp_p)
		Eol();
}

/* Beginning of window */

Bow()
{
	SetLine(next_line(curwind->w_top, min(SIZE(curwind) - 1, exp - 1)));
}

