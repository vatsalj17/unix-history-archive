/*
   Jonathan Payne at Lincoln-Sudbury Regional High School 5-25-83

   jove_curs.c

   The cursor optimization happens here.  You may decide that this
   is going too far with cursor optimization, or perhaps it should
   limit the amount of checking to when the output speed is slow.
   What ever turns you on ...   */

#include "jove.h"
#include "termcap.h"

extern int	CapCol,
		CapLine;

struct cursaddr {
	int	c_numchars,
		(*c_func)();
};

char	*Cmstr;
struct cursaddr	*HorMin,
		*VertMin,
		*DirectMin;

#define	FORWARD		0	/* Move forward */
#define FORTAB		1	/* Forward using tabs */
#define	BACKWARD	2	/* Move backward */
#define RETFORWARD	3	/* Beginning of line and then tabs */
#define NUMHOR		4

#define DOWN		0	/* Move down */
#define UPMOVE		1	/* Move up */
#define NUMVERT		2

#define DIRECT		0	/* Using CM */
#define HOME		1	/* HOME	*/
#define LOWER		2	/* Lower line */
#define NUMDIRECT	3

#define	home()		Placur(0, 0)
#define LowLine()	putpad(LL, 1), CapLine = LI - 1, CapCol = 0
#define PrintHo()	putpad(HO, 1), CapLine = CapCol = 0

struct cursaddr	WarpHor[NUMHOR],
		WarpVert[NUMVERT],
		WarpDirect[NUMDIRECT];

int	phystab = 8;

GoDirect(line, col)
register int	line,
		col;
{
	putpad(Cmstr, 1), CapLine = line, CapCol = col;
}

RetTab(col)
register int	col;
{
	outchar('\r'), CapCol = 0, ForTab(col);
}

HomeGo(line, col)
{
	PrintHo(), DownMotion(line), ForTab(col);
}

BottomUp(line, col)
register int	line,
		col;
{
	LowLine(), UpMotion(line), ForTab(col);
}

ForTab(col)
register int	col;
{
	register int	where,
			ntabs;

	if (TABS) {
		where = col - (col % phystab);	/* Round down. */
		if ((where + phystab) - col < col - where)
			where += phystab; /* Go past and come back. */
		if (where >= CO)
			where -= phystab;	/* Don't tab to last place
						 * or it is likely to screw
						 * up
						 */
		ntabs = (where / phystab) - (CapCol / phystab);
		while (--ntabs >= 0)
			outchar('\t');
		CapCol = where;
	}
	if (CapCol > col)
		BackMotion(col);
	else if (CapCol < col)
		ForMotion(col);
}

extern struct screenline	*Screen;

ForMotion(col)
register int	col;
{
	register int	length = Screen[CapLine].s_length -
					Screen[CapLine].s_line;
	register char	*cp = &Screen[CapLine].s_line[CapCol];

	while (CapCol < col) {

		if (CapCol >= length)
			*cp = ' ';
		outchar(*cp);
		cp++;
		CapCol++;
	}
}

BackMotion(col)
register int	col;
{
	while (CapCol > col) {	/* Go to the left. */
		if (BC)
			putpad(BC, 1);
		else
			outchar('\b');
		CapCol--;
	}
}

DownMotion(line)
register int	line;
{
	while (CapLine < line) {	/* Go down. */
		outchar('\n');
		CapLine++;
	}
}

UpMotion(line)
register int	line;
{
	while (CapLine > line) {	/* Go up. */
		putpad(UP, 1);
		CapLine--;
	}
}

InitCM()
{
	WarpHor[FORWARD].c_func = ForMotion;
	WarpHor[BACKWARD].c_func = BackMotion;
	WarpHor[FORTAB].c_func = ForTab;
	WarpHor[RETFORWARD].c_func = RetTab;

	WarpVert[DOWN].c_func = DownMotion;
	WarpVert[UPMOVE].c_func = UpMotion;

	WarpDirect[DIRECT].c_func = GoDirect;
	WarpDirect[HOME].c_func = HomeGo;
	WarpDirect[LOWER].c_func = BottomUp;

	HomeLen = HO ? strlen(HO) : 1000;
	LowerLen = LL ? strlen(LL) : 1000;
	UpLen = UP ? strlen(UP) : 1000;
}

extern int	InMode;

DoPlacur(line, col)
{
	int	dline,		/* Number of lines to move */
		dcol;		/* Number of columns to move */
	register int	best,
			i;
	register struct cursaddr	*cp;

#define CursMin(which,addrs,max) \
	for (best = 0, cp = &addrs[1], i = 1; i < max; i++, cp++) \
		if (cp->c_numchars < addrs[best].c_numchars) \
			best = i; \
	which = &addrs[best]; \

	if (line == CapLine && col == CapCol)
		return;		/* We are already there. */

	if (InMode)
		putstr(EI), InMode = 0;
	dline = line - CapLine;
	dcol = col - CapCol;

	/* Number of characters to move horizontally for each case.
	 * 1: Just move forward by typing the right character on the screen
	 * 2: Print the correct number of back spaces
	 * 3: Try tabbing to the correct place
	 * 4: Try going to the beginning of the line, and then tab
	 */

	if (dcol == 1 || dcol == 0) {		/* Most common case */
		HorMin = &WarpHor[FORWARD];
		HorMin->c_numchars = dcol;
	} else {
		WarpHor[FORWARD].c_numchars = dcol >= 0 ? dcol : 1000;
		WarpHor[BACKWARD].c_numchars = dcol < 0 ? -dcol : 1000;
		WarpHor[FORTAB].c_numchars = dcol >= 0 && TABS ?
				ForNum(CapCol, col) : 1000;
		WarpHor[RETFORWARD].c_numchars = (1 + (TABS ? ForNum(0, col) : col));

		/* Which is the shortest of the bunch */

		CursMin(HorMin, WarpHor, NUMHOR);
	}

	/* Moving vertically is more simple. */

	WarpVert[DOWN].c_numchars = dline >= 0 ? dline : 1000;
	WarpVert[UPMOVE].c_numchars = dline < 0 ? ((-dline) * UpLen) : 1000;

	/* Which of these is simpler */
	CursMin(VertMin, WarpVert, NUMVERT);

	/* Homing first and lowering first are considered 
	   direct motions.
	   Homing first's total is the sum of the cost of homing
	   and the sum of tabbing (if possible) to the right. */
	
	if (VertMin->c_numchars + HorMin->c_numchars <= 3) {
		DirectMin = &WarpDirect[DIRECT];	/* A dummy ... */
		DirectMin->c_numchars = 100;
	}		
	WarpDirect[DIRECT].c_numchars = CM ?
				strlen(Cmstr = tgoto(CM, col, line)) : 1000;
	WarpDirect[HOME].c_numchars = HomeLen + line +
				WarpHor[RETFORWARD].c_numchars;
	WarpDirect[LOWER].c_numchars = LowerLen + ((LI - line - 1) * UpLen) +
				WarpHor[RETFORWARD].c_numchars;

	CursMin(DirectMin, WarpDirect, NUMDIRECT);

	if (HorMin->c_numchars + VertMin->c_numchars < DirectMin->c_numchars) {
		if (line != CapLine)
			(*VertMin->c_func)(line);
		if (col != CapCol)
			(*HorMin->c_func)(col);
	} else
		(*DirectMin->c_func)(line, col);
}

#define abs(x)	((x) >= 0 ? (x) : -(x))

ForNum(from, to)
register int	from,
		to;
{
	register int	where;
	int		numchars = 0;

	if (from > to)
		return from - to;
	if (TABS) {
		where = to - (to % phystab);   /* Round down. */
		if ((where + phystab) - to < to - where)
			where += phystab;
		if (where >= CO)
			where -= phystab;	/* Don't tab to last place
						 * or it is likely to screw
						 * up
						 */
		numchars = (where / phystab) - (from / phystab);
		from = where;
	}
	return numchars + abs(from - to);
}

