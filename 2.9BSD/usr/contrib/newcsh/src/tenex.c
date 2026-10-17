static char *RCSid = 
"$Header: /usr/local/src/cmd/tcsh/tenex.c,v 1.9 83/10/05 21:56:27 kg Exp $";

/*
 * Tenex style file name recognition, .. and more.
 * History:
 *	Author: Ken Greer, Sept. 1975, CMU.
 *	Finally got around to adding to the Cshell., Ken Greer, Dec. 1981.
 *
 *	Search and recognition of command names (in addition to file names)
 *	by Mike Ellis, Fairchild A.I. Labs, Sept 1983.
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sgtty.h>
#include <dir.h>
#include <signal.h>
#include <pwd.h>
/* Don't include stdio.h!  Csh doesn't like it!! */
#ifdef TEST
#include <stdio.h>
#include "dir.h"
#define flush()		fflush(stdout)
#endif

#define TRUE		1
#define FALSE		0
#define ON		1
#define OFF		0
#define FILSIZ		512		/* Max reasonable file name length */
#define ESC		'\033'
#define equal(a, b)	(strcmp(a, b) == 0)
#define is_set(var)	adrof(var)
#define BUILTINS	"/usr/new/lib/builtins/" /* fake builtin bin */

extern short SHIN, SHOUT;
extern char *getenv ();
extern putchar ();

typedef enum {LIST, RECOGNIZE} COMMAND;

static char
    *BELL = "\07";

static
setup_tty (on)
{
    static struct tchars  tchars;	/* INT, QUIT, XON, XOFF, EOF, BRK */
    static char save_t_brkc = -1;	/* Save user's break character */

    sigignore (SIGINT);
    if (on)
    {
	struct sgttyb sgtty;

	ioctl (SHIN, TIOCGETC, &tchars);	/* Get current break character*/
	save_t_brkc = tchars.t_brkc;		/* Current break char, if any */
	if (save_t_brkc != ESC)			/* If it's not already ESCAPE */
	{
	    tchars.t_brkc = ESC;		/* Set break char to ESCAPE */
	    ioctl (SHIN, TIOCSETC, &tchars);
	}

	/*
	 * This is a useful feature in it's own right...
	 * The shell makes sure that the tty is not in some weird state
	 * and fixes it if it is.  But it should be noted that the
	 * tenex routine will not work correctly in CBREAK or RAW mode
	 * so this code below is, therefore, mandatory.
	 */
	ioctl (SHIN, TIOCGETP, &sgtty);
	if ((sgtty.sg_flags & (RAW | CBREAK)) ||
	   ((sgtty.sg_flags & ECHO) == 0))	/* not manditory, but nice */
	{
	    sgtty.sg_flags &= ~(RAW | CBREAK);
	    sgtty.sg_flags |= ECHO;
	    ioctl (SHIN, TIOCSETP, &sgtty);
	}
    }
    else
    {
	/*
	 * Reset break character to what user had when invoked
	 * (providing it is different from current one)
	 */
	if (save_t_brkc != tchars.t_brkc)
	{
	    tchars.t_brkc = save_t_brkc;
	    ioctl (SHIN, TIOCSETC, &tchars);
	}
    }
    sigrelse (SIGINT);
}

static
termchars ()
{
    extern char *tgetstr ();
    char bp[1024];
    static char area[256];
    static int been_here = 0;
    char *ap = area;
    register char *s;

    if (been_here)
	return;
    been_here = TRUE;

    if (tgetent (bp, getenv ("TERM")) != 1)
        return;
    if (s = tgetstr ("vb", &ap))		/* Visible Bell */
	BELL = s;
    return;
}

/*
 * Move back to beginning of current line
 */
static
back_to_col_1 ()
{
    struct sgttyb tty, tty_normal;
    sigignore (SIGINT);
    ioctl (SHIN, TIOCGETP, &tty);
    tty_normal = tty;
    tty.sg_flags &= ~CRMOD;
    ioctl (SHIN, TIOCSETN, &tty);
    (void) write (SHOUT, "\r", 1);
    ioctl (SHIN, TIOCSETN, &tty_normal);
    sigrelse (SIGINT);
}

/*
 * Push string contents back into tty queue
 */
static
pushback (string)
char  *string;
{
    register char  *p;
    struct sgttyb   tty, tty_normal;

    sigignore (SIGINT);
    ioctl (SHOUT, TIOCGETP, &tty);
    tty_normal = tty;
    tty.sg_flags &= ~ECHO;
    ioctl (SHOUT, TIOCSETN, &tty);

    for (p = string; *p; p++)
	ioctl (SHOUT, TIOCSTI, p);
    ioctl (SHOUT, TIOCSETN, &tty_normal);
    sigrelse (SIGINT);
}

/*
 * Concatonate src onto tail of des.
 * Des is a string whose maximum length is count.
 * Always null terminate.
 */
catn (des, src, count)
register char *des, *src;
register count;
{
    while (--count >= 0 && *des)
	des++;
    while (--count >= 0)
	if ((*des++ = *src++) == 0)
	    return;
    *des = '\0';
}

static
max (a, b)
{
    if (a > b)
	return (a);
    return (b);
}

/*
 * like strncpy but always leave room for trailing \0
 * and always null terminate.
 */
copyn (des, src, count)
register char *des, *src;
register count;
{
    while (--count >= 0)
	if ((*des++ = *src++) == 0)
	    return;
    *des = '\0';
}

/*
 * For qsort()
 */
static
fcompare (file1, file2)
char  **file1, **file2;
{
    return (strcmp (*file1, *file2));
}

static char
filetype (dir, file)
char *dir, *file;
{
    if (dir)
    {
	char path[512];
	struct stat statb;
	strcpy (path, dir);
	catn (path, file, sizeof path);
	if (stat (path, &statb) >= 0)
	{
	    if (statb.st_mode & S_IFDIR)
		return ('/');
	    if (statb.st_mode & 0111)
		return ('*');
	}
    }
    return (' ');
}

/*
 * Print sorted down columns
 */
static
print_by_column (dir, items, count, l_for_command)
register char *dir, *items[];
{
    register int i, rows, r, c, maxwidth = 0, columns;
    for (i = 0; i < count; i++)
	maxwidth = max (maxwidth, strlen (items[i]));
    maxwidth += l_for_command ? 1:2;	/* for the file tag and space */
    columns = 80 / maxwidth;
    rows = (count + (columns - 1)) / columns;
    for (r = 0; r < rows; r++)
    {
	for (c = 0; c < columns; c++)
	{
	    i = c * rows + r;
	    if (i < count)
	    {
		register int w;
		printf("%s", items[i]);
		w = strlen (items[i]);
		/* Print filename followed by '/' or '*' or ' ' */
		if (!l_for_command)
			putchar (filetype (dir, items[i])), w++;
		if (c < (columns - 1))			/* Not last column? */
		    for (; w < maxwidth; w++)
			putchar (' ');
	    }
	}
	printf ("\n");
    }
}

/*
 * expand "old" file name with possible tilde usage
 *		~person/mumble
 * expands to
 *		home_directory_of_person/mumble
 * into string "new".
 */

char *
tilde (new, old)
char *new, *old;
{
    extern struct passwd *getpwuid (), *getpwnam ();

    register char *o, *p;
    register struct passwd *pw;
    static char person[40] = {0};

    if (old[0] != '~')
    {
	strcpy (new, old);
	return (new);
    }

    for (p = person, o = &old[1]; *o && *o != '/'; *p++ = *o++);
    *p = '\0';

    if (person[0] == '\0')			/* then use current uid */
	pw = getpwuid (getuid ());
    else
	pw = getpwnam (person);

    if (pw == NULL)
	return (NULL);

    strcpy (new, pw -> pw_dir);
    (void) strcat (new, o);
    return (new);
}

/*
 * Cause pending line to be printed
 */
static
retype ()
{
    int     pending_input = LPENDIN;
    ioctl (SHOUT, TIOCLBIS, &pending_input);
}

static
beep ()
{
    (void) write (SHOUT, BELL, strlen(BELL));
}


/*
 * parse full path in file into 2 parts: directory and file names
 * Should leave final slash (/) at end of dir.
 */
static
dir_name (path, dir, name)
char   *path, *dir, *name;
{
    extern char *rindex ();
    register char  *p;
    p = rindex (path, '/');
    if (p == NULL)
    {
	copyn (name, path, MAXNAMLEN);
	dir[0] = '\0';
    }
    else
    {
	p++;
	copyn (name, p, MAXNAMLEN);
	copyn (dir, path, p - path);
    }
}


char *
getentry (dir_fd, l_for_lognames)
DIR *dir_fd;
{
    if (l_for_lognames)			/* Is it login names we want? */
    {
	extern struct passwd *getpwent ();
	register struct passwd *pw;
	if ((pw = getpwent ()) == NULL)
	    return (NULL);
	return (pw -> pw_name);
    }
    else					/* It's a dir entry we want */
    {
	register struct direct *dirp;
	if (dirp = readdir (dir_fd))
	    return (dirp -> d_name);
	return (NULL);
    }
}

static
free_items (items)
register char **items;
{
    register int i;
    for (i = 0; items[i]; i++)
	free (items[i]);
    free (items);
}

#define FREE_ITEMS(items)\
{\
    sighold (SIGINT);\
    free_items (items);\
    items = NULL;\
    sigrelse (SIGINT);\
}

#define FREE_DIR(fd)\
{\
    sighold (SIGINT);\
    closedir (fd);\
    fd = NULL;\
    sigrelse (SIGINT);\
}

static int  dirctr;		/* -1 0 1 2 ... */
static char dirflag[5];		/*  ' nn\0' - dir #s -  . 1 2 ... */

/*
 * Strip next directory from path; return ptr to next unstripped directory.
 */
 
char *dir_f_path (path, dir)
char *path, dir[];
{
    register char *d = dir;

    while (*path && (*path == ' ' || *path == ':')) path++;
    while (*path && (*path != ' ' && *path != ':')) *(d++) = *(path++);
    while (*path && (*path == ' ' || *path == ':')) path++;

    ++dirctr;
    if (*dir == '.')
        strcpy (dirflag, " .");
    else
    {
        dirflag[0] = ' ';
	if (dirctr <= 9)
	{
		dirflag[1] = '0' + dirctr;
		dirflag[2] = '\0';
	}
	else
	{
		dirflag[1] = '0' + dirctr / 10;
		dirflag[2] = '0' + dirctr % 10;
		dirflag[3] = '\0';
	}
    }
    *(d++) = '/';
    *d = 0;

    return path;
}

/*
 * Perform a RECOGNIZE or LIST command on string "word".
 */
static
search (word, wp, command, routine, max_word_length, l_for_command)
char   *word,
       *wp;			/* original end-of-word */
COMMAND command;
int (*routine) ();
{
#   define MAXITEMS 2048
    register numitems,
	    name_length,		/* Length of prefix (file name) */
	    l_for_lognames;	/* True if looking for login names */
    int	    showpathn;			/* True if we want path number */
    struct stat
	    dot_statb,			/* Stat buffer for "." */
	    curdir_statb;		/* Stat buffer for current directory */
    int	    dot_scan,			/* True if scanning "." */
	    dot_got;			/* True if have scanned dot already */
    char    tilded_dir[FILSIZ + 1],	/* dir after ~ expansion */
	    dir[FILSIZ + 1],		/* /x/y/z/ part in /x/y/z/f */
            name[MAXNAMLEN + 1],	/* f part in /d/d/d/f */
            extended_name[MAXNAMLEN+1],	/* the recognized (extended) name */
            *entry,			/* single directory entry or logname */
	    *path;			/* hacked PATH environment variable */
    static DIR 
	    *dir_fd = NULL;
    static char
           **items = NULL;		/* file names when doing a LIST */

    if (items != NULL)
	FREE_ITEMS (items);
    if (dir_fd != NULL)
	FREE_DIR (dir_fd);

    l_for_lognames = (*word == '~') && (index (word, '/') == NULL);
    l_for_command &= (*word != '~') && (index (word, '/') == NULL);

    if (l_for_command)
    {
        copyn (name, word, MAXNAMLEN);
        if ((path = getenv ("PATH")) == NULL)
	    path = "";
	/* setup builtins as 1st to search before PATH */
	copyn (dir, BUILTINS, sizeof dir);

	dirctr = -1;		/* BUILTINS -1 */
	dirflag[0] = 0;
    }
    numitems = 0;

    dot_got = FALSE;
    stat (".", &dot_statb);

cmdloop:	/* One loop per directory in PATH, if l_for_command */

    if (l_for_lognames)			/* Looking for login names? */
    {
	setpwent ();				/* Open passwd file */
	copyn (name, &word[1], MAXNAMLEN);	/* name sans ~ */
    }
    else
    {						/* Open directory */
        if (!l_for_command)
	    dir_name (word, dir, name);
	if ((tilde (tilded_dir, dir) == 0) ||	/* expand ~user/... stuff */
	    
	   ((dir_fd = opendir (*tilded_dir ? tilded_dir : ".")) == NULL))
	{
	    if (l_for_command)
	        goto try_next_path;
	    else
		return (0);
	}
	dot_scan = FALSE;
	if (l_for_command)
	{
	    /*
	     * Are we searching "."?
	     */
	    fstat (dir_fd->dd_fd, &curdir_statb);
	    if (curdir_statb.st_dev == dot_statb.st_dev &&
	        curdir_statb.st_ino == dot_statb.st_ino)
	    {
	        if (dot_got)			/* Second time in PATH? */
			goto try_next_path;
		dot_scan = TRUE;
		dot_got = TRUE;
	    }
	}
    }

    name_length = strlen (name);
    showpathn = l_for_command && is_set("listpathnum");

    while (entry = getentry (dir_fd, l_for_lognames))
    {
	if (!is_prefix (name, entry))
	    continue;

	/*
	 * Don't match . files on null prefix match
	 */
	if (name_length == 0 && entry[0] == '.' && !l_for_lognames)
	    continue;

	/*
	 * Skip non-executables if looking for commands:
	 * Only done for directory "." for speed.
	 * (Benchmarked with and without:
	 * With filetype check, a full search took 10 seconds.
	 * Without filetype check, a full search took 1 second.)
	 *                                   -Ken Greer
         */
	if (l_for_command && dot_scan && filetype (dir, entry) != '*')
	    continue;

	if (command == LIST)		/* LIST command */
	{
	    extern char *malloc ();
	    register int length;
	    if (numitems >= MAXITEMS)
	    {
		printf ("\nYikes!! Too many %s!!\n",
		    l_for_lognames ? "names in password file":"files");
		break;
	    }
	    if (items == NULL)
	    {
		items = (char **) calloc (sizeof (items[1]), MAXITEMS + 1);
		if (items == NULL)
		    break;
	    }
	    length = strlen(entry) + 1;
	    if (showpathn)
		length += strlen(dirflag);
	    if ((items[numitems] = malloc (length)) == NULL)
	    {
		printf ("out of mem\n");
		break;
	    }
	    copyn (items[numitems], entry, MAXNAMLEN);
	    if (showpathn)
	        catn (items[numitems], dirflag, MAXNAMLEN);
	    numitems++;
	}
	else					/* RECOGNIZE command */
	    if (recognize (extended_name, entry, name_length, ++numitems))
		break;
    }

    if (l_for_lognames)
	endpwent ();
    else
	FREE_DIR (dir_fd);

try_next_path:
    if (l_for_command && *path &&
    	(path = dir_f_path (path, dir), dir)) 
    	goto cmdloop;
    
    if (command == RECOGNIZE && numitems > 0)
    {
	if (l_for_lognames)
	    copyn (word, "~", 1);
	else if (l_for_command)
	    word[0] = 0;
	else
	    copyn (word, dir, max_word_length);		/* put back dir part */
	catn (word, extended_name, max_word_length);	/* add extended name */
	while (*wp) (*routine) (*wp++);
	return (numitems);
    }

    if (command == LIST)
    {
	qsort (items, numitems, sizeof (items[1]), fcompare);
	print_by_column (l_for_lognames ? NULL:tilded_dir, items,
			 numitems, l_for_command);
	if (items != NULL)
	    FREE_ITEMS (items);
    }
    return (0);
}

/*
 * Object: extend what user typed up to an ambiguity.
 * Algorithm:
 * On first match, copy full entry (assume it'll be the only match) 
 * On subsequent matches, shorten extended_name to the first
 * character mismatch between extended_name and entry.
 * If we shorten it back to the prefix length, stop searching.
 */
recognize (extended_name, entry, name_length, numitems)
char *extended_name, *entry;
{
    if (numitems == 1)				/* 1st match */
	copyn (extended_name, entry, MAXNAMLEN);
    else					/* 2nd and subsequent matches */
    {
	register char *x, *ent;
	register int len = 0;
	for (x = extended_name, ent = entry; *x && *x == *ent++; x++, len++);
	*x = '\0';				/* Shorten at 1st char diff */
	if (len == name_length)			/* Ambiguous to prefix? */
	    return (-1);			/* So stop now and save time */
    }
    return (0);
}

/*
 * return true if check items initial chars in template
 * This differs from PWB imatch in that if check is null
 * it items anything
 */
static
is_prefix (check, template)
char   *check,
       *template;
{
    register char  *check_char,
                   *templ_char;

    check_char = check;
    templ_char = template;
    do
	if (*check_char == 0)
	    return (TRUE);
    while (*check_char++ == *templ_char++);
    return (FALSE);
}

starting_a_command (wordstart, inputline)
register char *wordstart, *inputline;
{
    static char
	    *cmdstart = ";&(|`",
	    *cmdalive = " \t'\"";
    while (--wordstart >= inputline)
    {
	if (index (cmdstart, *wordstart))
	    break;
	if (!index (cmdalive, *wordstart))
	    return (FALSE);
    }
    if (wordstart > inputline && *wordstart == '&')	/* Look for >& */
    {
	while (wordstart > inputline &&
			(*--wordstart == ' ' || *wordstart == '\t'));
	if (*wordstart == '>')
		return (FALSE);
    }
    return (TRUE);
}

tenematch (inputline, inline_size, num_read, command, command_routine)
char   *inputline;		/* match string prefix */
int     inline_size;		/* max size of string */
int	num_read;		/* # actually in inputline */
COMMAND command;		/* LIST or RECOGNIZE */
int	(*command_routine) ();	/* either append char or display char */

{
    static char 
	    *delims = " '\"\t;&<>()|^%";
    char word [FILSIZ + 1];
    register char *str_end, *word_start, *cmd_start, *wp;
    int space_left;
    int is_a_cmd;		/* UNIX command rather than filename */

    str_end = &inputline[num_read];

   /*
    * Find LAST occurence of a delimiter in the inputline.
    * The word start is one character past it.
    */
    for (word_start = str_end; word_start > inputline; --word_start)
	if (index (delims, word_start[-1]))
	    break;

    space_left = inline_size - (word_start - inputline) - 1;

    is_a_cmd = starting_a_command (word_start, inputline);

    for (cmd_start = word_start, wp = word; cmd_start < str_end;
    	 *wp++ = *cmd_start++);
    *wp = 0;
   
    return search (word, wp, command, command_routine, space_left, is_a_cmd);
}

char *CharPtr;
static
CharAppend (c)
{
    putchar (c);
    *CharPtr++ = c;
    *CharPtr   = 0;
}
    
tenex (inputline, inline_size)
char   *inputline;
int     inline_size;
{
    register int numitems, num_read;

    setup_tty (ON);
    termchars ();
    while((num_read = read (SHIN, inputline, inline_size)) > 0)
    {
	register char *str_end, last_char, should_retype;
	COMMAND command;
	int tty_local = 0;			/* tty "local mode" bits */

	last_char = inputline[num_read - 1] & 0177;

	if (last_char == '\n' || num_read == inline_size)
	    break;

	ioctl (SHIN, TIOCLGET, &tty_local);

	if (last_char == ESC)		/* RECOGNIZE */
	{
	    if (tty_local & LCTLECH)
		printf ("\210\210  \210\210");	/* Erase ^[ */
	    /*
	    if (num_read == 1)
	    {
	        num_read = tenedit (inputline, inline_size, "");
		break;
	    }
	    else	
	    */
		command = RECOGNIZE;
		num_read--;
	}
	else				/* LIST */
	    command = LIST,
	    putchar ('\n');

	CharPtr = str_end = &inputline[num_read];
	*str_end = '\0';

	numitems = tenematch (inputline, inline_size, num_read, command,
			  command == LIST ? putchar : CharAppend);
	flush ();
			  
	if (command == RECOGNIZE)
	    if (numitems != 1) 			/* Beep = No match/ambiguous */
		beep ();

	/*
	 * Tabs in the input line cause trouble after a pushback.
	 * tty driver won't backspace over them because column positions
	 * are now incorrect. This is solved by retyping over current line.
	 */
	should_retype = FALSE;
	if (index (inputline, '\t')		/* tab in input line? */
	    || (tty_local & LCTLECH) == 0)	/* Control chars don't echo? */
	{
	    back_to_col_1 ();
	    should_retype = TRUE;
	}
	if (command == LIST)			/* Always retype after LIST */
	    should_retype = TRUE;

	if (should_retype)
	    printprompt ();

	pushback (inputline);

	if (should_retype)
	    retype ();
    }

    setup_tty (OFF);

    return (num_read);
}

#ifdef TEST

short SHIN = 0, SHOUT = 1;

printprompt ()
{
    (void) write (SHOUT, "-> ", 3);
    return (1);
}

main (argc, argv)
char **argv;
{
    char    string[128];
    int n;
    while (printprompt () && (n = tenex (string, 127)) > 0)
    {
	string[n] = '\0';
	printf ("Tenex returns \"%s\"\n", string);
    }
}
#endif


