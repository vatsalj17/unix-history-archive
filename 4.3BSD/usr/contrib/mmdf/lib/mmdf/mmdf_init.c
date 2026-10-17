#include "util.h"
#include "mmdf.h"
#include "ch.h"

extern LLog *logptr;
extern char *mmtailor;         /* where the tailoring file is          */
extern char *logdfldir;

extern int mid_enable;
extern Table tb_mailids;
extern Table tb_users;
extern Table tb_mc;
extern Table **tb_list;
extern int tb_numtables;
extern int tb_maxtables;

extern char *strdup();

static	char	version[] = "$@(#)MMDFII, Release B";

#define MAXARG 100

mmdf_init (pgmname)            /* initialize an mmdf process           */
char *pgmname;
{
    extern char *dupfpath ();
    char *argv[MAXARG];
    int  argc;
    char *savelog;      /* SEK hold log file name                       */
#ifdef DEBUG
    int dbind;
#endif

    pgmname = (pgmname ? pgmname : "???");	/* Paranoid */
    ll_log (logptr, LLOGPTR, "mmdf_init (%s)", pgmname);

    if (!isstr (mmtailor))      /* tailor info is compiled in           */
	return;

    mmdf_fdinit();

    /* SEK save initial part of log name    */
    savelog = strdup (logptr  -> ll_file);

    ll_hdinit (logptr, pgmname);  /* make header unique                 */

    if (logdfldir != (char *) 0)
	logptr -> ll_file = dupfpath (logptr -> ll_file, logdfldir);

    if (tai_init (mmtailor) != OK)
	err_abrt (RP_FIO, "Can't access tailoring file '%s'", mmtailor);

    while ((argc = tai_get (MAXARG, argv)) > 0)
    {                           /* test for mmdf or dial info   */
#ifdef DEBUG
	ll_log (logptr, LLOGBTR, "argc = %d", argc);
	for (dbind = 0; dbind < argc; dbind++)
	    ll_log (logptr, LLOGFTR, "(%d) '%s'", dbind, argv[dbind]);
#endif
	if (argv[0] == 0 || argv[0][0] == '\0')
		continue;               /* noop  */

	switch (uip_tai (argc, argv))
	{                       /* user program tailoring info? */
	    case YES:
	    case NOTOK:
		continue;
	}

	switch (mm_tai (argc, argv))
	{                       /* mmdf tailoring info? */
	    case YES:
	    case NOTOK:
		continue;
	}

#ifndef NODIAL
	switch (d_tai (argc, argv))
	{                       /* dialing package info? */
	    case YES:
	    case NOTOK:
		continue;
	}
#endif NODIAL

	post_tai (argc, argv);
    }

    if (argc == NOTOK)
	err_abrt (RP_MECH, "Error processing tailoring file '%s'", mmtailor);

    tai_end ();

    if (mid_enable) {
	if ((tb_numtables+1) >= tb_maxtables)
	    err_abrt (RP_MECH, "No space in tb_list for tb_users/mailids");
	tb_list[tb_numtables++] = &tb_users;
	tb_list[tb_numtables++] = &tb_mailids;
	tb_list[tb_numtables] = (Table *) 0;
    }

    if (logdfldir != (char *) 0)
    {                           /* we know of malloc in code, above */
	free (logptr -> ll_file);
	logptr -> ll_file = dupfpath (savelog, logdfldir);
	free (savelog);
    }
    return;
}


tai_error (error, errp, argc, argv)
char *error, *errp;
int argc;
char **argv;
{
	char    errline[LINESIZE];

	argv[argc] = (char *) 0;
	arg2vstr (0, sizeof errline, errline, argv);
	ll_log (logptr, LLOGFAT, "%s (%s) in '%s'", error, errp, errline);
}
