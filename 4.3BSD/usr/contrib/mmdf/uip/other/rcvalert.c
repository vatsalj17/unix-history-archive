#
/*
 *  RCVALERT personal receive-mail program
 *
 *  NOTE:  This is a USER program & is provided as a demonstration
 *         of an MMDF feature.  It is NOT part of the MMDF "package".
 *
 *  Notifies user of new mail, if user logged in, by printing scan line
 *  for the message on the user's terminal.
 *
 *  Does NOT actually deliver message.
 *
 *  Oct/79 David Crocker  Dept. of E.E., Univ. of Delaware
 *  Jun/80 David Crocker  Notify ALL tty's logged in as recipient
 *  Dec/80 David Crocker  Convert to stdio & v7
 *  Apr/83 Doug Kingston  Modified for BRL (mostly fixes)
 *  Apr/84 Doug Kingston  Modified for use as filter (rcvmail expunged)
 */
/**/

#undef	STDALONE /* run standalone, for debugging        */

#include "util.h"
#include "mmdf.h"
#include <pwd.h>

#define TOSHORT 16		  /* subject shorter than this => search  */
				  /*   body for chars to add to subject   */
#define SUBLEN  37		  /* length of subject & from fields      */
#define FROMLEN 15
#define UNSET   0		  /* nothing added to subject from body   */
#define SET     1		  /* something "    "    "     "    "     */

extern LLog *logptr;
extern char *compress();
extern FILE *tty_fp;

char    inbuf[BUFSIZ];

struct
{
    long    msglen;
    char    from[FROMLEN];
    char    subject[SUBLEN];
} scanl;			/* the line to be printed             */

int     sublen;                   /* chars in subject field of scan     */

char	username[10];		  /* string system name of recipient    */

/* *****************************  MAIN  **********************************/

main (argc, argv)
int     argc;
char   *argv[];
{
#ifndef STDALONE
    if (fork () != 0)		  /* let ch_local continue ASAP & do    */
	exit (RP_MECH);		  /*   actual delivery.                 */
				  /* child will do notification         */
#endif

    init(argc,argv);

    scanbld ();

    alert ();
    exit (0);
}

/**/
init(argc,argv)
int argc;
char **argv;
{
    if (argc > 1)
	scanl.msglen = atoi(argv[1]);
    else
	scanl.msglen = -1;

    mmdf_init ("ALERT");
    siginit ();

    setbuf (stdin, inbuf);

    guinfo ();			  /* who 'dis?                            */


}

/**/

guinfo ()			  /* get user name & login directory    */
{
    extern struct passwd *getpwuid ();
    register struct passwd *pwdptr;
    int     realid,
	    effecid;

    getwho (&realid, &effecid);
    pwdptr = getpwuid (effecid);

    strcpy (username, pwdptr -> pw_name);

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "id=%d, u='%s'", effecid, username);
#endif
}
/**/

alert ()                     /* send message to user's tty           */
{
    extern int errno;

    tty_init();

    for (;;)
    {
	if (tty_find (username) < OK)
	{
	    if (errno == EBUSY)
		continue;
	    else
		break;
	}

	fprintf (tty_fp, "\007\r\nNEW:  (");
	if (scanl.msglen < 0)
	    fprintf (tty_fp, " ???");
	else
	    if (scanl.msglen > 99999)
		fprintf (tty_fp, " BIG");
	    else
		fprintf (tty_fp, "%5ld", scanl.msglen);

	fprintf (tty_fp, ")  %-15s %s\r\n", scanl.from, scanl.subject);

	tty_close ();
    }

    tty_end ();
    return;
}
/**/

scanbld ()
{				  /* build scan line for tty              */
    char    tempbuf[512];
    register int    len;
    register char   addflag;

    sublen = 0;

    for (rewind (stdin); ;)
    {				  /* parse the headers                    */
	if (fgets (tempbuf, sizeof tempbuf, stdin) == NULL)
	    return;
	len = strlen (tempbuf) - 1;
	if (tempbuf[0] == '\n')
	    break;		  /* we done hit d' body                  */
	tempbuf[len] = '\0';

	if (isspace (tempbuf[0]))
	    continue;		  /* continuation line                    */

#ifdef DEBUG
	ll_log (logptr, LLOGFTR, "1: '%s'", tempbuf);
#endif
	if (equal ("from", tempbuf, 4))
	    getfrom (&tempbuf[4]);
	if (equal ("subject", tempbuf, 7))
	    getsub (&tempbuf[7]);
    }

    addflag = UNSET;		  /* not yet added any body to subject    */
    if (sublen < TOSHORT)	  /* see if subject should be added to    */
    {				  /*    from the body                     */
	while (sublen < (SUBLEN - 10) &&
		fgets (tempbuf, sizeof tempbuf, stdin) != NULL &&
		((len = strlen (tempbuf)) - 1) > 0)
	{
	    tempbuf[len - 1] = '\0';
				  /* get rid of newline                 */
#ifdef DEBUG
	    ll_log (logptr, LLOGFTR, "2:'%s'", tempbuf);
#endif
	    if (sublen > 0 && scanl.subject[sublen] != ' ')
				  /* separate from preceding text       */
		scanl.subject[sublen++] = ' ';

	    if (addflag == UNSET)
	    {
		scanl.subject[sublen++] = '(';
		addflag = SET;    /* we now have body text added        */
	    }

	    getsub (tempbuf);
	}
    }

    if (addflag == SET)
	strcpy (&(scanl.subject[sublen]), (len > 0) ? "...)" : ")");
    else
	scanl.subject[sublen] = '\0';
}
/**/

getfrom (src)                      /*  get name of sender                  */
register char  *src;
{
    register int    n;
    register char  *dest;

    while (*src++ != ':');        /* skip over to data                  */
    compress (src, src);

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "getfrom from '%s'", src);
#endif

    for (dest = scanl.from, n = (sizeof scanl.from); --n > 0;
	    *dest++ = *src++)
    {
	switch (*src)
	{
	    case '\0': 
	    case '\n': 
	    case '(':
	    case '<': 
	    case '@': 
	    case '%':
		break;

	    case ' ':
		if (equal (" at ", src, 4))
		    break;
	    default:
		continue;
	}
	break;
    }
}

getsub (src)			  /* get first part of subject            */
register char  *src;
{
    register char  *dest;

    while (isspace (*src))         /* skip over to data                  */
	src++;
    if (*src == ':')
	src++;
    compress (src, src);          /* eliminate leading white space        */

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "getsub from '%s'", src);
#endif

    for (dest = &(scanl.subject[sublen]);
	    sublen < (SUBLEN - 5) && !isnull (*src);
	    *dest++ = *src++, sublen++);
}
