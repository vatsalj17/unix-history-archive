/*****************************************************************************
* Comment the channel lines out of a tailoring file.  This is intended to    *
* be run after fixtai has extraced the channel information into a            *
* static-initialization.  The looked-for commands MUST agree with            *
* the ones processed by fixtai.  The comment convention MUST agree           *
* with the one used in the uncomment program used just before fixtai         *
* is run                                                                     *
*                                                                            *
* Another subtlety is that the various functional pieces must agree          *
* with the corresponding pieces in fixtai, even though they have been        *
* recoded and may not look the same (e.g., cmdsrch, tai_token,               *
* lexequ).                                                                   *
*                                                                            *
* Wed May 30 08:42:42 EDT 1984, bpc                                          *
*****************************************************************************/

#include "util.h"
#include "cmd.h"

#define     COMMENT_CHAR    ';'
#define     COMMENT     ";:;"   /* Our special comment character */

extern int errno;
extern char *malloc();

FILE * tai_handle;      /* for reading the tailoring file */
#define MAXLINE     500 /* Max length of a line in the tailoring file */
char tai_line[MAXLINE];

/*
** These next MUST agree with the corresponding data in fixtai
*/
#define MMTBL           42
#define MMCHAN          43
Cmd mycmdtab[] =
{
    "mchn",        MMCHAN,     1,
    "mchan",       MMCHAN,     1,
    "mtable",      MMTBL,      1,
    "mtbl",        MMTBL,      1,
    0,             0,          0
};
/**/
main (argc, argv)
    char * argv[];
{
    if (argc != 2)
    {
	fprintf (stderr, "usage: %s tailoringfile_name\n", argv[0]);
	exit (1);
    }
    tai_handle = fopen (argv[1], "r");
    if (tai_handle == NULL)
    {
	fprintf (stderr, "Can't access tailoring file '%s'", argv[1]);
	exit (1);
    }
    read_line ();
    for ( ; ; )
    {
	char token[100];    /* First token on the line */
	
	if (tai_token (tai_line, token) != OK
	                        || mycmdsrch (token, mycmdtab) != OK)
	{
	    printf ("%s", tai_line);
	    read_line ();
	}
	else
	    do 	        /* It is one of our commands, so 'remove' it */
	    {
	        printf ("%s%s", COMMENT, tai_line);
	        read_line ();
	    } while (tai_token (tai_line, token) != OK);
    }
}
/**/
mycmdsrch (str, cmd)        /* find command. return token reference */
    char str[];                 /* test string  */
    register Cmd *cmd;          /* table of known commands */
{
    for ( ; (cmd -> cmdname) != (char *) 0; cmd++)
	if (lexequ (str, cmd -> cmdname))
	    return (OK);        /* got a hit */ 
    return (NOTOK);
}


/*****************************************************************************
*                           | r e a d _ l i n e |                            *
*                            *******************                             *
* Read the next line from the tailoring file.  If we hit EOF, just           *
* quit                                                                       *
*****************************************************************************/
read_line ()
{
    if (fgets (tai_line, MAXLINE-1, tai_handle) == NULL)
	exit (0);
}
/*/*****************************************************************************
*                           | t a i _ t o k e n |                            *
*                            *******************                             *
* Find the first token on the supplied line - copy it out as                 *
* requested                                                                  *
*****************************************************************************/
tai_token (strp, tokenp)
    register char * strp;
    register char * tokenp;
{
    if (!isalnum (*strp))
	return (NOTOK);
    for ( ; ; )
	switch (*strp)
	{
	    default:        /* just copy it                     */
		*tokenp++ = *strp++;
		break;

	    case '\\':      /* quote next character             */
	    case '=':   /* make '=' prefix to pair  */
	    case '\"':      /* beginning or end of string       */
		return (NOTOK); /* No commands we care about will
				   include this stuff */

	    case ' ':
	    case '\t':
	    case '\n':
	    case '\r':
	    case ',':
	    case ';':
	    case ':':
	    case '/':
	    case '|':
	    case '.':
	    case '\0':
		*tokenp = '\0';
		return (OK);
	}
}
