#include "util.h"                 /* to get mmdf reply codes            */
#include "mmdf.h"                 /* to get mmdf reply codes            */
#include "cnvtdate.h"
#include <pwd.h>
#include <signal.h>
#include <sys/stat.h>

/*  Upgrade to use MMDF Submit; 10 Nov. 1978; David H. Crocker
 *    1 Dec 78 Dave Crocker:  add mail-reading, approx like old "mail"
 *   15 Dec 78 Dave Crocker:  add sndmsg & memo & "-" interfaces
 *   16 Dec 78 Dave Crocker:  "-" not generate return address
 *   25 Apr 80 Dave Crocker:  fix .mail acquisition, default to save
 *   14 Jun 80 Dave Crocker:  turn tty writing off/on
 *   17 Jul 80 Dave Crocker:  fix -c
 *   24 Sep 80 Dave Crocker:  add -r
 *   12 Oct 80 Dave Crocker:  add -g & signature file
 *   15 Nov 80 Dave Crocker:  conversion to V7 package & use of mm_io
 *    1 Dec 80 Dave Crocker:  check success of ALL system calls
 *   20 Jul 81 Dave Crocker:  fix dobody() to detect end from terminal
 *   16 Jul 82 Dave Crocker:  make addresses have '@' only; no " at "
 *    6 Dec 83 Doug Kingston: remove tty write stuff, this is not the place.
 *   19 May 83 Doug Kingston: fix -r to pass a flag to submit
 */

long int    curnewpos,
	    poshack;              /* keep track of current position     */
				  /*   because ftell isn't on V6        */
FILE *  newfp;                    /* handle on .mail file               */

char   *logdir;
char   *username;
char   *mbxname;                  /* full pathname to mailbox           */
char    *binbox;
char    ttyobuf[BUFSIZ];          /* for buffering ttyoutput            */
char	*index();
char    *strdup();

/* mmdf globals */

extern int sentprotect;           /* default mailbox protection         */
extern char     *mldflfil;        /* default mailbox file               */
extern char     *mldfldir;        /* directory containing mailbox file  */
extern char     *delim1;          /* string delimiting messages         */
extern char     *locname;
extern char     *locdomain;

/* end of mmdf globals */


char    noret;                    /* no return address                  */
char    watchit;                  /* user wants to watch delivery       */
char    qrychar;                  /* user-specified default save status */

char    usertype;
#define U_MAIL  0                 /* the original mail command          */
#define U_SND   1                 /* sndmsg-type prompting              */
#define U_MEMO  2                 /* memo prompting                     */
#define U_LMEM  3                 /* logged in as memo, for memo        */

#define FRMNONE 0                 /* no From field specified            */
#define FRMTXT  1                 /* From field had text only           */
#define FRMSNDR 2                 /* field had sender info, too         */


/* **************************  MAIN  ******************************* */

main (argc, argv)
int     argc;
char   *argv[];
{
    setbuf (stdout, ttyobuf);
    mmdf_init (argv[0]);
    pgminit ();

    if (initstr ("snd", argv[0], 3)
	    || strindex ("/snd", argv[0]) != -1
	    || initstr ("snd", &argv[0][1], 3)
	    || strindex ("sndmsg", argv[0]) != -1)
	usertype = U_SND;
    else
	if (strindex ("memo", argv[0]) != -1)
	    usertype = U_MEMO;
	else
	    if (argv[0][0] == '-' && argv[0][1] == '\0')
	    {                     /* "-" => login shell   */
		usertype = U_LMEM;
		signal (SIGHUP, SIG_IGN);
				  /* renable these for exiting  */
		prompt ("Send a memo\n");
	    }
    if (usertype == U_MAIL && (argc == 1 || (argc == 2 && *argv[1] == '-')))
	readmail (argc, argv);    /* only when invoked at "mail"  */
    else
	sendmail (argc, argv);
}
/**/

leave (val)
int     val;
{
    if (newfp)
	lk_fclose(newfp, mbxname, (char *)0, (char *)0);
    exit (val == OK ? 0 : 99);
}

pipsig ()
{
    if (rp_gval (endchild (NOTOK)) == RP_NO)
	prompt ("Problem accessing mail submission program\n");
				  /* only comment if it looks like      */
				  /*  submit died ugly                  */
    leave (NOTOK);
}

pgminit ()
{
    extern struct passwd *getpwuid ();
    extern char *getmailid ();
    struct passwd  *pwdptr;
    int     realid,
	    effecid;

    signal (SIGPIPE, pipsig);    /* catch write to bad pipe            */

    getwho (&realid, &effecid);   /* who am i?                          */

    if ((pwdptr = getpwuid (realid)) == (struct passwd *) NULL)
	err_abrt (RP_PARM, "Unable to locate user's name");

    /*  save login directory   */
    logdir = strdup(pwdptr -> pw_dir);

    if ((username = getmailid(pwdptr -> pw_name)) == NULL)
	err_abrt (RP_PARM, "Unable to locate user's mailid");
}
/**/

readmail (argc, argv)
int     argc;
char   *argv[];
{
    register int    count;

    if (argc == 2)
	qrychar = argv[1][1];

    readinit ();
    for (count = 0; msgstrt () && showmsg () && savmsg (); count++);
    if (count == 0)
	err_abrt (RP_OK, "No new mail");
    endread ();
}

readinit ()
{
    extern char *multcat ();
    int     newfd;
    extern int  errno;

    mbxname = multcat (
		((mldfldir == 0 || isnull(*mldfldir)) ? logdir : mldfldir),
		 "/",
		((mldflfil == 0 || isnull(*mldflfil)) ? username : mldflfil),
		  0);
    binbox = multcat (
		((mldfldir == 0 || isnull(*mldfldir)) ? logdir : mldfldir),
		 "/._",
		((mldflfil == 0 || isnull(*mldflfil)) ? username : mldflfil),
		  0);
    if (access(binbox, 0) == 0) {
	fprintf(stderr, "Your mailbox has a binary box (%s),\n", binbox);
	fprintf(stderr, "probably created by another mail reading program like MSG.\n");
	fprintf(stderr, "You should remove the binary box before running this program\n");
	fprintf(stderr, "again, or use MSG instead to read your mail.\n");
	exit (99);
    }

    if ((newfd = lk_open (mbxname, 0, (char *)0, (char *)0, 5)) == -1)
	err_abrt (RP_OK,
		    (errno == ETXTBSY) ? "Mailbox is busy" : "No new mail");

    newfp = fdopen (newfd, "r");
    poshack = 0l;
}

endread ()
{
    fflush (stdout);

    if (creat (mbxname, sentprotect) == NOTOK)
				  /* truncate the file            */
	err_abrt (RP_FCRT, "Unable to reset %s mailbox", mbxname);
    leave (OK);
}
/**/

msgstrt ()
{
    char    linebuf[LINESIZE];

    FOREVER
    {
	curnewpos = poshack;
	if (fgets (linebuf, sizeof linebuf, newfp) == NULL)
	    return (FALSE);
	if (!strequ (linebuf, delim1))
	    break;
	poshack += strlen (linebuf);
    }
    fseek (newfp, curnewpos, 0);
    return (TRUE);
}

showmsg ()
{
    char    linebuf[LINESIZE];
    register int    lines;

    for (lines = 0; fgets (linebuf, sizeof linebuf - 1, newfp) != NULL;)
    {
	poshack += strlen (linebuf);
	if (strequ (linebuf, delim1))
	    return (TRUE);

	fputs (linebuf, stdout);
	if (lines++ > 4)
	{
	    fflush (stdout);
	    lines = 0;
	}
    }
    return ((ferror (newfp) || ferror (stdout)) ? FALSE : TRUE);
}
/**/

savmsg ()
{
    static  FILE * mboxfp;
    char    linebuf[LINESIZE];

    if (!querysav ())
	return (TRUE);

    if (mboxfp == NULL)
    {
	if ((mboxfp = fopen ("mbox", "a")) == NULL ||
		chmod ("mbox", sentprotect) == NOTOK)
	    err_abrt (RP_FOPN, "Unable to access/create mbox");
    }

    poshack = curnewpos;
    for ( fseek (newfp, curnewpos, 0);
	    fgets (linebuf, sizeof linebuf - 1, newfp) != NULL;
	    fputs (linebuf, mboxfp))
    {
	poshack += strlen (linebuf);
	if (strequ (linebuf, delim1))
	    break;
    }
    fputs (delim1, mboxfp);
    fflush (mboxfp);
    return (ferror (newfp) ? FALSE : TRUE);
}

querysav ()
{
    register int  respchar;

    if (qrychar == 0)
    {
	prompt ("\nSave? ");
	switch (respchar = getchar ())
	{
	    case '\n':
		break;
	    case '\0':
		exit (-1);
	    default:
		while (getchar () != '\n');
	}
    }
    else
	respchar = qrychar;
    if (respchar == 'n' || respchar == 'N')
	return (0);               /* deletion must be explicit            */
    return (1);                   /* default to saving                    */
}
/* *************************  SENDING ****************************** */

static char regargs[] = "rmxto,cc*";
  /* quick NS timeout, return to sender, mail, extract addrs from to & cc  */

sendmail (argc, argv)             /* Send a message                     */
int     argc;
char   *argv[];
{
    extern char *sbargs ();
    int     retval;
    char    linebuf[ADDRSIZE];
    char   *sbmtargs;
    char    hadfrom,
	    hadto,
	    hadcc;

    if (rp_isbad (mm_init ()) || rp_isbad (mm_sbinit ()))
	err_abrt (RP_MECH, "Unable to submit mail; please report this error");

    strcpy (linebuf, regargs);    /* standard stuff                     */

    if ((sbmtargs = sbargs (argc, argv)) != 0)
				  /* any args to be args to submit?       */
	strcat (linebuf, sbmtargs);

    if (usertype == U_LMEM)
	strcat (linebuf, "u");    /* do not verify the from field       */
    if (noret)
	strcat (linebuf, "q");    /* quiet, do not return on errors  */
    if (watchit)
	strcat (linebuf, "w");    /* user wants to watch the process     */

    if (rp_isbad (mm_winit ((char *) 0, linebuf, (char *) 0)))
	err_abrt (RP_MECH, "problem with submit message initialization");

    dodate ();

    hadfrom = dofrom (argc, argv);

    dosubject (argc, argv);

    if (hadfrom == FRMTXT && usertype != U_LMEM)
	dosender ("Sender:  ", (char *) 0, TRUE);
				/* simple address */
    hadto = doto (argc, argv);
    hadcc = docc (argc, argv);

    if (!hadto && !hadcc && usertype == U_MAIL)
	err_abrt (RP_PARM, "No addressees specified");

    doid (argc, argv);

    if (usertype != U_MAIL)
    {
	doprompt ();              /* get headers          */
	prompt ("Text:\n");
    }
    mm_wtxt ("\n", 1);           /* start the body             */


    dobody ();
    retval = endbody ();

    endchild (OK);

    if (usertype != U_MAIL && rp_isgood (retval))
	prompt ("Message posted\n");

    leave ((rp_isbad (retval)) ? NOTOK : OK);
}
/*  *************  ARGUMENT PARSING FOR SENDMAIL  **************  */

char *
	sbargs (argc, argv)               /* any args to be args to submit?   */
int     argc;
char   *argv[];
{
    register int    i;
    char   *argptr;

    for (i = 1, argptr = 0; i < argc; i++)
    {                             /* create message header fields */
	if (argv[i][0] == '-')
	    switch (argv[i][1])
	    {
		case '-':
		    argptr = &(argv[i][2]);
		    continue;

		case 'r':
		    noret = TRUE;
		    break;

		case 'w':
		    watchit = TRUE;
		    break;
	    }
    }
    return (argptr);
}

sndhdr (name, contents)
char	*name,
	*contents;
{
    char    linebuf[LINESIZE];

    sprintf (linebuf, "%-10s%s\n", name, contents);
    mm_wtxt (linebuf, strlen (linebuf));
}
/**/

dodate ()
{
    char    datbuf[64];

    cnvtdate (TIMREG, datbuf);    /* rfc733 format date                 */
    sndhdr ("Date:  ", datbuf);
}

doid (argc, argv)
int     argc;
char   *argv[];
{
    char    linebuf[LINESIZE];
    char    datbuf[64];
    register int    i;

    for (i = 1; i < argc; i++)
    {                             /* create message header fields */
	if (argv[i][0] == '-' && argv[i][1] == 'i')
	{
	    cnvtdate (TIMCOM, datbuf);
	    sprintf (linebuf, "<%s.%d@%s.%s>", datbuf, getpid(),
			locname, locdomain);
	    sndhdr ("Message-Id:  ", linebuf);
	    return;
	}
    }
}

dosubject (argc, argv)
int     argc;
char   *argv[];
{
    register int    i;

    for (i = 1; i < argc; i++)    /* create message header fields */
	if (argv[i][0] == '-' && argv[i][1] == 's')
	    sndhdr ("Subject:  ", argv[++i]);
}
/**/

dofrom (argc, argv)
int     argc;
char   *argv[];
{
    static char fldnam[] = "From:  ";
    register int    i;

    for (i = 1; i < argc; i++)
    {                             /* create message header fields */
	if (argv[i][0] == '-' && (argv[i][1] == 'f' || argv[i][1] == 'g'))
	{
	    if (argv[i][1] == 'g')
	    {
		dosender (fldnam, argv[++i], TRUE);
		return (FRMSNDR);
	    }
	    else                  /* no mailbox portion                 */
	    {
		sndhdr (fldnam, argv[++i]);
		return (FRMTXT);
	    }
	}
    }
    if (usertype != U_LMEM)       /* not memo                     */
	dosender (fldnam, (char *) 0, TRUE);

    return (FRMNONE);
}
/**/

dosender (cmpnt, name, fancy)
char    cmpnt[],
	name[];
int     fancy;          /* make address fancy? */
{
    int     sigfd,
	    sigsiz;
    char   *ptr,
	    linebuf[ADDRSIZE],
	    sigtxt[FILNSIZE];     /* where is signature text?           */
    char    gotsig;

    gotsig = FALSE;

    if (!fancy || name != 0)
    {
	strcpy (sigtxt, name);
	gotsig = TRUE;
    }
    else
    {                             /* user didn't give us a signature    */
	sprintf (linebuf, "%s/.signature", logdir);

	if ((sigfd = open (linebuf, 0)) >= 0)
	{                         /* there is a file w/signature?       */
	    if ((sigsiz = read (sigfd, sigtxt, sizeof sigtxt)) > 0) {
	    	char *cp;

		sigtxt[sigsiz - 1] = '\0';
	    	if (cp = index(sigtxt, '\n'))
	    	    *cp = '\0';
	    	if (sigtxt[0])
		    gotsig = TRUE;
	    }
	}
    }

    if (gotsig)                   /* real name + mailbox                */
    {
	for (ptr = locname; *ptr != 0; ptr++)
	    *ptr = uptolow (*ptr);      /* Screw locname */
	sprintf (linebuf, "%s <%s@%s>", sigtxt, username, locname);
    }
    else                          /* just the mailbox info              */
	sprintf (linebuf, "%s@%s", username, locname);

    sndhdr (cmpnt, linebuf);
}
/**/

doto (argc, argv)
int     argc;
char   *argv[];
{
    char    linebuf[ADDRSIZE];
    char    someto;
    register int    i;

    someto = FALSE;
    if (usertype == U_MAIL && argv[1][0] != '-')
    {
	someto = TRUE;
	i = dolist (1, argc, argv, linebuf);
	sndhdr ("To:  ", linebuf);
    }
    else
	i = 1;

    for (; i < argc; i++)
    {                             /* create message header fields */
	if (argv[i][0] == '-' && argv[i][1] == 't')
	{
	    someto = TRUE;
	    i = dolist (i + 1, argc, argv, linebuf);
	    sndhdr ("To:  ", linebuf);
	}
    }
    return (someto ? TRUE : FALSE);
}
/**/

docc (argc, argv)
int     argc;
char   *argv[];
{
    char    linebuf[ADDRSIZE];
    char    somecc;
    register int    i;

    for (somecc = FALSE, i = 1; i < argc; i++)
	if (argv[i][0] == '-' && argv[i][1] == 'c')
	{
	    somecc = TRUE;
	    i = dolist (i + 1, argc, argv, linebuf);
	    sndhdr ("cc:  ", linebuf);
	}

    return (somecc ? TRUE : FALSE);
}

dolist (i, argc, argv, dest)
register int    i;
int     argc;
char   *argv[];
char   *dest;                     /* where to put the list                */
{
    *dest = '\0';                 /* in case nothing found                */
    if (i < argc && *argv[i] != '-')
	for (strcat (dest, argv[i++]); i < argc && *argv[i] != '-';
		strcat (dest, ", "), strcat (dest, argv[i++]));

    return (i - 1);
}
/**/

doprompt ()
{
    if (usertype == U_LMEM)       /* "free" memo          */
	cpyprompt ("From:  ");
    cpyprompt ("To:  ");
    if (usertype == U_SND)        /* sndmsg               */
	cpyprompt ("cc:  ");
    cpyprompt ("Subject:  ");
}

prompt (str)
char   *str;
{
    printf ("%s", str);
    fflush (stdout);
}

cpyprompt (prom)
char    prom[];
{
    prompt (prom);
    copylin (prom);
}

copylin (prelim)
char   *prelim;
{
    char    linebuf[LINESIZE];
    register int    morin;
    register int    c;

    for (morin = TRUE; morin; sndhdr (prelim, linebuf), prelim = "")
    {
	morin = FALSE;
	if (fgets (linebuf, sizeof linebuf, stdin) == NULL)
	    err_abrt (RP_LIO, "Input error");

	if (linebuf[0] == '\n')
	    return;

	c = strlen (linebuf);
	linebuf[c - 1] = '\0';
	if (linebuf[c - 2] == '\\')
	{
	    morin = TRUE;
	    linebuf[c-- - 2] = '\n';
	}
    }
}
/**/

dobody ()
{
    char    buffer[BUFSIZ];
    register int    i;

    while (!feof (stdin) && !ferror (stdin) &&
	    (i = fread (buffer, sizeof (char), sizeof (buffer), stdin)) > 0)
	if (rp_isbad (i = mm_wtxt (buffer, i)))
		err_abrt (i, "Problem writing body");

    if (ferror (stdin))
	err_abrt (RP_FIO, "Problem reading body");
}

endbody ()
{
    struct rp_bufstruct thereply;
    int     len;

    if (rp_isbad (mm_wtend ()))
	err_abrt (RP_MECH, "problem ending submission");

    if (rp_isbad (mm_rrply (&thereply, &len)))
	err_abrt (RP_MECH, "problem getting submission status");

    if (rp_isbad (thereply.rp_val))
	err_abrt (thereply.rp_val, "%s", thereply.rp_line);

    return (thereply.rp_val);
}

doreset ()
{
    exit (NOTOK);
}

/* *********************  UTILITIES  ***************************  */

endchild (type)
int     type;
{
    int retval;


    if (rp_isgood (retval = mm_sbend ()))
	retval = mm_end (type);

    return ((retval == NOTOK) ? RP_FIO : (retval >> 8));
}

/*VARARGS2*/
err_abrt (code, fmt, b, c, d)     /* terminate the process              */
int     code;                     /* a mmdfrply.h termination code      */
char   *fmt, *b, *c, *d;
{
    if (fmt) {
	printf (fmt, b, c, d);
    	putchar ('\n');
    }
    if (code != RP_OK)
	printf ("Message posting aborted\n");
    fflush (stdout);
    endchild (NOTOK);
    leave (code);
}
