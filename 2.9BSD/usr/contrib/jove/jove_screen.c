/*
   Jonathan Payne at Lincoln-Sudbury Regional High School 5-25-83

   jove_screen.c

   Deals with writing output to the screen optimally i.e. it doesn't
   write what is already there.  It keeps an exact image of the screen
   in the Screen array.  */

#include "jove.h"
#include "jove_temp.h"
#include "termcap.h"

extern int	BufSize;

int	CheckTime,
	tabstop = 8;

struct scrimage
	*nimage,	/* What lines should be where after redisplay */
	*oimage;	/* What line are after redisplay */

struct screenline	*Screen,	/* The screen */
			*Curline;	/* Current line */
char	*cursor,			/* Offset into current line */
	*cursend;

int	CapCol,
	CapLine,

	i_line,
	i_col;

make_scr()
{
	register int	i;
	register struct screenline	*ns;
	register char	*nsp;

	nimage = (struct scrimage *) emalloc(LI * sizeof (struct scrimage));
	oimage = (struct scrimage *) emalloc(LI * sizeof (struct scrimage));

	ns = Screen = (struct screenline *)
			emalloc(LI * sizeof(struct screenline));

	nsp = (char *) emalloc(CO * LI);

	for (i = 0; i < LI; i++) {
		ns->s_line = nsp;
		nsp += CO;
		ns->s_length = nsp - 1;		/* End of line */
		ns++;
	}
	cl_scr();
}

clrline(cp1, cp2)
register char	*cp1,
		*cp2;
{
	while (cp1 <= cp2)
		*cp1++ = ' ';
}

cl_eol()
{
	if (InputPending || cursor > cursend)
		return;

	if (cursor < Curline->s_length) {
		clrline(cursor, Curline->s_length);
		Curline->s_length = cursor;
		Placur(i_line, i_col);
		putpad(CE, 1);
	}
}

cl_scr()
{
	register int	i;
	register struct screenline	*sp = Screen;

	for (i = 0; i < LI; i++, sp++) {
		clrline(sp->s_line, sp->s_length);
		sp->s_length = sp->s_line;
		oimage[i].Line = 0;
	}
	putpad(CL, LI);
	Placur(0, 0);
	UpdMesg++;
}

#define soutputc(c)	if (--n > 0) sputc(c); else { sputc('!'); goto outahere;}

/* Output one character (if necessary) at the current position */

#define sputc(c)	((*cursor != c) ? dosputc(c) : (cursor++, i_col++))

dosputc(c)
register char	c;
{
	if (*cursor != c) {
		Placur(i_line, i_col);
		outchar(c);
		CapCol++;
		*cursor++ = c;
		i_col++;
	} else {
		cursor++;
		i_col++;
	}
}

/* Write `line' at the current position of `cursor'.  Stop when we
   reach the end of the screen.  Aborts if there is a character
   waiting.  */

swrite(line)
register char	*line;
{
	register char	c;
	int	col = 0,
		aborted = 0;
	register int	n = cursend - cursor;

	while (c = *line++) {
		if (CheckTime) {
			flusho();
			CheckTime = 0;
			if (InputPending = charp()) {
				aborted = 1;
				break;
			}
		}
		if (c == '\t') {
			int	nchars;

			nchars = (tabstop - (col % tabstop));
			col += nchars;

			while (nchars--)
				soutputc(' ')
		} else if (c < 040 || c == '\177') {
			soutputc('^')
			soutputc(c == '\177' ? '?' : c + '@')
			col += 2;
		} else {
			soutputc(c)
			col++;
		}
	}
outahere:
	if (cursor > Curline->s_length)
		Curline->s_length = cursor;
	return !aborted;
}

/* This is for writing a buffer line to the screen.  This is to
 * minimize the amount of copying from one buffer to another buffer.
 * This gets the info directly from ibuff[12].
 */

BufSwrite(linenum)
{
	register char	c,
			*bp;
	LINE	*lp = nimage[linenum].Line;
	register int	n = cursend - cursor;
	int	tl = lp->l_dline,
		nl,
		col = 0,
		StartCol = nimage[linenum].StartCol,
		aborted = 0;

#define OkayOut(c)	if (col++ >= StartCol) soutputc(c) else

	if (lp == curline) {
		bp = linebuf;
		nl = BUFSIZ;
	} else {
		bp = getblock(tl, READ);
		nl = nleft;
		tl &= ~OFFMSK;
	}

	while (c = *bp++) {
		if (CheckTime) {
			flusho();
			CheckTime = 0;
			if (InputPending = charp()) {
				aborted = 1;
				break;
			}
		}
		if (c == '\t') {
			int	nchars;

			nchars = (tabstop - (col % tabstop));

			while (nchars--)
				OkayOut(' ');

		} else if (c < 040 || c == '\177') {
			OkayOut('^');
			OkayOut(c == '\177' ? '?' : c + '@');
		} else
			OkayOut(c);

		if (--nl == 0) {
			bp = getblock(tl += INCRMT, READ);
			nl = nleft;
		}
	}
outahere:
	if (cursor > Curline->s_length)
		Curline->s_length = cursor;
	return !aborted;		/* Didn't abort */
}

putstr(str)
register char	*str;
{
	register char	c;

	while (c = *str++)
		outchar(c);
}

i_set(nline, ncol)
register int	nline,
		ncol;
{
	Curline = &Screen[nline];
	cursor = Curline->s_line + ncol;
	cursend = &Curline->s_line[CO - 1];
	i_line = nline;
	i_col = ncol;
}

extern int	diffnum;

/* Insert `num' lines a top, but leave all the lines BELOW `bottom'
 * alone (at least they won't look any different when we are done).
 * This changes the screen array AND does the physical changes.
 */

v_ins_line(num, top, bottom)
{
	register int	i;
	struct screenline	savelines[MAXNLINES];

	/* Save the screen pointers. */

	for(i = 0; i < num && top + i <= bottom; i++)
		savelines[i] = Screen[bottom - i];

	/* Num number of bottom lines will be lost.
	 * Copy everything down num number of times.
	 */

	for (i = bottom; i > top && i-num >= 0; i--) {
		Screen[i] = Screen[i - num];
		Screen[i] = Screen[i - num];
	}

	/* Restore the saved ones, making them blank. */

	for (i = 0; i < num; i++) {
		Screen[top + i] = savelines[i];
		clrline(Screen[top + i].s_line, Screen[top + i].s_length);
	}

	/* VT100/bitgraph are special cases ... uhg! */

        if (BG) {	/* But it makes such a big difference built in */
		putstr(sprint("\033[%d;%dr\033[%dL\033[r",
		  top + 1, bottom + 1, num));
		CapCol = CapLine = 0;
	} else if (VT) {
		Placur(top, 0);
		putstr(sprint("\0337\033[%d;%dr\0338", top + 1, bottom + 1));
		for (i = 0; i < num; i++)
			putstr(SR);
		putstr("\0337\033[r\0338");
	} else {
		Placur(bottom - num + 1, 0);
		for (i = 0; i < num; i++)
			putpad(DL, LI - CapLine);
		Placur(top, 0);
		for (i = 0; i < num; i++)
			putpad(AL, LI - CapLine);
	}
}

/* Delete `num' lines starting at `top' leaving the lines below `bottom'
   alone.  This updates the internal image as well as the physical image.  */

v_del_line(num, top, bottom)
{
	register int	i,
			bot;
	struct screenline	savelines[MAXNLINES];

	bot = bottom;

	/* Save the lost lines. */

	for (i = 0; i < num && top + i <= bottom; i++)
		savelines[i] = Screen[top + i];

	/* Copy everything up num number of lines. */

	for (i = top; num + i <= bottom; i++) {
		Screen[i] = Screen[i + num];
		Screen[i] = Screen[i + num];
	}

	/* Restore the lost ones, clearing them. */

	for (i = 0; i < num; i++) {
		Screen[bottom - i] = savelines[i];
		clrline(Screen[bot].s_line, Screen[bot].s_length);
		bot--;
	}

	if (BG) {	/* It makes such a big difference when built in!!! */
		putstr(sprint("\033[%d;%dr\033[%dM\033[r",
				top + 1, bottom + 1, num));
		CapCol = CapLine = 0;
	} else if (VT) {
		Placur(bottom, 0);
		putstr(sprint("\0337\033[%d;%dr\0338", top + 1, bottom + 1));
		for (i = 0; i < num; i++)
			outchar('\n');
		putstr("\0337\033[r\0338");
	} else {
		Placur(top, 0);
		for (i = 0; i < num; i++)
			putpad(DL, LI - CapLine);
		Placur(bottom + 1 - num, 0);
		for (i = 0; i < num; i++)
			putpad(AL, LI - CapLine);
	}
}
