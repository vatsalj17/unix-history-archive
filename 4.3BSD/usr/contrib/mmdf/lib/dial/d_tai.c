#include "util.h"
#include "ll_log.h"
#include  "d_proto.h"
#include "d_structs.h"
#include "cmd.h"

/*      tailor the dial package   */
/*
 * Feb 82       Dan Long        Added type field in dports tailor
 * Mar 82	Dan Long	Fixed access setting for d_lines and
 *				added NULL'ing of next item in structs
 *
 */
extern char *def_trn;
extern struct ll_struct ph_log;
#ifdef lint
char *tai_eptr;		/* keep lint happy -- defined in ../mmdf/mm_tai.c */
#else
extern char *tai_eptr;
#endif
extern int errno;
extern struct dialports  *d_prts;
extern int d_numprts;
extern int d_maxprts;
extern struct directlines  *d_lines;
extern int d_numlines;
extern int d_maxlines;
extern char  *d_calllog;
extern unsigned short d_lrill[];
extern unsigned short d_lxill[];
extern struct speedtab d_spdtab[];

#define CMDDPORT    1
#define CMDDLINE    2
#define CMDDACCT    3
#define CMDDBADIN   4
#define CMDDBADOUT  5
#define CMDDBAD     6
#define	CMDPHLOG    7
#define	DEFTRAN     8

LOCVAR Cmd cmddial[] =
{
    "d_port",   CMDDPORT,   4,
    "dport",    CMDDPORT,   4,
    "dprt",     CMDDPORT,   4,
    "port",     CMDDPORT,   4,
    "d_line",   CMDDLINE,   4,
    "dline",    CMDDLINE,   4,
    "dlin",     CMDDLINE,   4,
    "line",     CMDDLINE,   4,
    "daccount", CMDDACCT,   1,
    "dacct",    CMDDACCT,   1,
    "acct",     CMDDACCT,   1,
    "dbadin",	CMDDBADIN,  8,
    "dbadout",	CMDDBADOUT, 8,
    "dbad",	CMDDBAD,    8,
    "phlog",	CMDPHLOG,   1,
    "deftran",  DEFTRAN,    1,
    0,          0,          0
};

#define CMDPACCESS  1
#define CMDPPREF    2
#define CMDPPOST    3

LOCVAR Cmd cmddparm[] =
{
    "pref",     CMDPPREF,   1,
    "suff",     CMDPPOST,   1,
    "access",   CMDPACCESS, 1,
#ifdef NVRCOMPIL
    "post",     CMDPPOST,   1,
#endif
    0,          0,          0
};


d_tai (argc, argv)              /* tailor some dial parameter */
    int argc;                   /* number of values     */
    char *argv[];               /* list of values       */
{
    register int ind;
    int tspeed;

    switch (cmdsrch (argv[0], argc - 1, cmddial))
    {
	case CMDPHLOG:
	    return (tai_log (argc, &argv[1], &ph_log));

	case DEFTRAN:
	    def_trn = argv[1];
	    return(YES);

	case CMDDPORT:
	    d_prts[d_numprts].p_port = argv[1];
	    d_prts[d_numprts].p_lock = argv[2];
	    d_prts[d_numprts].p_acu  = argv[3];
	    sscanf (argv[4], "%o", &(d_prts[d_numprts].p_speed));
	    d_prts[d_numprts].p_ltype = argv[5];

	    for (ind = 6; ind < argc; ind++)
		if (!lexequ (argv[ind], "="))
		{
		    errno = EINVAL;
		    tai_eptr = argv[ind];
		    argv[ind + 1];
		    return (NOTOK);
		}
		else
		{
		    ind += 2;
		    switch (cmdsrch (argv[ind - 1], argc - ind, cmddparm))
		    {
			case CMDPPREF:
			    d_prts[d_numprts].p_acupref = argv[ind];
			    break;

			case CMDPPOST:
			    d_prts[d_numprts].p_acupost = argv[ind];
			    break;

			case CMDPACCESS:
			    d_prts[d_numprts].p_access = argv[ind];
			    break;

			default:
			    errno = EINVAL;
			    tai_eptr = argv[ind - 1];
			    argv[ind + 1];
			    return (NOTOK);
		    }
	    }
	    if (++d_numprts > d_maxprts)
	    {
		d_prts[--d_numprts].p_port = NULL;
		errno = ENFILE;
		tai_eptr = argv[0];
		return (NOTOK);
	    }
            d_prts[d_numprts].p_port = NULL;
	    return (YES);

	case CMDDLINE:
	    d_lines[d_numlines].l_name = argv[1];
	    d_lines[d_numlines].l_tty  = argv[2];
	    d_lines[d_numlines].l_lock = argv[3];

	    sscanf (argv[4], "%d", &tspeed);
	    for (ind=0; d_spdtab[ind].s_speed != 0; ind++) {
		if (tspeed == d_spdtab[ind].s_index ||
		    strcmp (argv[4], d_spdtab[ind].s_speed) == 0)
		    break;
	    }
	    if (d_spdtab[ind].s_speed == 0) {
		    errno = EINVAL;
		    tai_eptr = argv[4];
		    return(NOTOK);
	    }
	    d_lines[d_numlines].l_speed = d_spdtab[ind].s_index;


	    for (ind = 5; ind < argc; ind++)
	    {
		if (lexequ (argv[ind], "="))
		{
		    if ((ind += 2) >= argc)
		    {
			    errno = EINVAL;
			    tai_eptr = argv[ind - 1];
			    argv[ind + 1];
			return (NOTOK);
		    }
		    if (lexequ (argv[ind - 1], "access"))
			d_lines[d_numlines].l_access = argv[ind];
		    else
		    {
			    errno = EINVAL;
			    tai_eptr = argv[ind - 1];
			    argv[ind + 1];
			    return (NOTOK);
		    }
		}
		else
		{
			    errno = EINVAL;
			    tai_eptr = argv[ind];
			    argv[ind + 1];
		    return (NOTOK);
		}
	    }
	    if (++d_numlines > d_maxlines)
	    {
		d_lines[--d_numlines].l_tty = NULL;
		errno = ENFILE;
		tai_eptr = argv[0];
		return (NOTOK);
	    }
	    d_lines[d_numlines].l_tty = NULL;
	    return (YES);

	case CMDDACCT:
	    d_calllog = argv[1];
	    return (YES);

	case CMDDBADOUT:          /* can't send bare */
	    return (d_vec (argc, &argv[1], d_lxill));

	case CMDDBADIN:           /* can't receive bare */
	    return (d_vec (argc, &argv[1], d_lrill));

	case CMDDBAD:             /* send/receive list the same */
	    if (d_vec (argc, &argv[1], d_lxill) == YES &&
	        d_vec (argc, &argv[1], d_lrill) == YES)
		    return (YES);

    }
    return (NO);                /* not a dial parameter         */
}
/**/

d_vec (argc, argv, vec)
    int argc;
    char *argv[];
    unsigned short vec[];
{
    register int ind;
    int tmp;

    if (argc > 8)
	argc = 8;               /* only 128 bits @ 16 bits/word */
    for (ind = 0; ind < argc; ind++) {
	sscanf (argv[ind], "%o", &tmp);
	/* convert to short */
	vec[ind] = (unsigned) tmp;
    }

    return (YES);
}
