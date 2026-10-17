/*
 * Unix 6
 * Editor
 *
 */

/* Mods: George Coulouris (QMC)
 *	March, 76
 *
 *	prompts (suppress with `-p' flag)
 *	", %, &, to display a screen full of context
 *	`x' - as `s' but interactive
 *	`n' flag when appended to `s' or `x' commands
 *		prints number of replacements
 *	% == current file name in ! commands
 *	!! == repeat the last ! command you executed
 *	-e flag == "elfic" mode :-
 *		no "w", "r\n" commands, auto w before q
 *	`o' command for text input with local editing via control keys
 *	`b' to set a threshold for automatic line breaks in `o' mode.
 *	`h' displays a screen full of help with editor commands
 *		(the help is in /usr/lib/emhelp)
 */

/* Mods: Ian Johnstone (AGSM)
 *	September, 77
 *
 *	When receive terminate (14) signal, output current file
 *	to "saved.file" and tell user about this - then exit.
 */

/* Mods: Greg Rose
 *	77/09/12
 */

#define	AUTOW

/*	A new command, ti to set the time between automatic writes of the file.
 *	Syntax: tin (where n is an integer) will write the file
 *	every n minutes to the current file.
 *	A pair of bells are output to warn the user,
 *	and rubouts are trapped.
 *
 *	Also the pattern matching routines have been improved.
 *	If a new pattern match includes `&', the `&' is
 *	replaced with the old pattern.
 */

/* Mods: Craig McGregor (CSU)
 *	77/12/23
 *
 *	Make numerous fixes and modifications.
 *	1. Allow the % in a shell command line to be
 *	escaped with `\'.
 *	2. Perform % evaluation in a shell command line
 *	when the line is actually executed rather than
 *	the first time it is typed in.  This will allow
 *	it to reflect the current file name all of the time.
 *	3. The `o' command has some slight corrections.
 *	When the command is aborted with a rubout
 *	character it no longer loses the next line.
 *	It no longer fouls up when a null character is
 *	received; it is now ignored.  The error message
 *	has been changed from some obscure character used
 *	on ITT terminals to simply a bell.
 *	4. Always redirect the confirmation input for the
 *	`x' command so that will work even as part of the
 *	`g' command.
 *	5. Allow new lines to be substituted into lines
 *	even when the `g' command is in use.
 *	6. The use of `s', `x' and `o' commands does not
 *	remove labels from lines.  When a line is changed
 *	to more than one, the label is left attached to
 *	the first.  This is also relevent to the `c' command
 *	however only the first line of the change preserves
 *	its tag.
 *	Also it is now possible to place tags on line #1
 *	and use them there.
 *	7. The `o' command modified to use the UCASE bit
 *	before converting "\b" to upper-case B.
 *	8. Make `xl' command correctly handle tabs etc.
 *	9. A switch (-l) to select a default mode of `l'
 *	instead of `p' when a line number or newline is
 *	given without a command.
 *	11. Two new subcommands have been added to the
 *	`o' command's repertoire: ^P to skip to the end
 *	of the line; ^T to backup a single character.
 *	12. Handle long-line errors correctly.
 *	13. Allow the '.' to finish off an append, insert
 *	or change input to be escaped.  This will allow
 *	users to create lines consisting of a single dot.
 */

/* Mods: Kevin Hill
 *	April - May, 78
 */

#define JOIN

/*	1.  Added a new command for joining lines together :
 *
 *		(.,.+1)j
 *
 *	The given lines are joined, with the new line characters
 *	replaced by blanks.  If the total line exceeds the allowed
 *	linelength, the command is aborted without changing the file.
 *	May also type 'jc', for join-and-compress; that is, join the two
 *	(or more) lines together, removing all blanks and tabs at the join
 *	except for one blank.  Any label on the first line is maintained.
 *	Note for future modifiers :  ALL reading operations MUST
 *	be completed before a write is attempted.
 *
 *	2.  Removed modification 13 above, which caused more trouble
 *	than it was worth.  Suggested method of entering '.' :  enter
 *	'..' and later on do 's/.//p'.
 *
 *	3.  Stylistic improvements were made to make the program readable!
 *
 *	4.  '"l', '%l', and '&l' now working.
 */

#define SEARCH_STRING

/*	5.  Incredibly small change made so that the search string is
 *	not forgotten if accidently enter '/' instead of '//', or '?'
 *	instead of '??'.  Another change was made so that filename
 *	is not forgotten if enter 'f ' (or something similar).
 */

#define GLOBAL

/*	6.  Print out '??' if no match in a global command pattern
 *	in the entire file.  Note :  do NOT seek!
 */

#define ERRORS

/*	7.  Improved error recovery for command files.  The line number
 *	at which the error occurred is printed.  Assisted by Ian J, who wrote
 *	the verbose error messages as well as catching 'init' errors, and
 *	initially requesting core for INITLINES lines to speed up init.
 *	When invoked as 'e -s file', the verbose error messages are suppressed
 *	in favour of '?' and '??'.  When invoked as 'e - file', match-fail and
 *	empty-file error messages are suppressed (useful for command files).
 */

#define	N_LINES

/*	8.   Added feature to `"', `%', and `&' commands :
 *
 *			`"n'	or	`%n'	or	`&n'
 *
 *	where `n' specifies the screen size, which is remembered until reset.
 *	May be followed by `l' or `p', as before.  The default values is LINES.
 *	The minimum value allowed is 4.
 */

#define	NULLS

/*	9.  Tell the user how many nulls have been thrown away when the file
 *	is read in.  Also, add a '\n' onto the last line, if none present,
 *	and inform the user (previously, such a line was discarded).
 */

#define	ADDRESS

/*	10.  Address-reading routine improved to reject rubbish, such as
 *	1,2,3,4,5,6,7p etc. etc. (result same as before for valid addresses).
 *	Added defaults: if no address before `,' - assume `1'; if no address
 *	after `,' - assume `$'.  Thus, for example, `,p' is identical to
 *	`1,$p', and prints the whole file.  Also added `/^/' default
 *	to the global command - thus, for example, `gp' prints all
 *	lines in the file.  The drawback is that the search-string
 *	delimiters may not be alphabetics (as they are then interpreted
 *	as commands).
 */

#define	CHDIR

/*	11.  Added 'cd' to the command list.  Form :  'cd pathname', where
 *	the pathname cannot contain the short-cuts '%' or '\@'.  This is
 *	because /etc/glob could not be used, for obvious (?) reasons.
 */

#define	PRINT_STRING

/*	12.  Added new command 'z' to print out the current search-string.
 *	Added '-' facility to the search string: [a-d] = [abcd].
 *	The '-' may be escaped: [abc\-f] = [abcf] + [-].
 */

#define	NICE_EXIT

/*	13.  Fixed up 'o' mode so that, if have typed '\n', and then hit
 *	DEL to exit unchanged, will not lose rest of current line.
 */

#define	BETTER_SWAP

/*	14.  Improved the 's' and 'x' algorithm - it was incredibly inefficient
 *	for 'x' and 's/../../g' commands before this change, copying the
 *	linebuffer backwards and forwards numerous times.
 */

#define	UNIXCMD

/*	15.  Altered unix command line so that `!' is replaced by the last
 *	unix command.  This means `!!' works as before, but you may now
 *	type such things as `!!&' or `!! > junk', etc.
 *	(Improved the code while I was at it - what a mess!)
 */

#define	MISC

/*	16.  Numerous fixes/mods.  Fixed `elfic' mode so can exit with
 *	null file name and null file, can enter `f' with no arguments, and
 *	can enter `f' with an argument if there is no file name.  Change `w'
 *	to flush core buffers into temporary file (less lost if system
 *	crashes).  Corrected sloshing to make it consistent: absorb unnecessary
 *	sloshes, and treat `\\x' as distinct from `\x'.  Made open-mode
 *	smarter about its kill and erase characters.  Improved `getchar' so
 *	ignores `control-d' from a terminal (doesn't kill append, etc.)
 *	(left this change ifdef'ed under AUTOW).  Changed `count' to a long.
 *	Corrected test for tty (use `gtty' instead of `fstat').  Added `wx'
 *	option to `w' command - write with execute permission (mode 0700).
 *	Some corrections to AUTOW: catch create-error; object if there is
 *	no file name.  Fix `compsub' so `g/../s/../p' is caught
 *	(used to hang waiting for input).  Check first word of file to
 *	object to packed files, objects, etc.
 */

/* Mods: David Milway
 *	May, 78
 */

#define	DRM

/*	Added a new option to 'w' command :  'wa' to write onto the end
 *	of the named file.  If the file is not present, it is created.
 */

#define	NOSPELL

/*	Temporary measure until `spell' works correctly!
 *
 *				Kevin Hill
 */

#define	SIGHUP	1
#define	SIGINTR	2
#define	SIGQUIT	3
#define	SIGTERM	14
#ifdef	AUTOW
#define	SIGCLOK	15
#endif	AUTOW
#define	FNSIZE	64
#define	LBSIZE	512
#ifdef	ERRORS
#define	INITLINES	200	/* initial number of lines */
#endif	ERRORS
#define	ESIZE	128
#define	GBSIZE	256
#define	NBRA	8
#define	EOF	-1
#define	UNIXBUFL 100		/* must be < LBSIZE */

#define	CBRA	1
#define	CCHR	2
#define	CDOT	4
#define	CCL	6
#define	NCCL	8
#define	CDOL	10
#define	CEOF	11
#define	CKET	12

#define	STAR	01

#define	READ	0
#define	WRITE	1

/* screen dimensions */
#define	LINES	24
#define	LENGTH	80
#define	PROMPT	'>'
#define	CONFIRM	'.'
#define	TABSET	7	/* this should be determined dynamically */

#define	RAW	040
#define	ECHO	010
#define	UCASE	04

#define	OPEN	'/'
#define	BELL	'\007'
#define	ESCAPE	'\033'
#define	SPACE	' '
#define	BACKSL	'\\'
#define	RUBOUT	'\177'
#define	CTRLA	'\01'
#define	CTRLB	'\02'
#define	CTRLC	'\03'
#define	CTRLD	'\04'
#define	CTRLE	'\05'
#define	CTRLF	'\06'
#define	CTRLG	'\07'		/* == bell */
#define	CTRLH	'\010'		/* == backspace */
#define	CTRLI	'\011'		/* == TAB */
#define	CTRLL	'\014'		/* == form-feed */
#define	CTRLP	'\020'
#define	CTRLQ	'\021'
#define	CTRLR	'\022'
#define	CTRLS	'\023'
#define	CTRLT	'\024'
#define	CTRLV	'\026'
#define	CTRLW	'\027'
#define	CTRLX	'\030'
#define	CTRLZ	'\032'


struct { int integer; };

int	peekc;
int	lastc;
char	savedfile[FNSIZE];
char	file[FNSIZE];
char	linebuf[LBSIZE];
char	rhsbuf[LBSIZE/2];
char	expbuf[ESIZE+4];
int	circfl;
int	*zero;
int	*dot;
int	*dol;
int	*endcore;
int	*fendcore;
int	*addr1;
int	*addr2;
char	genbuf[LBSIZE];
long	count;
char	*nextip;
char	*linebp;
int	ninbuf;
int	nline;
int	io;
int	pflag;
int	onhup;
int	onquit;
int	vflag;
int	listf;
int	col;
char	*globp;
int	tfile;
int	tline;
char	*tfname;
char	*loc1;
char	*loc2;
char	*locs;
char	ibuff[512];
int	iblock;
char	obuff[512];
int	oblock;
int	ichanged;
int	nleft;
int	names[26];
char	*braslist[NBRA];
char	*braelist[NBRA];

int	margin;			/* used to set threshold in 'open' */

int	elfic;			/* true if "elfic" (-e) flag */
int	firstime;		/* ugh - used to frigg initial "read" for elfic mode */
char	unixbuffer[UNIXBUFL];
int	xflag;			/* used in 'xchange' command */

int	oflag;
char	*threshold, *savethresh;
char	*lnp, *gnp, *brp;
int	savetty, tty[3];

char	lflag;

#ifdef	AUTOW
int	writetime;
int	justwrit;	/* for the auto-write */
int	delaywrite;
int	writewaiting;	/* kludge for 'o' mode, since it flogs a buffer */
#endif	AUTOW

#ifdef	JOIN
int	nocmpress;	/* for join-and-compress option */
#endif	JOIN

#ifdef	ERRORS
int	istty;		/* set when input commands are from a tty */
int	linenr;		/* line number of each command as taken from a file */
int	currlnnr;	/* for the current command line number */
int	sflag;		/* for '-s' option */
#endif	ERRORS

#ifdef	N_LINES
int	lines;			/* number of lines on screen for ", %, and & */
#endif	N_LINES

#ifdef	NICE_EXIT
int	oldline, *thestart;
#endif	NICE_EXIT

#ifdef	BETTER_SWAP
char	*ssp, *slp;	/* pointers to genbuf and linebuf */
#endif	BETTER_SWAP

#ifdef	MISC
int	nbra;		/* number of subexpressions */
int	chkflg;		/* flag for reading funny files */
#endif	MISC
