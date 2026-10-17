/*
 * kbdselect.c - 386 only!
 *
 * Get keyboard and console preferences and write results to stdout.
 *
 * cc kbdselect.c -lcurses [-lterm]
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <curses.h>
#include <sys/kb.h>

#include "build0.h"

extern	char	*fgets();
extern	char	*malloc();

#define	KBDLIST	"/conf/kbd/kblist"	/* List of keyboards.	*/

#define MONO_SESSION_VAR "MONO_COUNT"	/* Number of monochrome sessions. */
#define COLOR_SESSION_VAR "VGA_COUNT"	/* Number of color sessions. */
#define	KBDY	3			/* y-coordinate of keyboard list. */
#define	NLINE	512			/* Line length.		*/
#define	NKBDS	(24-KBDY)		/* Max number of keyboards */
#define	USAGE	"Usage: /etc/kbdselect outputfile"
#define MAX_NUM_SESSIONS	8

/* Language specifiers for nonloadable kb's. */
#define DE_LANG_STR	"DE"
#define FR_LANG_STR	"FR"
#define US_LANG_STR	"-"

/* Keyboard list file entries. */
typedef	struct	kline	{
	int	k_loadable;		/* True: loadable tables supported */
	int	k_lang;			/* Used only for nonloadable kb. */
	char	*k_driver;		/* Name of keyboard driver object */
	char	*k_table;		/* Keyboard table name	*/
	char	*k_desc;		/* Keyboard description	*/
}	KLINE;

/* Forward */
char	*copystr();
void	display_line();
void	display_rev();
int	blank_line();
void	get_sessions();
void	my_fatal();			/* handles window cleanup */
void	format_error();
void	my_nonfatal();			/* handles window cleanup */
void	read_klist();

/* Globals. */
int	bflag;				/* Invoked from build.	*/
char	buf[NBUF];			/* Input buffer.	*/
KLINE	klist[NKBDS];			/* Keyboard list.	*/
int	nkbds;				/* Number of keyboards.	*/
int	initflag;			/* Curses initialized.	*/
int	uflag;				/* Invoked from update.	*/
int	virtual;			/* Virtual consoles wanted */
int	default_entry = -1;		/* Which entry to default to */

FILE * outfp;
FILE *dfp;

main(argc, argv)
int argc;
char * argv[];
{
	register int n, c;

	dfp = fopen("/dev/color0", "w");
	
	/* Get name of output file.  No default. */
	if (argc != 2)
		my_fatal(USAGE);
		
	if ((outfp = fopen(argv[1], "w")) == NULL)
		my_fatal("Can't open output file %s", argv[1]);
		
	/* Initialize screen and terminal modes. */
	initscr();
	initflag = 1;
	noecho();
	raw();
	clear();

	/*
	 * Ask if virtual console support is wanted
	 * and set variable "virtual" as needed.
	 */
	mvaddstr(0, 0,
"This release of COHERENT supports multiple simultaneous sessions on the");
	mvaddstr(1, 0,
"system console.  This feature, known as virtual consoles, supports both");
	mvaddstr(2, 0,
"monochrome and color video cards (text mode only) with multiple \"login\"");
	mvaddstr(3, 0,
"sessions on each, depending upon your hardware configuration.  If you have");
	mvaddstr(4, 0,
"both color and monochrome adapters on your system, you can run multiple");
	mvaddstr(5, 0,
"sessions on each of them at the same time.");

	mvaddstr(7, 0,
"You may choose a loadable keyboard table or a non-loadable keyboard table.");
	mvaddstr(8, 0,
"Loadable keyboard tables allow you to remap keys and load new tables while");
	mvaddstr(9, 0,
"your system is active. This requires keyboards that are 100% IBM AT compliant.");
	mvaddstr(11, 0,
"If you chose a non-loadable keyboard, you will not have this flexibility.");
	mvaddstr(13,0,
"Not all keyboards are 100% IBM AT compliant and will not work with loadable");
	mvaddstr(14,0,
"keyboard tables. If your keyboard is not 100% IBM AT compliant, it may result");
	mvaddstr(15,0,
"in COHERENT not booting properly after the installation process is completed,");
	mvaddstr(16,0,
"or you may find that your keyboard keys are not mapped as they are labeled and");
	mvaddstr(17,0,
"that some keys cease to function at all. For this reason, the default selection");
	mvaddstr(18,0,
"is the non-loadable keyboard.");
	mvaddstr(20, 0,
"Greek keyboard support is available only if virtual consoles are selected.");
	mvaddstr(22, 0,
"Do you wish to include support for virtual consoles? [y or n] ");
	refresh();
	echo();
	for (;;) {
		c = getch();
		switch (c) {
		case 'Y':
		case 'y':
			++virtual;
			break;
		case 'N':
		case 'n':
			break;
		default:
			continue;
		}
		noecho();
		refresh();
		mvaddstr(23, 0, "Please hit <Enter> ...");
		refresh();
		while ((c = getch()) != '\r' && c != '\n')
			;
		mvaddstr(23, 0, "One moment, please ....");
		refresh();
		break;
	}

	clear();

	/* Read the keyboard list file. */
	read_klist();

	/* Display instructions at top of screen. */
	mvaddstr(0, 0,
"Select the entry below which indentifies your keyboard type.  Hit <Enter> to");
	mvaddstr(1, 0,
"select the highlighted entry.  Hit <space> to move to the next entry.");

	/* Display choices. */
	for (n = 0; n < nkbds; n++)
		display_line(n);

	/* Interactive input loop: display choice n highlighted. */
	for (n = (default_entry >= 0) ? default_entry : 0; ; ) {
		display_rev(n);		/* display default choice in reverse */
		refresh();
		switch (c = getch()) {
		case 'A':		/* n.b.:  ESC [ A is Up Arrow key */
			if (--n < 0)
				n = nkbds - 1;
			continue;
		case ' ':		/* space/Down Arrow: try next choice */
		case 'B':		/* n.b.:  ESC [ B is Down Arrow key */
			if (++n >= nkbds)
				n = 0;
			continue;
		case '\n':
		case '\r':
			/*
			 * enter: take default value.
			 * If Greek, force virtual consoles.
			 */
			if (strncmp(klist[n].k_table, "greek", 5) == 0)
				virtual = 1;
			break;
		default:
			continue;	/* ignore */
		}
		break;			/* done, choice is in n */
	}

	/* Output driver name. */
	fprintf(outfp, "DRIVER=%s%s\n", virtual ? "vt" : "", klist[n].k_driver);


	if (klist[n].k_loadable) {

		/* Output keyboard table name. */
		fprintf(outfp, "TABLE=%s\n", klist[n].k_table);

	} else {

		char * kb_lang_val;

		/*
		 * Non-loadable keyboard table selected.
		 */
		switch(klist[n].k_lang) {
		case kb_lang_us:
			kb_lang_val = "kb_lang_us";
			break;
		case kb_lang_de:
			kb_lang_val = "kb_lang_de";
			break;
		case kb_lang_fr:
			kb_lang_val = "kb_lang_fr";
			break;
		}

		/* Output keyboard language. */
		fprintf(outfp, "KB_LANG=%s\n", kb_lang_val);
	}


	/* Back to non-curses output. */
	clear();

	refresh();

	endwin();


	/* For virtual consoles, ask how many sessions. */

	if (virtual) {
		get_sessions();
	}

	fclose(outfp);
	
	/* Done. */
	exit(0);
}

/*
 * Allocate a copy of a string and return a pointer to it.
 */
char *
copystr(s) register char *s;
{
	register char *cp;

	if ((cp = malloc(strlen(s) + 1)) == NULL)
		my_fatal("no space for string \"%s\"");
	strcpy(cp, s);
	return cp;
}

/*
 * Display choice n.
 */
void
display_line(n) register int n;
{
	mvaddstr(KBDY+n, 12, klist[n].k_desc);
}

/*
 * Display choice n in reverse video.
 */
void
display_rev(n) int n;
{
	static int last = -1;

	if (last != -1)
		display_line(last);	/* redisplay last in normal */
	standout();
	display_line(n);		/* display n in reverse */
	standend();
	last = n;
}

/*
 * Cry and die.
 */
/* VARARGS */
void
my_fatal(s) char *s;
{
	if (initflag)
		endwin();
	fprintf(stderr, "kbdselect: %r\n", &s);
	exit(1);
}

/*
 * Issue nonfatal informative message.
 */
/* VARARGS */
void
my_nonfatal(s) char *s;
{
	if (initflag)
		endwin();
	fprintf(stderr, "kbdselect: %r\n", &s);
}

/*
 * Read a file containing keyboard file names and descriptions,
 * build a keyboard list.
 *
 * Format of input is:
 *	# Comment line
 *      blank line
 * or
 *	Default <tab> Loadable <tab> Driver <tab> Table <tab> Descr.
 */
void
read_klist()
{
	register FILE *fp;
	register unsigned char *s, *s1;

	if ((fp = fopen(KBDLIST, "r")) == NULL)
		my_fatal("cannot open \"%s\"", KBDLIST);
	while (fgets(buf, sizeof buf, fp) != NULL) {
		if (buf[0] == '#' || blank_line(buf))
			continue;		/* ignore comments */
		if (nkbds == NKBDS) {
			my_nonfatal("more than %d keyboard entries", NKBDS-1);
			continue;
		}

		/*
		 * Parse input lines and emit error messages if input bad.
		 */
		if (buf[0] == 'Y' || buf[0] == 'y')
			default_entry = nkbds;
		if ((s = strchr(buf, '\t')) == NULL) {
			format_error(buf);
			continue;
		}

		switch (*++s) {
		case 'Y': case 'y':
			klist[nkbds].k_loadable = TRUE;
			break;
		case 'N': case 'n':
			klist[nkbds].k_loadable = FALSE;
			break;			/* ... or if VC's desired */
		default:
			format_error(buf);
			continue;
		}
		if ((s = strchr(++s, '\t')) == NULL) {
			format_error(buf);
			continue;
		}
		if ((s1 = strchr(++s, '\t')) == NULL) {
			format_error(buf);
			continue;
		}
		*s1++ = '\0';
		klist[nkbds].k_driver = copystr(s);	/* keyboard driver */
		if ((s = strchr(s1, '\t')) == NULL) {
			format_error(s1);
			continue;
		}
		*s++ = '\0';
		klist[nkbds].k_table = copystr(s1);	/* keyboard table */

		/* k_table must contain DE/FR/- for non-loadable driver. */
		if (!klist[nkbds].k_loadable) {
			if (strcmp(klist[nkbds].k_table, DE_LANG_STR) == 0)
				klist[nkbds].k_lang = kb_lang_de;
			else if (strcmp(klist[nkbds].k_table, FR_LANG_STR) == 0)
				klist[nkbds].k_lang = kb_lang_fr;
			else if (strcmp(klist[nkbds].k_table, US_LANG_STR) == 0)
				klist[nkbds].k_lang = kb_lang_us;
			else {
				format_error(s1);
				continue;
			}
		}

		if ((s1 = strchr(s, '\n')) == NULL) {
			my_nonfatal("no newline in \"%s\"", s);
			continue;
		}

		*s1 = '\0';
		klist[nkbds].k_desc = copystr(s);	/* kbd description */

		nkbds++;
	}
	fclose(fp);
}

/*
 * Input error found: display an error message and continue.
 */
void
format_error(line)
char *line;
{
	my_nonfatal("Input format error found in \"%s\" -- ignored.", line);
}

/*
 * Return TRUE if line consists only of whitespace.
 */
int
blank_line(str)
char * str;
{
	int is_blank = TRUE;

	while(*str) {
		if (isspace(*str))
			str++;
		else {
			is_blank = FALSE;
			break;
		}
	}

	return is_blank;
}

/*
 * For virtual console users only, ask how may mono/color sessions
 * and write the result to stdout.
 */
void
get_sessions()
{
	int	color_sessions;		/* number of color sessions */
	int	mono_sessions;		/* number of mono sessions */
	int	mono_display;		/* 1 if mono display is present */
	int	color_display;		/* 1 if color display is present */
	int	high_prompt;
	int	choice;

	for (;;) {
		mono_sessions = 0;
		color_sessions = 0;
		printf(
"Please indicate the type of video adapter you are using.\n"
"Many portable computers now use VGA video adapters, even though the\n"
"display is monochrome.  For these systems, select (1) below.\n\n"
"\t1.  Color or VGA monochrome.\n"
"\t2.  Monochrome, (non-VGA).\n"
"\t3.  Both (two video adapters, two displays).\n");

		choice = get_int(1, 3, "\nYour choice: ");

		switch (choice) {
		case 1:
			mono_display = 0;
			color_display = 1;
			break;
		case 2:
			mono_display = 1;
			color_display = 0;
			break;
		case 3:
			mono_display = 1;
			color_display = 1;
			break;
		}

		printf("\nYou may select a total of up to %d virtual "
		  "console sessions.\n", MAX_NUM_SESSIONS);

		high_prompt = MAX_NUM_SESSIONS;

		if (color_display) {
			sprintf(buf, "\nHow many active virtual console sessions "
			  "would you like on your\ncolor video display "
			  "[1-%d]?", high_prompt);
			color_sessions = get_int(1, high_prompt, buf);
			high_prompt = MAX_NUM_SESSIONS - color_sessions;
		}

		if (mono_display) {
			sprintf(buf, "\nHow many active virtual console sessions "
			  "would you like on your\nmonochrome video display "
			  "[1-%d]?", high_prompt);
			mono_sessions = get_int(1, high_prompt, buf);
		}

		if (mono_sessions + color_sessions > 0)
			break;

		/* else - try again - no sessions enabled! */
		printf("\nYou must allow at least one session!\n");
	}

	/* Output selections to stdout. */
	fprintf(outfp, "COLOR_COUNT=%d\n", color_sessions);
	fprintf(outfp, "MONO_COUNT=%d\n", mono_sessions);
}
