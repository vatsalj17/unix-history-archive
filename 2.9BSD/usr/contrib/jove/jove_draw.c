/*
   Jonathan Payne at Lincoln-Sudbury Regional High School 4-19-83
  
   jove_draw.c

   This contains, among other things, the modeline formatting, and the
   message routines (for prompting).  */

#include "jove.h"

#include "termcap.h"

char mesgbuf[100];

char *
bufmod(bp)
BUFFER	*bp;
{
	if (IsModified(bp))
		return "* ";
	return "";
}

/*
 * Find_pos returns the position on the line, that c_char represents
 * in line.
 */

find_pos(line, c_char)
LINE	*line;
{
	register char	*lp;
	char	buf[LBSIZE];

	if (line == curline)
		lp = linebuf;
	else
		lp = getcptr(line, buf);
	return calc_pos(lp, c_char);
}

calc_pos(lp, c_char)
register char	*lp;
register int	c_char;
{
	register int	pos = 0;
	register char	c;

	while ((--c_char >= 0) && (c = *lp++)) {
		if (c == '\t')
			pos += (tabstop - (pos % tabstop));
		else if (c == '\177' || c < ' ')
			pos += 2;
		else
			pos++;
 	}
	return pos;
}

/*
 * message prints the null terminated string onto the bottom line of the
 * terminal.
 */

message(str)
char	*str;
{
	if (Input)
		return;
	UpdMesg++;
	errormsg = 0;
	if (str != mesgbuf)
		strncpy(mesgbuf, str, sizeof mesgbuf);
}

/*
 * This prints all the information about the current mode, and the
 * current filename.
 */

#define strcop(lp,str)		for (--lp, pp = str; (*lp++ = *pp++); )

ModeLine(w)
WINDOW	*w;
{
	int	lineno = FLine(w) + SIZE(w),
		i;
	char	line[132];
	int	*flags = w->w_bufp->b_flags;

	register char	*lp = line + 1,		/* Pay attention to this! */
			*pp;
	static char	*modenames[] = {
		"TE-",
		"OV-",
		"C-",
		"MA-",
		"CIS-",
		"SM-",
		"AI-"
	};

	i_set(lineno, 0);

	if (w->w_next != fwind)
		strcop(lp, "--- ");
	else
		strcop(lp, "    ");

	strcop(lp, "JOVE (");

	for (i = 0; i < NFLAGS; i++, flags++) {
		if (*flags == 0)
			continue;
		strcop(lp, modenames[i]);
	}
	if (*(lp - 2) == '(')
		strcop(lp, "NORMAL)   ");
	else {
		lp--;	/* Back over the '-' */
		strcop(lp, ")   ");
	}
	strcop(lp, sprint("Buffer: %s ", w->w_bufp->b_name));
	pp = w->w_bufp->b_fname ? sprint("  \"%s\" ", w->w_bufp->b_fname) : "  [No file] ";
	strcop(lp, pp);
	if (IsModified(w->w_bufp))
		strcop(lp, "* ");
	--lp;	/* strcop leaves lp after the null */
	if (w->w_next != fwind)
		while (lp < &line[CO - 2])
			*lp++ = '-';
	*lp = 0;
	if (swrite(line))
		do_cl_eol(lineno);
}

RedrawDisplay()
{
	LINE	*newtop = prev_line(curwind->w_line, exp_p ?
				exp : HALF(curwind));

	if (newtop == curwind->w_top)
		v_clear(FLine(curwind), FLine(curwind) + SIZE(curwind));
	else
		SetTop(curwind, newtop);
}

v_clear(line1, line2)
register int	line1,
		line2;
{
	while (line1 <= line2) {
		i_set(line1, 0);
		cl_eol();
		oimage[line1].Line = nimage[line1].Line = 0;
		line1++;
	}
}

ClAndRedraw()
{
	cl_scr();
}

NextPage()
{
	LINE	*newline;

	if (exp_p)
		UpScroll();
	else {
		newline = next_line(curwind->w_top, max(1, SIZE(curwind) - 1));
		DotTo(newline, 0);
		if (in_window(curwind, curwind->w_bufp->b_dol) == -1)
			SetTop(curwind, newline);
		curwind->w_line = newline;
	}
}

PrevPage()
{
	LINE	*newline;

	if (exp_p)
		DownScroll();
	else {
		newline = prev_line(curwind->w_top, max(1, SIZE(curwind) - 1));
		DotTo(newline, 0);
		SetTop(curwind, curline);
		curwind->w_line = curwind->w_top;
	}
}

int	VisBell = 0,
	RingBell = 0;	/* So if we have a lot of errors ...
			   ring the bell only ONCE */

rbell()
{
	RingBell++;
}
