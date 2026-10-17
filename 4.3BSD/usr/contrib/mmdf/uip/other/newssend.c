/*
 *	A program modelled after resend to feed news articles into
 *	the Internet Mail system.
 */

#include "util.h"                 /* to get mmdf reply codes            */
#include "mmdf.h"                 /* to get mmdf reply codes            */
#include "cnvtdate.h"
#include <signal.h>
#include <sys/stat.h>

extern char     *locname;
extern char     *locdomain;

char	*logdir;
char    *strdup();

char    noret;                    /* no return address                  */
char    watchit;                  /* user wants to watch delivery       */
int	hadto;

#define FRMNONE 0                 /* no From field specified            */
#define FRMTXT  1                 /* From field had text only           */
#define FRMSNDR 2                 /* field had sender info, too         */


char *byebyelines[] = {
	"resent",
	"reply-to",
	"arpa-address",
	"status",
	"article-i.d.",
	"article-id",
	"in-reply-to",
	"relay-version",
	"posting-version",
	"date-received",
	"distribution",
	"reference",
	"summary",
	"path",
	"organization",
	"lines",
	0
};
/* **************************  MAIN  ******************************* */

main (argc, argv)
int     argc;
char   *argv[];
{
	mmdf_init (argv[0]);
	pgminit ();

	sendmail (argc, argv);
}

pipsig ()
{
	if (rp_gval (endchild (NOTOK)) == RP_NO)
		err_abrt(RP_LIO, "Abnormal return from submit\n");
	exit (NOTOK);
}

pgminit ()
{
	int     realid,
	effecid;

	signal (SIGPIPE, pipsig);    /* catch write to bad pipe            */

	getwho (&realid, &effecid);   /* who am i?                          */
	if (setuid(effecid) < 0)	/* Need to be MMDF */
		err_abrt (RP_LIO, "Unable to setuid to effecid");
}
/*^L*/

char *newschannel = "smtp";
char regargs[100] = "kqrmthUSENET*";
/* quick NS timeout, do not return on error, return to sender, mail, trustme */

sendmail (argc, argv)             /* Send a message                     */
int     argc;
char   *argv[];
{
	char	*replyto;
	int     retval;
	int	i;

	if (rp_isbad (mm_init ()) || rp_isbad (mm_sbinit ()))
		err_abrt (RP_MECH, "Unable to submit mail; please report this error");

	if (lexequ("-r", argv[1])) {
		replyto = argv[2];
		argc -= 2;
		argv += 2;
	} else
		replyto = (char *)0;

	if (argc < 3)
		err_abrt (RP_MECH, "Usage:  newssend [-r replyto] faketo realto ...");

	if (rp_isbad (mm_winit (newschannel, regargs, replyto)))
		err_abrt (RP_MECH, "problem with submit message initialization");
	for (i = 2; i < argc; i++)
		if (rp_isbad(mm_wadr((char *)0, argv[i])))
			err_abrt (RP_MECH, "problem submitting address %s", argv[i]);
	mm_waend();

	dumpheader();
	sndhdr("To:", argv[1]);
	mm_wtxt("\n", 1);

	dobody ();
	retval = endbody ();

	endchild (OK);

	exit ((rp_isbad (retval)) ? NOTOK : OK);
}
/*  *************  ARGUMENT PARSING FOR SENDMAIL  **************  */

dumpheader()
{
	char	line[LINESIZE];
	int	found;

	found = 0;
	while (fgets (line, LINESIZE, stdin) != NULL) {
		register int j;

		if (line[0] == '\n')
			break;
		if (isspace(line[0]) && found)
			continue;
		found = 0;
		for (j = 0; byebyelines[j] != NULL; j++) {
			if (prefix(byebyelines[j], line)) {
				++found;
				break;
			}
		}
		if (found)
			continue;
		found = 0;
		mm_wtxt (line, strlen (line));
	}
}

sndhdr (name, contents)
char    name[],
contents[];
{
	char    linebuf[LINESIZE];

	sprintf (linebuf, "%-10s%s\n", name, contents);
	mm_wtxt (linebuf, strlen (linebuf));
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
		err_abrt (thereply.rp_val, thereply.rp_line);

	return (thereply.rp_val);
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

/* VARARGS2 */

err_abrt (code, fmt, b, c, d)     /* terminate the process              */
int     code;                     /* a mmdfrply.h termination code      */
char   *fmt,
*b,
*c,
*d;
{
	if (fmt) {
		printf (fmt, b, c, d);
		printf (" [%s]", rp_valstr (code));
		putchar ('\n');
	}
	printf ("Message posting aborted\n");
	fflush (stdout);
	endchild (NOTOK);
	exit (NOTOK);
}
