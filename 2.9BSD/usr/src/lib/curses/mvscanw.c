# include	"curses.ext"

/*
 * implement the mvscanw commands.  Due to the variable number of
 * arguments, they cannot be macros.  Another sigh....
 *
 * 1/26/81 (Berkeley) @(#)mvscanw.c	1.1
 */

mvscanw(y, x, fmt, args)
reg int		y, x;
char		*fmt;
int		args; {

	return move(y, x) == OK ? _sscans(stdscr, fmt, &args) : ERR;
}

mvwscanw(win, y, x, fmt, args)
reg WINDOW	*win;
reg int		y, x;
char		*fmt;
int		args; {

	return wmove(win, y, x) == OK ? _sscans(win, fmt, &args) : ERR;
}

