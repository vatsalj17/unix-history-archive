#include "util.h"
#include "mmdf.h"
#include "cmd.h"
#include "ch.h"
#include "ap.h"

extern char *malloc ();

extern LLog *logptr, chanlog;

extern char
	    *locmachine,        /* UCL local machine name       */
	    *locname,
	    *locdomain,
	    *sitesignature,
	    *mmdflogin,
	    *supportaddr;

extern char
	    *logdfldir,
	    *phsdfldir,
	    *tbldfldir,
	    *tbldbm,
	    *quedfldir,
	    *cmddfldir,
	    *chndfldir,
	    *lckdfldir,
	    *mldfldir;

extern int
	    queprot,
	    warntime,
	    failtime,
	    maxhops,
	    maxqueue,
	    mgt_addid,
	    lnk_listsize,
	    mid_enable,
	    mailsleep,
	    sentprotect;

extern LLog
	    msglog,
	    chanlog,
	    authlog;

extern char *def_trn;

extern char
	    *tquedir,
	    *aquedir,
	    *mquedir;

extern char
	    *namsubmit,
	    *pathsubmit,
	    *namdeliver,
	    *pathdeliver,
	    *nampkup,
	    *pathpkup,
	    *nammail,
	    *pathmail;

extern char
	    *ch_dflnam,
	    *dlvfile,
	    *mldflfil,
	    *delim1,
	    *delim2;

extern char                     /* NIFTP channel bits                   */
		*pn_quedir;

extern char
		*authrequest,   /* Authorisation request		*/
		*Uuxstr;	/* uux command string			*/
/**/

#define MMLOCHOST        1
#define MMSIGN           2
#define MMLOGIN          3
#define MMSUPPORT        4
#define MMLOGDIR         5
#define MMPHSDIR         6
#define MMTBLDIR         7
#define MMDBM            8
#define MMQUEDIR         9
#define MMCMDDIR        10
#define MMCHANDIR       11
#define MMDLVRDIR       12
#define MMTEMPT         13
#define MMADDRQ         14
#define MMMSGQ          15
#define MMQUEPROT       16
#define MMMSGLOG        17
#define MMCHANLOG       18
#undef	Gonzo           19
#undef	Gonzo           20
#define MMWARNTIME      21
#define MMFAILTIME      22
#define MMMAXSORT       23
#define MMSLEEP         24
#define MMSUBMIT        25
#define MMDELIVER       26
#define MMPICKUP        27
#define MMV6MAIL        28
#define MMMBXPROT       29
#define MMMBXNAME       30
#define MMMBXPREF       31
#define MMMBXSUFF       32
#define MMDLVFILE       33
#define	MMAXHOPS	34
#define	MADDID		35
#define MLISTSIZE	36
#define MMDFLCHAN       37
#define UUxstr          38
#define MMTBL           39
#define MMLOCDOMAIN     40
#define MMDOMAIN        41
#define	ALIAS           42
#define MMLOCMACHINE    43
#define NIQUEDIR        44
#define MMMAILIDS       45
#define MMLCKDIR        46
#define AUTHLOG         47
#define AUTHREQUEST	48
#define MMCHAN          49
#define MMNOOP         100

/**/

Cmd cmdtab[] =
{
    "mlocmachine", MMLOCMACHINE, 1,
    "mlname",      MMLOCHOST,  1,
    "mlochost",    MMLOCHOST,  1,
    "mldomain",    MMLOCDOMAIN, 1,
    "msig",        MMSIGN,     1,
    "mlogin",      MMLOGIN,    1,
    "msupport",    MMSUPPORT,  1,
    "mlogdir",     MMLOGDIR,   1,
    "mphsdir",     MMPHSDIR,   1,
    "mtbldir",     MMTBLDIR,   1,
    "mlckdir",     MMLCKDIR,   1,
    "mdbm",        MMDBM,      1,
    "mdflchan",    MMDFLCHAN,  1,
    "mquedir",     MMQUEDIR,   1,
    "mcmddir",     MMCMDDIR,   1,
    "mchndir",     MMCHANDIR,  1,
    "mdlvrdir",    MMDLVRDIR,  1,
    "mtempt",      MMTEMPT,    1,
    "mtmpt",       MMTEMPT,    1,
    "maddrq",      MMADDRQ,    1,
    "mmsgq",       MMMSGQ,     1,
    "mqueprot",    MMQUEPROT,  1,
    "mmsglog",     MMMSGLOG,   1,
    "mchanlog",    MMCHANLOG,  1,
    "authlog",     AUTHLOG,    1,
    "mwarntime",   MMWARNTIME, 1,
    "mfailtime",   MMFAILTIME, 1,
    "mmaxsort",    MMMAXSORT,  1,
    "msleep",      MMSLEEP,    1,
    "msubmit",     MMSUBMIT,   1,
    "mdeliver",    MMDELIVER,  1,
    "mpkup",       MMPICKUP,   1,
    "mv6mail",     MMV6MAIL,   1,
    "mmbxprot",    MMMBXPROT,  1,
    "mmbxname",    MMMBXNAME,  1,
    "mmbxpref",    MMMBXPREF,  1,
    "mmbxsuff",    MMMBXSUFF,  1,
    "mdlv",        MMDLVFILE,  1,
    "maddid",	   MADDID,     1,
    "mmaxhops",    MMAXHOPS,   1,
    "mlistsize",   MLISTSIZE,  1,
    "uuxstr",      UUxstr,     1,
    "mtbl",        MMTBL,      1,
    "mchn",        MMCHAN,     1,
    "mmailid",     MMMAILIDS,  1,
    "mdmn",        MMDOMAIN,   1,
    "niquedir",    NIQUEDIR,   1,
    "authrequest", AUTHREQUEST,1,
    "alias",	   ALIAS,      1,
    "",            MMNOOP,     0,
    0,             0,          0
};
/**/

mm_tai (argc, argv)     /* process mmdf tailor info     */
    int argc;
    char *argv[];
{
    int retval;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "m_tai (%s: %d)", argv[0], argc);
#endif

    retval = YES;

    switch (cmdsrch (argv[0], --argc, cmdtab))
    {
	case NO:
	case MMNOOP:
	default:        /* command not known to us      */
	    retval = NO;
	    break;

	case MMLOCMACHINE:
	    locmachine = argv[1];
	    break;

	case MMLOCHOST:
	    locname = argv[1];
	    break;

	case MMLOCDOMAIN:
	    locdomain = argv[1];
	    break;

	case MMSIGN:
	    sitesignature = argv[1];
	    break;

	case MMLOGIN:
	    mmdflogin = argv[1];
	    break;

	case MMSUPPORT:
	    supportaddr = argv[1];
	    break;

	case MMLOGDIR:
	    logdfldir = argv[1];
	    break;

	case MMPHSDIR:
	    phsdfldir = argv[1];
	    break;

	case MMTBLDIR:
	    tbldfldir = argv[1];
	    break;

	case MMDBM:
	    tbldbm = argv[1];
	    break;

	case MMQUEDIR:
	    quedfldir = argv[1];
	    break;

	case MMCMDDIR:
	    cmddfldir = argv[1];
	    break;

	case MMCHANDIR:
	    chndfldir = argv[1];
	    break;

	case MMDLVRDIR:
	    mldfldir = argv[1];
	    break;

	case MMLCKDIR:
	    lckdfldir = argv[1];
	    break;

	case MMTEMPT:
	    tquedir = argv[1];
	    break;

	case MMADDRQ:
	    aquedir = argv[1];
	    break;

	case MMMSGQ:
	    mquedir = argv[1];
	    break;

	case MMQUEPROT:
	    sscanf (argv[1], "%o", &queprot);
	    break;

	case MMMSGLOG:
	    retval = tai_log (argc, &argv[1], &msglog);
	    break;

	case MMCHANLOG:
	    retval = tai_log (argc, &argv[1], &chanlog);
	    break;

	case AUTHLOG:
	    retval = tai_log (argc, &argv[1], &authlog);
	    break;

	case AUTHREQUEST:
	    authrequest = argv[1];
	    break;

	case MMWARNTIME:
	    warntime = atoi (argv[1]);
	    break;

	case MMFAILTIME:
	    failtime = atoi (argv[1]);
	    break;

	case MMMAXSORT:
	    maxqueue = atoi (argv[1]);
	    break;

	case MMSLEEP:
	    mailsleep = atoi (argv[1]);
	    break;

	case MMSUBMIT:
	    retval = tai_pgm (argc, &argv[1], &namsubmit, &pathsubmit);
	    break;

	case MMDELIVER:
	    retval = tai_pgm (argc, &argv[1], &namdeliver, &pathdeliver);
	    break;

	case MMPICKUP:
	    retval = tai_pgm (argc, &argv[1], &nampkup, &pathpkup);
	    break;

	case MMV6MAIL:
	    retval = tai_pgm (argc, &argv[1], &nammail, &pathmail);
	    break;

	case MMMBXPROT:
	    sscanf (argv[1], "%o", &sentprotect);
	    break;

	case MMMBXNAME:
	    mldflfil = argv[1];
	    break;

	case MMMBXPREF:
	    delim1 = argv[1];
	    break;

	case MMMBXSUFF:
	    delim2 = argv[1];
	    break;

	case MMDLVFILE:
	    dlvfile = argv[1];
	    break;

    	case MMAXHOPS:
    	    maxhops = atoi(argv[1]);
    	    break;

    	case MADDID:
    	    mgt_addid = atoi(argv[1]);
    	    break;

    	case MLISTSIZE:
	    lnk_listsize = atoi(argv[1]);
    	    break;

	case UUxstr:
	    Uuxstr = argv[1];
	    break;

	case MMTBL:
	    retval = tb_tai (argc, &argv[1]);
	    break;

	case MMDFLCHAN:
	    ch_dflnam = argv[1];
	    break;

	case MMCHAN:
	    retval = ch_tai (argc, &argv[1]);
	    break;

	case MMMAILIDS:         /* enable/disable use of Mail-Ids */
	    mid_enable = atoi(argv[1]);
	    break;

	case MMDOMAIN:
	    retval = dm_tai (argc, &argv[1]);
	    break;

	case NIQUEDIR:
	    pn_quedir = argv[1];
	    break;

	case ALIAS:
    	    al_tai (argc, &argv[1]);
    	    break;
    }

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "m_tai (ret=%d)", retval);
#endif

    return (retval);
}
/**/

extern Table    **tb_list;
extern int  tb_maxtables;
extern int  tb_numtables;

#define CMDTBASE    1
#define CMDTNAME    2
#define CMDTFILE    3
#define CMDTSHOW    4
#define CMDTFLAGS   5
#define CMDTNOOP    6

LOCVAR Cmd
	    cmdtbl[] =
{
    "base",     CMDTBASE,   1,
    "name",     CMDTNAME,   1,
    "file",     CMDTFILE,   1,
    "show",     CMDTSHOW,   1,
    "flags",	CMDTFLAGS,  1,
    "",         CMDTNOOP,   0,
    0,          0,          0
};

#define CMDTFFILE	1
#define CMDTFDBM	2
#define CMDTFNS		3
#define CMDTFDOMAIN     4
#define CMDTFCHANNEL    5
#define CMDTFROOT       6
#define CMDTFPARTIAL    7

LOCVAR Cmd
	tbflags [] =
{
    "file",	CMDTFFILE,	0,
    "dbm",	CMDTFDBM,	0,
    "ns",	CMDTFNS,	0,
    "domain",   CMDTFDOMAIN,    0,
    "channel",  CMDTFCHANNEL,   0,
    "root",     CMDTFROOT,      0,
    "partial",  CMDTFPARTIAL,   0,
    0,          0,              0
};

tb_tai (argc, argv)
    int argc;
    char *argv[];
{
    int ind;
    register Table *tbptr;

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "tb_tai (%s: %d)", argv[0], argc);
#endif

    if (tb_numtables >= tb_maxtables)
    {
	tai_error ("exceed table size", argv[0], argc, argv);
	return (NOTOK);
    }
#ifdef DEBUG
    if (logptr -> ll_level == LLOGFTR)
    {
        ll_log (logptr, LLOGFTR, "old tables (%d)", tb_numtables);
        for (ind = 0; tb_list[ind] != (Table *) 0; ind++)
    	    ll_log (logptr, LLOGFTR, "tb(%d) '%s'", 
		ind, tb_list[ind] -> tb_name);
    }
#endif

    /*NOSTRICT*/
    tb_list[tb_numtables++] = tbptr =
			(Table *) malloc ((unsigned) (sizeof (Table)));
    tb_list[tb_numtables] = (Table *) 0;
    tbptr -> tb_name =  "notablename";
    tbptr -> tb_show =  (char *) 0;
    tbptr -> tb_file =  (char *) 0;
    tbptr -> tb_fp   =  (FILE *) NULL;
    tbptr -> tb_pos  =  0L;
    tbptr -> tb_flags = TB_FILE;

    for (ind = 0; ind < argc; ind++)
    {
	if (!lexequ (argv[ind], "="))
	{
	    if (ind == 0)
	    {                   /* ok to default first field only */
#ifdef DEBUG
		ll_log (logptr, LLOGBTR, "defaulting base '%s'", argv[ind]);
#endif
		goto dobase;
	    }
	    else
	    {
#ifdef DEBUG
		tai_error ("unassignable, bare data field",
				argv[ind], argc, argv);
#endif
		continue;
	    }
	}
	else
	{
	    ind += 2;
#ifdef DEBUG
	    ll_log (logptr, LLOGFTR, "tb '%s'(%d)", argv[ind - 1], argc - ind);
#endif
	    switch (cmdsrch (argv[ind - 1], argc - ind, cmdtbl))
	    {
		case CMDTBASE:
	dobase:
		    tbptr -> tb_name =  argv[ind];
		    tbptr -> tb_show =  argv[ind];
		    tbptr -> tb_file =  argv[ind];
		    break;

		case CMDTNAME:
		    tbptr -> tb_name =  argv[ind];
		    break;

		case CMDTFILE:
		    tbptr -> tb_file =  argv[ind];
		    break;

		case CMDTSHOW:
		    tbptr -> tb_show=  argv[ind];
		    break;

	    	case CMDTFLAGS:
#ifdef DEBUG
		    ll_log (logptr, LLOGFTR, "table flag '%s'", argv[ind]);
#endif
		    switch (cmdsrch (argv[ind], 0, tbflags))
		    {
			case CMDTFFILE:
			    tbptr -> tb_flags |= TB_FILE;
			    break;

			case CMDTFDBM:
			    tbptr -> tb_flags |= TB_DBM;
			    break;

			case CMDTFNS:
			    tbptr -> tb_flags |= TB_NS;
			    break;

			case CMDTFDOMAIN:
			    tbptr -> tb_flags |= TB_DOMAIN;
			    break;

			case CMDTFCHANNEL:
			    tbptr -> tb_flags |= TB_CHANNEL;
			    break;

			case CMDTFROOT:
			    tbptr -> tb_flags |= TB_ROOT;
			    break;

			case CMDTFPARTIAL:
			    tbptr -> tb_flags |= TB_PARTIAL;
			    break;

			default:
#ifdef DEBUG
			    tai_error ("unknown table flag", argv[ind-1],
					argc, argv);
#endif
			    continue;
		    }
	    	    break;

		case CMDTNOOP:
		    break;      /* noop */

		default:
#ifdef DEBUG
		    tai_error ("unknown table parm", argv[ind], argc, argv);
#endif
		    break;
	    }
	}
    }

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "tb_tai (ret=YES)");
    ll_log (logptr, LLOGFTR, "name=%s, ref=%s, file=%s, flags=%0o",
		tbptr -> tb_name, tbptr -> tb_show, tbptr -> tb_file,
		tbptr -> tb_flags);
#endif

    return (YES);
}
/**/

#include "dm.h"

extern Domain   **dm_list;
extern int  dm_maxtables;
extern int  dm_numtables;

#define CMDDBASE    1
#define CMDDNAME    2
#define CMDDDMN     3
#define CMDDSHOW    4
#define CMDDTABLE   5
#define CMDDNOOP    7
#define CMDDLNAME   8

LOCVAR Cmd
	    dmntbl[] =
{
    "base",     CMDDBASE,   1,
    "name",     CMDDNAME,   1,
    "dmn",      CMDDDMN,    1,
    "show",     CMDDSHOW,   1,
    "table",    CMDDTABLE,  1,
    "lname",    CMDDLNAME,  1,
    "",         CMDDNOOP,   0,
    0,          0,          0
};

dm_tai (argc, argv)
    int argc;
    char *argv[];
{
    int ind;
    int tbind,
	showind;
    char *tbname;
    register Domain *dmnptr;

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "dm_tai (%s: %d)", argv[0], argc);
#endif

    if (dm_numtables >= dm_maxtables)
    {
	tai_error ("exceed domain table size", argv[0], argc, argv);
	return (NOTOK);
    }
#ifdef DEBUG
    if (logptr -> ll_level == LLOGFTR)
    {
        ll_log (logptr, LLOGFTR, "old domains (%d)", dm_numtables);
        for (ind = 0; dm_list[ind] != (Domain *) 0; ind++)
    	    ll_log (logptr, LLOGFTR, "dm(%d) '%s'", 
		ind, dm_list[ind] -> dm_name);
    }
#endif

    /*NOSTRICT*/
    dm_list[dm_numtables++] = dmnptr =
			(Domain *) malloc ((unsigned) (sizeof (Domain)));
    dm_list[dm_numtables] = (Domain *) 0;
    dmnptr -> dm_name     = "nodomainname";
    dmnptr -> dm_domain   =  (char *) 0;
    dmnptr -> dm_lname    =  (char *) 0;
    dmnptr -> dm_show     =  (char *) 0;
    dmnptr -> dm_table    =  (Table *) 0;
    tbind = -1;
    showind = -1;

    for (ind = 0; ind < argc; ind++)
    {
	if (!lexequ (argv[ind], "="))
	{
	    if (ind == 0)
	    {                   /* ok to default first field only */
#ifdef DEBUG
		ll_log (logptr, LLOGBTR, "defaulting base '%s'", argv[ind]);
#endif
		goto dobase;
	    }
	    else
	    {
#ifdef DEBUG
		tai_error ("unassignable, bare data field", argv[ind], argc, argv);
#endif
		continue;
	    }
	}
	else
	{
	    ind += 2;
#ifdef DEBUG
	    ll_log (logptr, LLOGFTR, "dm '%s'(%d)", argv[ind - 1], argc - ind);
#endif
	    switch (cmdsrch (argv[ind - 1], argc - ind, dmntbl))
	    {
		case CMDDBASE:
	dobase:
		    dmnptr -> dm_name   = argv[ind];
		    dmnptr -> dm_domain = argv[ind];
		    showind = ind;
		    break;

		case CMDDNAME:
		    dmnptr -> dm_name =  argv[ind];
		    break;

		case CMDDDMN:
		    dmnptr -> dm_domain =  argv[ind];
		    break;

		case CMDDSHOW:
		    showind = ind;
		    break;

		case CMDDTABLE:
		    tbind = ind;
		    break;

		case CMDDLNAME:
		    dmnptr -> dm_lname = argv[ind];
		    break;

		case CMDDNOOP:
		    break;      /* noop */

		default:
		    argv[ind + 1] = (char *) 0; /* eliminate rest of args */
#ifdef DEBUG
		    tai_error ("unknown domain parm", argv[ind-1], argc, argv);
#endif
		    break;
	    }
	}
    }
    tbname = (tbind >= 0) ? argv[tbind] : dmnptr -> dm_name;
    if ((dmnptr -> dm_table = tb_nm2struct (tbname)) == (Table *) NOTOK)
    {                   /* table does not already exist */
#ifdef DEBUG
	ll_log (logptr, LLOGBTR, "'%s' unknown domain table", tbname);
#endif
	if (tb_numtables >= tb_maxtables)
	{
#ifdef DEBUG
	    tai_error ("exceed tb_list table size", tbname, argc, argv);
#endif
	}
	else
	{
	    /*NOSTRICT*/
	    tb_list[tb_numtables++] = dmnptr -> dm_table =
		(Table *) malloc ((unsigned) (sizeof (Table)));
	    tb_list[tb_numtables] = (Table *) 0;
#ifdef DEBUG
	    ll_log (logptr, LLOGBTR, "allocating dm_table addr (%o)", dmnptr -> dm_table);
#endif
	    dmnptr -> dm_table -> tb_name =  tbname;
	    dmnptr -> dm_table -> tb_file =  tbname;
	    dmnptr -> dm_table -> tb_fp   =  (FILE *) NULL;
	    dmnptr -> dm_table -> tb_pos  =  0L;
	    dmnptr -> dm_table -> tb_show =
		(showind >= 0 ? argv[showind] : tbname);
	}
    }
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "dm_table addr (%o)", dmnptr -> dm_table);
#endif

    dmnptr -> dm_show = (showind >= 0 ? argv[showind] : dmnptr -> dm_name);
    if (dmnptr -> dm_lname == (char *) 0)
	dmnptr -> dm_lname = locname;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "dm_tai (ret=YES)");
    ll_log (logptr, LLOGFTR, "domain '%s'('%s'), table='%s'(%s)",
		dmnptr -> dm_show, dmnptr -> dm_name,
		dmnptr -> dm_table -> tb_show, dmnptr -> dm_table -> tb_name);
    ll_log (logptr, LLOGBTR, "dm_table addr (%o)", dmnptr -> dm_table);
#endif

    return (YES);
}
/**/
extern Alias   *al_list;

al_tai (argc, argv)
    int argc;
    char *argv[];
{
    int ind;
    register Alias *alptr, *palptr;

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "al_tai (%s: %d)", argv[0], argc);
#endif

    /*NOSTRICT*/
    alptr = (Alias *) malloc ((unsigned) (sizeof (Alias)));
    alptr -> al_flags = 0;
    alptr -> al_next = NULL;

    for (ind = 0; ind < argc; ind++)
    {
	if (!lexequ (argv[ind], "="))
	{
	    if (lexequ (argv[ind], "trusted"))
		alptr->al_flags |= AL_TRUSTED;
	    else if (lexequ (argv[ind], "nobypass"))
		alptr->al_flags |= AL_NOBYPASS;
	    else {
#ifdef DEBUG
		tai_error ("unassignable, alias field", argv[ind], argc, argv);
#endif
		continue;
	    }
	} else {
	    ind += 2;
#ifdef DEBUG
	    ll_log (logptr, LLOGFTR, "al '%s'(%d)", argv[ind - 1], argc - ind);
#endif
	    if (lexequ(argv[ind-1], "table")) {
		if ((alptr->al_table = tb_nm2struct(argv[ind]))
		   == (Table *)NOTOK)
		{
#ifdef DEBUG
		    ll_log (logptr, LLOGBTR, "'%s' unknown alias table", argv[ind]);
#endif
		    if (tb_numtables >= tb_maxtables) {
#ifdef DEBUG
			tai_error ("exceed tb_list table size", argv[ind], argc, argv);
#endif
		    } else {
			/*NOSTRICT*/
			tb_list[tb_numtables++] = alptr->al_table =
			    (Table *) malloc ((unsigned) (sizeof (Table)));
			tb_list[tb_numtables] = (Table *) 0;
			alptr -> al_table -> tb_name =  argv[ind];
			alptr -> al_table -> tb_file =  argv[ind];
			alptr -> al_table -> tb_fp   =  (FILE *) NULL;
			alptr -> al_table -> tb_pos  =  0L;
		    	alptr -> al_table -> tb_show =  argv[ind];
		    }
		}
	    } else {
		argv[ind + 1] = (char *) 0; /* eliminate rest of args */
#ifdef DEBUG
		tai_error ("unknown alias parm", argv[ind-1], argc, argv);
#endif
		break;
	    }
	}
    }
    if (al_list == (Alias *)0)
	al_list = alptr;
    else {
	for (palptr = al_list; palptr->al_next != NULL; palptr = palptr->al_next);
	palptr->al_next = alptr;
    }
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "al_tai (ret=YES)");
#endif
    return (YES);
}
/**/

extern Chan **ch_tbsrch;
extern int  ch_maxchans;
extern int  ch_numchans;

#define CMDCBASE    1
#define CMDCNAME    2
#define CMDCSHOW    3
#undef	Gonzo       4    /* OBSOLETE */
#define CMDCQUE     5
#define CMDCTBL     6
#define CMDCPGM     7
#define CMDCHOST    8
#define CMDCUSER    9
#define CMDCSCRIPT 10
#define CMDCPOLL   11
#define CMDCACCESS 12
#define CMDCNOOP   13
#define CMDCLNAME  14
#define CMDCLDOMN  15
#define CMDCLISRC  16
#define CMDCLOSRC  17
#define CMDCLIDEST 18
#define CMDCLODEST 19
#define CMDCKNOWN  20
#define CMDCCONFSTR 21
#define CMDCAP     22
#define CMDCAUTH   23
#define CMDCTRANS  24
#define CMDCTTL    25
#define CMDCLLOG   26
#define CMDCLLEV   27

LOCVAR Cmd
	    cmdchan[] =
{
    "base",     CMDCBASE,   1,
    "name",     CMDCNAME,   1,
    "show",     CMDCSHOW,   1,
    "que",      CMDCQUE,    1,
    "tbl",      CMDCTBL,    1,
    "pgm",      CMDCPGM,    1,
    "host",     CMDCHOST,   1,
    "user",     CMDCUSER,   1,
    "scr",      CMDCSCRIPT, 1,
    "trn",      CMDCTRANS,  1,
    "poll",     CMDCPOLL,   1,
    "mod",      CMDCACCESS, 1,
    "lname",    CMDCLNAME,  1,
    "ldomain",  CMDCLDOMN,  1,
    "insrc",    CMDCLISRC,  1,
    "outsrc",   CMDCLOSRC,  1,
    "indest",   CMDCLIDEST, 1,
    "outdest",  CMDCLODEST, 1,
    "known",    CMDCKNOWN,  1,
    "confstr",  CMDCCONFSTR,1,
    "ap",       CMDCAP,     1,
    "auth",     CMDCAUTH,   1,
    "ttl",      CMDCTTL,    1,
    "log",      CMDCLLOG,   1,
    "level",    CMDCLLEV,   1,
    "",         CMDCNOOP,   0,
    0,          0,          0
};

#define CMDPREG     1
#define CMDPBAK     2
#define CMDPPSV     3
#define CMDPDID     4
#define CMDPIMM     5
#define CMDPPICK    6
#define CMDPSEND    7
#define CMDPNOOP    8

LOCVAR Cmd
	    parmchan[] =
{
    "reg",      CMDPREG,    0,
    "bak",      CMDPBAK,    0,
    "psv",      CMDPPSV,    0,
    "imm",      CMDPIMM,    0,
    "pick",     CMDPPICK,   0,
    "send",     CMDPSEND,   0,
    "",         CMDPNOOP,   0,
    0,          0,          0
};


#define CMDASAME        1
#define CMDA733         2
#define CMDA822         3
#define CMDABIG         4
#define CMDANODOTS      5
#define CMDAJNT         6



LOCVAR Cmd
	parmap [] =
{
    "same",     CMDASAME,       0,
    "733",      CMDA733,        0,
    "822",      CMDA822,        0,
    "big",      CMDABIG,        0,
    "nodots",   CMDANODOTS,     0,
    "jnt",      CMDAJNT,        0,
    0,          0,              0
};

#define CMDTFREE        0
#define CMDTINLOG       1
#define CMDTINWARN      2
#define CMDTINBLOCK     3
#define CMDTOUTLOG      4
#define CMDTOUTWARN     5
#define CMDTOUTBLOCK    6
#define CMDTHAU         7
#define CMDTDHO         8

LOCVAR  Cmd
	authmap [] =
{
    "free",     CMDTFREE,       0,
    "inlog",    CMDTINLOG,      0,
    "inwarn",   CMDTINWARN,     0,
    "inblock",  CMDTINBLOCK,    0,
    "outlog",   CMDTOUTLOG,     0,
    "outwarn",  CMDTOUTWARN,    0,
    "outblock", CMDTOUTBLOCK,   0,
    "hau",      CMDTHAU,        0,
    "dho",      CMDTDHO,        0,
    0,          0,              0
};

ch_tai (argc, argv)
    int argc;
    char *argv[];
{
    int ind;
    int tbind,
	showind;
    char *tbname;
    register Chan *chptr;

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "ch_tai (%s: %d)", argv[0], argc);
#endif

    if (ch_numchans >= ch_maxchans)
    {
#ifdef DEBUG
	tai_error ("exceed ch table size", argv[0], argc, argv);
#endif
	return (NOTOK);
    }
#ifdef DEBUG
    if ( logptr -> ll_level == LLOGFTR ) {
	ll_log (logptr, LLOGFTR, "old channels (%d)", ch_numchans);
	for (ind = 0; ch_tbsrch[ind] != (Chan *) 0; ind++)
	    ll_log (logptr, LLOGFTR, "ch(%d) '%s'",
		ind, ch_tbsrch[ind] -> ch_name);
    }
#endif

    /*NOSTRICT*/
    ch_tbsrch[ch_numchans++] = chptr =
			(Chan *) malloc ((unsigned) (sizeof (Chan)));
    ch_tbsrch[ch_numchans] = (Chan *) 0;

    chptr -> ch_name   =  "nochanname";
    chptr -> ch_show   =  (char *) 0;
    chptr -> ch_table  =  (Table *) 0;
    chptr -> ch_queue  =  (char *) 0;
    chptr -> ch_access =  0;                    /* == DLVRREG */
    chptr -> ch_ppath  =  (char *) 0;
    chptr -> ch_lname  =  locname;
    chptr -> ch_ldomain = locdomain;
    chptr -> ch_host   =  (char *) NORELAY;
    chptr -> ch_login  =  (char *) NOLOGIN;
    chptr -> ch_poltime = (char) 8; /* every two hours */
    chptr -> ch_script =  (char *) 0;
    chptr -> ch_trans  =  (char *) DEFTRANS;
    chptr -> ch_insource = (Table *) 0;
    chptr -> ch_outsource = (Table *) 0;
    chptr -> ch_indest  =  (Table *) 0;
    chptr -> ch_outdest =  (Table *) 0;
    chptr -> ch_confstr = (char *) 0;
    chptr -> ch_apout = AP_SAME;
    chptr -> ch_known = (Table  *) 0;
    chptr -> ch_auth = CH_FREE;
    chptr -> ch_dead = (Cache *) 0;
    chptr -> ch_ttl = (time_t)7200;  /* dead cache TimeToLive, two hours */
    chptr -> ch_logfile = chanlog.ll_file;
    chptr -> ch_loglevel = chanlog.ll_level;
    tbind = -1;
    showind = -1;

    for (ind = 0; ind < argc; ind++)
    {
	if (!lexequ (argv[ind], "="))
	{                       /* not an 'assignment' parameter        */
	    if (ind == 0)
	    {
#ifdef DEBUG
		ll_log (logptr, LLOGBTR, "defaulting base '%s'", argv[ind]);
#endif
		goto dobase;    /* default first field only */
	    }
	    else
	    {
#ifdef DEBUG
		tai_error ("unassignable, bare channel field", argv[ind], argc, argv);
#endif
		continue;
	    }
	}
	else
	{
	    ind += 2;
#ifdef DEBUG
	    ll_log (logptr, LLOGFTR, "ch '%s'(%d)", argv[ind - 1], argc - ind);
#endif
	    switch (cmdsrch (argv[ind - 1], argc - ind, cmdchan))
	    {
		case CMDCBASE:
	dobase:
		    chptr -> ch_name = argv[ind];
		    chptr -> ch_queue = argv[ind];
		    chptr -> ch_ppath = argv[ind];
		    break;

		case CMDCNAME:
		    chptr -> ch_name =  argv[ind];
		    break;

		case CMDCSHOW:
		    showind = ind;
		    break;

		case CMDCQUE:
		    chptr -> ch_queue =  argv[ind];
		    break;

		case CMDCTBL:
		    tbind = ind;
		    break;

		case CMDCPGM:
		    chptr -> ch_ppath =  argv[ind];
		    break;

		case CMDCHOST:
		    chptr -> ch_host =  argv[ind];
		    break;

		case CMDCUSER:
		    chptr -> ch_login =  argv[ind];
		    break;

		case CMDCSCRIPT:
		    chptr -> ch_script =  argv[ind];
		    break;

		case CMDCTRANS:
		    chptr -> ch_trans = argv[ind];
		    break;

		case CMDCPOLL:
		    chptr -> ch_poltime =  atoi (argv[ind]);
		    break;

		case CMDCTTL:
		    /*NOSTRICT*/
		    chptr -> ch_ttl = (time_t) atoi (argv[ind]) * 60L;
		    break;

		case CMDCLLOG:
		    chptr -> ch_logfile = argv[ind];
		    break;

		case CMDCLLEV:
		    if ((chptr -> ch_loglevel = tai_llev (argc, &argv[ind])) < 0)
			chptr -> ch_loglevel = chanlog.ll_level;
		    break;

		case CMDCACCESS:
#ifdef DEBUG
		    ll_log (logptr, LLOGFTR, "ch parm '%s'", argv[ind]);
#endif
		    switch (cmdsrch (argv[ind], 0, parmchan))
		    {
			case CMDPREG:
			    chptr -> ch_access |= DLVRREG;
			    break;

			case CMDPBAK:
			    chptr -> ch_access |= DLVRBAK;
			    break;

			case CMDPPSV:
			    chptr -> ch_access |= DLVRPSV;
			    break;

			case CMDPIMM:
			    chptr -> ch_access |= DLVRIMM;
			    break;

			case CMDPPICK:
			    chptr -> ch_access |= CH_PICK;
			    break;

			case CMDPSEND:
			    chptr -> ch_access |= CH_SEND;
			    break;

			case CMDPNOOP:
			    break;      /* noop */

			default:
#ifdef DEBUG
			    tai_error ("unknown access parm", argv[ind-1],
					argc, argv);
#endif
			    continue;
		    }
		    break;

		case CMDCNOOP:
		    break;      /* noop */

		case CMDCLNAME:
		    chptr -> ch_lname =  argv[ind];
		    break;

		case CMDCLDOMN:
		    chptr -> ch_ldomain =  argv[ind];
		    break;

		case CMDCLISRC:
		    if ((chptr -> ch_insource = tb_nm2struct (argv[ind]))
							== (Table *) NOTOK)
		    {
#ifdef DEBUG
			tai_error ("unknown source table", argv[ind], argc, argv);
#endif
			continue;
		    }
		    break;

		case CMDCLOSRC:
		    if ((chptr -> ch_outsource = tb_nm2struct (argv[ind]))
							== (Table *) NOTOK)
		    {
#ifdef DEBUG
			tai_error ("unknown source table", argv[ind], argc, argv);
#endif
			continue;
		    }
		    break;

		case CMDCLIDEST:
		    if ((chptr -> ch_indest = tb_nm2struct (argv[ind]))
							== (Table *) NOTOK)
		    {
#ifdef DEBUG
			tai_error ("unknown dest table", argv[ind], argc, argv);
#endif
			continue;
		    }
		    break;

		case CMDCLODEST:
		    if ((chptr -> ch_outdest = tb_nm2struct (argv[ind]))
							== (Table *) NOTOK)
		    {
#ifdef DEBUG
			tai_error ("unknown dest table", argv[ind], argc, argv);
#endif
			continue;
		    }
		    break;

		case CMDCKNOWN:
		    if ((chptr -> ch_known = tb_nm2struct (argv[ind]))
							== (Table *) NOTOK)
		    {
#ifdef DEBUG
			tai_error ("unknown known table", argv[ind], argc, argv);

#endif
			continue;
		    }
		    break;

		case CMDCCONFSTR:
		    chptr -> ch_confstr = argv [ind];
		    break;

		case CMDCAP:
#ifdef DEBUG
		    ll_log (logptr, LLOGFTR, "Ap style '%s'", argv[ind]);
#endif
		    switch (cmdsrch (argv[ind], 0, parmap))
		    {
			case CMDASAME:
			    chptr -> ch_apout = AP_SAME;
			    break;
			case CMDA733:
			    chptr -> ch_apout = AP_733;
			    break;
			case CMDA822:
			    chptr -> ch_apout = AP_822;
			    break;
			case CMDABIG:
			    chptr -> ch_apout |= AP_BIG;
			    break;
			case CMDANODOTS:
			    chptr -> ch_apout |= AP_NODOTS;
			    break;
			case CMDAJNT:
			     chptr -> ch_apout = (AP_733 | AP_BIG);
			     break;
			default:
#ifdef DEBUG
			    tai_error ("unknown address style", argv[ind], argc, argv);
#endif
			    continue;
		    }
		    break;

		case CMDCAUTH:
#ifdef DEBUG
		    ll_log (logptr, LLOGFTR, "Authorisation '%s'", argv[ind]);
#endif
		    switch (cmdsrch (argv[ind], 0, authmap))
		    {
			case CMDTFREE:
			    chptr -> ch_auth |= CH_FREE;
			    break;
			case CMDTINLOG:
			    chptr -> ch_auth |= CH_IN_LOG;
			    break;
			case CMDTINWARN:
			    chptr -> ch_auth |= CH_IN_WARN;
			    break;
			case CMDTINBLOCK:
			    chptr -> ch_auth |= CH_IN_BLOCK;
			    break;
			case CMDTOUTLOG:
			    chptr -> ch_auth |= CH_OUT_LOG;
			    break;
			case CMDTOUTWARN:
			    chptr -> ch_auth |= CH_OUT_WARN;
			    break;
			case CMDTOUTBLOCK:
			    chptr -> ch_auth |= CH_OUT_BLOCK;
			    break;
			case CMDTHAU:
			    chptr -> ch_auth |= CH_HAU;
			    break;
			case CMDTDHO:
			    chptr -> ch_auth |= CH_DHO;
			    break;

			default:
#ifdef DEBUG
			    tai_error ("unknown auth type", argv[ind], argc, argv);
#endif
			    continue;
		    }
		    break;


		default:
#ifdef DEBUG
		    tai_error ("unknown channel parm", argv[ind], argc, argv);
#endif
		    break;
	    }
	}
    }
    tbname = (tbind >= 0) ? argv[tbind] : chptr -> ch_name;
    if ((chptr -> ch_table = tb_nm2struct (tbname)) == (Table *) NOTOK)
    {                   /* table does not already exist */
#ifdef DEBUG
	ll_log (logptr, LLOGBTR, "'%s' unknown table", tbname);
#endif
	if (tb_numtables >= tb_maxtables)
	{
#ifdef DEBUG
	    tai_error ("exceed tb table size", tbname, argc, argv);
#endif
	    return (NOTOK);
	}
	else
	{
	    /*NOSTRICT*/
	    tb_list[tb_numtables++] = chptr -> ch_table =
		(Table *) malloc ((unsigned) (sizeof (Table)));
	    tb_list[tb_numtables] = (Table *) 0;
	    chptr -> ch_table -> tb_name =  tbname;
	    chptr -> ch_table -> tb_file =  tbname;
	    chptr -> ch_table -> tb_fp   =  (FILE *) NULL;
	    chptr -> ch_table -> tb_pos  =  0L;
	    chptr -> ch_table -> tb_show =
		(showind >= 0 ? argv[showind] : tbname);
	}
    }
    chptr -> ch_show = (showind >= 0 ? argv[showind] : chptr -> ch_name);

    for (ind = 0; ch_tbsrch[ind] != (Chan *) 0; ind++) {
#ifdef DEBUG
	ll_log (logptr, LLOGFTR, "ch(%d) '%s'",
	    ind, ch_tbsrch[ind] -> ch_name);
#endif
	if (ch_tbsrch[ind] == chptr)
	   continue;
	if (lexequ(ch_tbsrch[ind] -> ch_name, chptr -> ch_name)) {
#ifdef DEBUG
		ll_log (logptr, LLOGFTR, "Replacing channel");
#endif
		ch_tbsrch[ind] = chptr;
		ch_tbsrch[--ch_numchans] = (Chan *) 0;
	}
    }
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ch_tai (ret=YES)");
    ll_log (logptr, LLOGFTR, "nam=%s, tbl=%s, que=%s, acc=0%o, path=%s",
	    chptr -> ch_name, chptr -> ch_show, chptr -> ch_queue,
	    chptr -> ch_access, chptr -> ch_ppath);
    ll_log (logptr, LLOGFTR, "lnam=%s, ldomain=%s, host=%s",
	    chptr -> ch_lname, chptr -> ch_ldomain, chptr -> ch_host);
    ll_log (logptr, LLOGFTR, "login=%s, pol=%d, script=%s",
	    chptr -> ch_login, (int) (chptr -> ch_poltime), chptr -> ch_script);
    ll_log (logptr, LLOGFTR, "ipcs=%s, ap=%d",
	chptr -> ch_confstr, chptr -> ch_apout);
#endif

    return (YES);
}

/**/

/*      The following is one of those ugly buggers that has to play
 *      hairy games, just to do something simple.  It uses the full
 *      log-tailoring call, just to convert a logging-level specification
 *      into a number.  Yuch.  (Dave Crocker)
 */

tai_llev (argc, argv)            /* return logging level value */
     int argc;
     char *argv[];
{
    LLog faklog;
    char *fakargv[4];

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "tai_llev (%s : %d)", argv[0], argc);
#endif

    fakargv[0] =  "=";
    fakargv[1] =  "level";
    fakargv[2] =  (char *) 0;
    fakargv[3] =  (char *) 0;

    if (argc <= 0)
	return (NOTOK);

    fakargv[2] = argv[0];       /* assign the user's string as the 'value' */
    faklog.ll_level = LLOGFST;  /* something reasonable? */
#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "tai_llev before (%d)", faklog.ll_level);
#endif
    if (tai_log (3, fakargv, &faklog) < 0)
    {
#ifdef DEBUG
	tai_error ("bad log level", argv[0], argc, argv);
#endif
	return (NOTOK);
    }
#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "tai_llev after (%d)", faklog.ll_level);
#endif
    return (faklog.ll_level);
}
