#include "hd.h"
#include "mydir.h"
#include "command.h"

#define	ISDIR	((scr_stb.st_mode & S_IFMT) == S_IFDIR)
#define	paction(arg)	{ \
	atfile (file, 4);printf (action [cpage][file] ? "//" : "  ");}

/* User first indicates which files to remove by selecting them
   with a-t keypresses.  Selected files are stored in the action
   array.  When ready, the user presses R, and the files are gone.
*/

static char quitcmds[] = {CR, LF, EOT, 0};
static char dircmds [] = {"0123456789;+-uL"};

static char *rmmsg[] = {
	"Select the files you wish to remove, then press -R-",
	"Press -Return- to leave with out removing files. -?- for more help"
};

remove () {			/* remove (unlink) files */

register cmd, file;  register char *cp;

short starmode;
#define	OFF	0
#define	ON	1
#define	UNDEF	2

extern char *index ();

char action [mpages][nfpp];

if (access (DOT, 3)) {
	myperror ("Cannot remove files");
	return FAILURE;
}
for (cp = &action [0][0]; cp < &action [mpages-1][nfpp];) *cp++ = OFF;
dispaction (action);
disprmmsg ();

for (;;) {
	at (2380);
	cmd = getch ();
	file = cmd - 'a';
	if (index (quitcmds, cmd) != CNULL) {
		clearmsg (0);  return NOREPLOT;
	}
	else if (index (dircmds, cmd) != CNULL) {
		if (dircmd (cmd) & REPLOT) rmdispdir (action);
	}
	else if (cmd == '*') {
		starmode = UNDEF;
		for (file = 0; file < pageend; file++) {
			cp = &action [cpage][file];

			if (compe (filename (file), DOT));
			else if (compe (filename (file), DOTDOT));

			else if (starmode == UNDEF)
				*cp = starmode = !*cp;
			else *cp = starmode;
		}
		dispaction (action);
	}

	else if (cmd == 'R') break;
	else if (cmd == '?') {
		if (help (RMHELP) & REPLOT) rmdispdir (action);
	}
	else if (cmd >= 'a' && cmd <= 'z' && file < pageend) {
		if (compe (filename (file), DOT));
		else if (compe (filename (file), DOTDOT));
		else {
			action [cpage][file] ^= 1;
			paction (file);
		}
	}
}
act (action);
return REPLOT;
}

dispaction (action) char action [mpages][nfpp]; {
	register file;

	bufout ();
	for (file = 0; file < pageend; file++)
		paction (file);
	unbufout ();
}

rmdispdir (action) char action [mpages][nfpp]; {
	dispdir (0);
	dispaction (action);
	disprmmsg ();
}

disprmmsg () {
	clearmsg (2);
	bufout ();
	printf ("%s\n%s", rmmsg [0], rmmsg [1]);
	unbufout ();
}

/* This does the actual removing */
act (action) char action [mpages][nfpp]; {
	register page, file;

	clearmsg (1);
	for (page = 1; page <= tpages; page ++)
		for (file = 0; file < nfpp; file++)
		if (action [page][file]) {

		if (page != cpage) {
			cpage = page;  dispdir (0);
			dispaction (action);  clearmsg (1);
		}

		atfile (file, 0);  printf ("->");
		at (2301);
		if (stat (filename (file), &scr_stb)) {
			myperror (filename (file));
			if (rmerror ()) return;
		}
		else if (ISDIR && f_exec
			("/bin/rmdir", "+", filename (file), 0)) {
			if (rmerror ()) return;
		}
		else if (!ISDIR && unlink (filename (file))) {
			myperror (filename (file));
			if (rmerror ()) return;
		}
		else {
			atfile (file, 0);  clearline ();
		}
	};
	return;
}
rmerror () {		/* Error in removing file */
	register ch;

	at (2365);  printf ("Press -Return-");
	ch = getch ();
	clearmsg (0);
	return (ch == EOT);
}
