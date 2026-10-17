/*
 *                      N I C T A B L E . C
 *
 *                    (Revised as of 28 March)
 *
 *  Generates domain or channel tables from dhosts.txt
 *  (Based on code for dmtable.c from Steve Kille)
 *
 *    nictable <-D,-C,-T> [infile] [-d domain] [-t transport] [-s service]
 *      infile is location of dhosts.txt
 *      -D build a domain table
 *      -C build a channel table
 *      -T build a top table
 *      -d specifies domain to look for (e.g. ARPA or CSNET)
 *      -t specifies transport protocol (e.g. TCP)
 *      -s specifies service to look for (e.g. SMTP)
 *
 */
#include "util.h"
#include "mmdf.h"

char    *domain,                /* domain to add                        */
	*transport,             /* transport service to look for        */
	*service;               /* name of service to look for          */
int     domainlen = 0;
int     target = 0;
#define T_DOMAIN        1
#define T_CHANNEL       2
#define T_TOP           3

FILE    *infp = stdin;

char    entry[1000],            /* fully-assembled entry (multi-line) */
	linebuf[1000];          /* current line */

extern  char *index();
extern  char *rindex();
extern  char *strncpy();

main (argc, argv)
    int argc;
    char *argv[];
{
    mmdf_init (argv[0]);
    arg_init (argc, argv);

    doit ();
    exit (0);
}
/**/

arg_init (argc, argv)
    int argc;
    char *argv[];
{
    register int ind;

    if (argc < 2) {
	fputs ("Usage:  nictable <-D,-C,-T> [infile] [-d domain] [-t transport] [-s service]\n",
		stderr);
	exit(NOTOK);
    }

    for (ind = 1; ind < argc; ind++)
    {
	if (argv[ind][0] != '-')
	{
	    if ((infp = fopen (argv[ind], "r")) == NULL)
	    {
		fprintf (stderr,  "Unable to open '%s':", argv[ind]);
		perror ("");
		exit (NOTOK);
	    }
	}
	else
	    switch (argv[ind][1])
	    {
		case 'C':
		    target = T_CHANNEL;
		    break;

		case 'D':
		    target = T_DOMAIN;
		    break;

		case 'T':
		    target = T_TOP;
		    break;

		case 's':
		    service = argv[++ind];
		    break;

		case 't':
		    transport = argv[++ind];
		    break;

		case 'd':
		    domain = argv[++ind];
		    domainlen = strlen (domain);
		    break;
	    }
    }

    if (target == 0) {
	fputs ("nictable: you must specify target type (-C, -D, -T)\n", stderr);
	exit(NOTOK);
    }
}
/**/

#define NUMPARTS    75

doit ()
{
    int argc;
    char *argv[NUMPARTS];       /* fields of entry */

    fgets (linebuf, sizeof (linebuf), infp);
				/* load first line into buffer */
    while (getentry ())
    {
	argc = cstr2arg (entry, NUMPARTS, argv, ':');
	switch (argc)
	{
	    case -1:
		fprintf (stderr, "problem parsing entry '%s':", entry);
		continue;

	    case 0:
	    case 1:
	    case 2:
		continue;
	}

	if (!lexequ (argv[0], "host"))
	    continue;           /* only process host entries    */

	if (goodservice (argv [argc - 2]))
	    output (argc, argv);
    }
}
/**/

getentry ()
{
    if (linebuf[0] != '\0')
    {
	if (linebuf[0] != ';')
	    (void) strcpy (entry, linebuf);
	linebuf[0] = '\0';
    }
    else
	entry[0] = '\0';

    while (fgets (linebuf, sizeof (linebuf), infp) != NULL)
	switch (linebuf[0])
	{
	    case ';':           /* comment */
		break;

	    case ' ':           /* continuation */
	    case '\t':
		strcat (entry, linebuf);
		break;

	    default:            /* new entry */
		spstrip (entry);
		return (TRUE);
	}

    if (ferror (stdin))
    {
	perror ("problem reading host table");
	exit (NOTOK);
    }

    spstrip (entry);
    return ((entry[0] == '\0') ? FALSE : TRUE);
}
/**/

goodservice (str)      /* entry have right transport/service? */
char *str;
{
	int sargc;
	char *sargv[NUMPARTS];
	char *ts;               /* transport from entry         */
	char *sv;               /* service from entry           */
	int i;

	if (transport == (char *)0 && service == (char *)0)
		return (TRUE);

	sargc = cstr2arg (str, NUMPARTS, sargv, ',');
	for (i = 0; i < sargc; i++)
	{
	    ts = sargv [i];
	    if ((sv = index (ts, '/')) != 0)
		    *sv++ = '\0';

	    if (transport != (char *)0 && !lexequ (transport, ts))
		    continue;

	    if (sv == 0)
		return (TRUE);          /* implies all service  */

	    if (service != (char *)0 && (!lexequ (service, sv)))
		    continue;

	    return (TRUE);
	}
	return (FALSE);
}

gooddomain (str)      /* entry have right domain? */
char *str;
{
	if (domain == (char *)0)
		return (TRUE);
	if (lexequ (domain, str + strlen(str) - domainlen))
		return (TRUE);
	return (FALSE);
}
/**/
/*ARGSUSED*/
output (argc, argv)       /* output the list of host references */
    int argc;
    char *argv[];
{
    char *oargv[NUMPARTS];
    char *p;
    int i;
    int oargc;
    char buf [LINESIZE];

    if (target == T_CHANNEL) {
	if ((p = index (argv [2], ',')) != 0)
	    *p = '\0';
	oargc = cstr2arg (argv [1], NUMPARTS, oargv, ',');
	for (i = 0; i < oargc; i++)
	    printf ("%s:%s\n", argv [2], oargv [i]);
    } else if (target == T_DOMAIN) {
	oargc = cstr2arg (argv [2], NUMPARTS, oargv, ',');
	if (gooddomain (oargv[0]))
	{
	    if (domainlen) {
		(void) strncpy (buf, oargv[0], strlen(oargv[0])-domainlen-1);
		buf [strlen(oargv[0]) - domainlen - 1]  = '\0';
	    } else {  /* KLUDGE: No domain specified, wing it! */
		(void) strcpy (buf, oargv[0]);
		if (p = rindex(buf, '.'))
		    *p = '\0';
	    }
	    printf ("%s:%s\n", buf, oargv [0]);
	    for (i = 1; i < oargc; i++)
		if ((index(oargv[i], '.') == (char *) 0)
			&& !lexequ (buf, oargv[i]))
		    printf ("%s:%s\n", oargv  [i], oargv [0]);
	}
    } else if (target == T_TOP) {
	oargc = cstr2arg (argv [2], NUMPARTS, oargv, ',');
	if (gooddomain (oargv[0]))
	{
	    for (i = 1; i < oargc; i++)
		if ((index(oargv[i], '.') != (char *) 0)
			&& !gooddomain (oargv[i]))
		    printf ("%s:%s\n", oargv  [i], oargv [0]);
	}
	else
	{
	    for (i = 0; i < oargc; i++)
		if (index(oargv[i], '.')  != (char *) 0)
		    printf ("%s:%s\n", oargv  [i], oargv [0]);
	}
    } else {
	/* Invalid target */
	fprintf (stderr, "Internal nictable error, target is %d\n", target);
	exit (NOTOK);
    }

}

spstrip (str)           /* strip LWSP chars                     */
char *str;
{
	char *src, *dest;

	for (src = dest = str; *src != '\0';)
		if (!isspace (*src))
			*dest++ = *src++;
		else
			src++;
	*dest ='\0';
}
