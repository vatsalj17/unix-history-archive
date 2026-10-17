/*
 *     MULTI-CHANNEL MEMO DISTRIBUTION FACILITY  (MMDF)
 *
 *
 *     Copyright (C) 1979,1980,1981  University of Delaware
 *
 *     Department of Electrical Engineering
 *     University of Delaware
 *     Newark, Delaware  19711
 *
 *     Phone:  (302) 738-1163
 *
 *
 *     This program module was developed as part of the University
 *     of Delaware's Multi-Channel Memo Distribution Facility (MMDF).
 *
 *     Acquisition, use, and distribution of this module and its listings
 *     are subject restricted to the terms of a license agreement.
 *     Documents describing systems using this module must cite its source.
 *
 *     The above statements must be retained with all copies of this
 *     program and may not be removed without the consent of the
 *     University of Delaware.
 *
 *
 *     version  -1    David H. Crocker    March   1979
 *     version   0    David H. Crocker    April   1980
 *     version  v7    David H. Crocker    May     1981
 *     version   1    David H. Crocker    October 1981
 *
 */
/*      check directory, file, and program conditions, for MMDF
 *
 *  Jan 82  D. Crocker    Cleaned up protection-setting, a bit more
 *                        Temporarily ignore group id mismatches
 *  Jun 82  S. Manion     Major functional upgrade, such as verifying
 *                        necessary alias entries exist.
 *  Feb 84  D. Long       Check for slave in cmddfldir...not /usr/mmdf/lib.
 *  Mar 84  D. Long       Report the line type of dialports.
 */

#include "util.h"
#include "mmdf.h"
#include <pwd.h>
#include <sys/stat.h>
#include "ch.h"
#ifndef NODIAL
#include "d_proto.h"
#include "d_structs.h"
#endif NODIAL
#include "gettys.h"
#include "dm.h"

#define MAXARG 50

/*  maximum number of output lines to be queued at once  */
#define         QUESIZ          20

/*  the maximum number of path names that can be remembered as
 *  having been checked already.  See chkpath()
*/
#define         MAXPATH         15

/*  various verbosity levels  */
#define         PERROR             01   /*  perror() should be called */
#define         ANYLEVEL          070   /*  any bit in this field */
#define         LEVEL1            010   /*  fatal */
#define         LEVEL1P      LEVEL1|PERROR
#define         LEVEL3            020   /*  major section */
#define         LEVEL4            030   /*  sub-section */
#define         LEVEL5            040
#define         LEVEL6            050
#define         LEVEL6_5          060   /*  warning [..] messages */
#define         LEVEL7            070   /* nitty gritty junk */
#define         LEVEL0       LEVEL1|PERROR

#define         BACKGROUND      LEVEL6  /* default verbosity */
#define         FINAL           0
#define         PARTIAL         1

extern char *dupfpath();
extern char *strncpy();
extern int  sys_nerr;
extern char *sys_errlist[];
extern int  errno;

extern LLog msglog;
extern Chan **ch_tbsrch;       /* defined channels */
extern Table **tb_list;        /* full list of known tables */

extern Domain **dm_list;
extern char *mmdflogin;        /* login name for mmdf processes */
extern char *mmtailor;        /* external tailoring information */


extern char     *lckdfldir,
	       *pn_quedir;

#ifdef JNTMAIL
int daemonuid,
    daemongid;
#define DAEMON  "daemon";       /* Can be overridden at compile time */
char daemonname[]  = DAEMON;
#endif JNTMAIL

extern char	*locname, *locdomain;

extern char
		*logdfldir,
		*phsdfldir,
		*tbldfldir,
		*tbldbm,
		*cmddfldir,
		*chndfldir,
		*mldfldir;

extern char
		*quedfldir,
		*tquedir,
		*aquedir,
		*mquedir,
		*squepref;

extern char
		*pathdeliver,
		*pathsubmit,
		*pathpkup,
		*pathmail;

extern char *tai_eptr;

extern int queprot;             /* protection on quedfldir[] parent */

extern struct ll_struct	msglog,
			chanlog;

#ifndef NODIAL
extern struct ll_struct	ph_log;
extern struct dialports *d_prts;
extern struct directlines *d_lines;
#endif NODIAL
extern struct passwd *getpwuid ();
extern struct passwd *getpwnam ();

struct stat statbuf;

struct prtque  {
    int q_level;
    char q_text[126];
};

int mmdfuid,
    mmdfgid;
int rootgid;
int verbosity = BACKGROUND;
char hdrfmt[] = "\n%-24s: %s\n";
char subhdrfmt[] = "    %-20s: %s\n";
char probfmt[] = "%-18s: %s\n";
char errflg;                    /* error during mmdf_init */
char *errstr();
/**/

main (argc, argv)
  int argc;
  char *argv[];
{
    char tmpfile[FILNSIZE];
    struct passwd *pwdptr;
    int ind;

    /*  check for the verbosity flag  */
    flaginit (argc, argv);

    que (BACKGROUND, "\n**  Asterisks indicate potentially serious anomolies.\n");
    que (LEVEL6_5, "[ Information which is bracketed is advisory. ]\n");
    qflush (LEVEL0);

    chktai ();  /* is the tailoring file there ? */

    mmdf_init (argv[0]);
    if (errflg)
	endit (NOTOK);
    ll_hdinit (&msglog, argv[0]);
    msglog.ll_file = dupfpath (msglog.ll_file, logdfldir);

#ifdef JNTMAIL
    if ((pwdptr = getpwnam (daemonname)) == (struct passwd *) NULL)
    {
      que (LEVEL1, hdrfmt, "** No /etc/passwd login", daemonname);
      endit (NOTOK);
    }
    daemonuid = pwdptr -> pw_uid;
    daemongid = pwdptr -> pw_gid;
#endif JNTMAIL

    que (BACKGROUND, hdrfmt, "Default local name", locname);
    que (LEVEL7, subhdrfmt, "", "this is your 'official' host name");
    qflush (LEVEL7);

    que (BACKGROUND, hdrfmt, "Default domain name", locdomain);
    que (LEVEL7, subhdrfmt, "", "this is your 'official' domain name");
    qflush (LEVEL7);

    /*  get uid & gid for mmdf login, for setting directory ownerships */
    if ((pwdptr = getpwnam (mmdflogin)) == (struct passwd *) NULL)
    {
	que (LEVEL1, hdrfmt, "** No /etc/passwd login", mmdflogin);
	endit (NOTOK);
    }
    mmdfuid = pwdptr -> pw_uid;
    mmdfgid = pwdptr -> pw_gid;

    que (LEVEL3, "\nMMDF login %-11s  : uid (%d), gid (%d)\n",
		mmdflogin, mmdfuid, mmdfgid);
    chkalias (mmdflogin);
    que (LEVEL7, subhdrfmt, "", "alias this to root or systems staff");
    qflush (LEVEL7);
    qflush (LEVEL3);

    /* check for hashed table, if required */
    if (tbldbm != (char *) 0)
    {
	que (BACKGROUND, hdrfmt, "MMDF DBM database", tbldbm);
	que (LEVEL7, hdrfmt, "", "compiled by dbmbuild");
	qflush (LEVEL7);

	getfpath (tbldbm, tbldfldir, tmpfile);
	strcat (tmpfile, ".dir");
	que (LEVEL7, subhdrfmt, "data directory", tmpfile);
	chkfile (tmpfile, 0644, 0664, mmdfuid, mmdfgid, mmdflogin);

	getfpath (tbldbm, tbldfldir, tmpfile);
	strcat (tmpfile, ".pag");
	que (LEVEL7, subhdrfmt, "data pages", tmpfile);
	chkfile (tmpfile, 0644, 0664, mmdfuid, mmdfgid, mmdflogin);
	qflush (LEVEL7);
    }

    que (BACKGROUND, hdrfmt, "Trouble report address", "Postmaster");
    que (LEVEL7, subhdrfmt, "", "alias this to root or systems staff");
    qflush (LEVEL7);
    chkalias ("Postmaster");    /*  make sure the name is aliased  */
    qflush (LEVEL0);

    if ((pwdptr = getpwuid (0)) != (struct passwd *) NULL)
	rootgid = pwdptr -> pw_gid;

    que (LEVEL3, hdrfmt, "Checking directories", "");

    /*  standard directories */
    que (LEVEL4, subhdrfmt, "Logging directory", logdfldir);
    if (chkfile (logdfldir, 0711, 0755, mmdfuid, mmdfgid, mmdflogin) >= OK)
	chklog ();
    qflush (LEVEL4);

    que (LEVEL4, "\n");
    que (LEVEL4, subhdrfmt, "Phase (timestamps)", phsdfldir);
    chkfile (phsdfldir, 0711, 0755, mmdfuid, mmdfgid, mmdflogin);
    qflush (LEVEL4);

#ifndef V4_2BSD
    que (LEVEL4, "\n");
    que (LEVEL4, subhdrfmt, "Locking directory", lckdfldir);
    chkfile (lckdfldir, 0777, 0777, mmdfuid, mmdfgid, mmdflogin);
    qflush (LEVEL4);
#endif V4_2BSD

#ifdef JNTMAIL
    que (LEVEL4, "\n");
    que (LEVEL4, subhdrfmt, "NIFTP spool directory", pn_quedir);
    chkfile (pn_quedir, 0700, 0700, daemonuid, daemongid, daemonname);
    qflush (LEVEL4);
#endif

    que (LEVEL4, "\n");
    que (LEVEL4, subhdrfmt, "Tables & scripts", tbldfldir);
    if (chkfile (tbldfldir, 0711, 0755, mmdfuid, mmdfgid, mmdflogin) >= OK)
	chktab ();
    qflush (LEVEL4);

    que (LEVEL4, hdrfmt, "MMDF Commands", cmddfldir);
    if (chkfile (cmddfldir, 0711, 0755, mmdfuid, mmdfgid, mmdflogin) >= OK)
	chkcmd ();
    qflush (LEVEL4);

    que (LEVEL4, hdrfmt, "Channel programs", chndfldir);
    if (chkfile (chndfldir, 0500, 0770, mmdfuid, mmdfgid, mmdflogin) >= OK)
	chkchan ();
    qflush (LEVEL4);

    if (isstr(mldfldir))
    {                           /* all mail delivered in common dir     */
	que (LEVEL4, hdrfmt, "Shared receipt directory", mldfldir);
	chkfile (mldfldir, 0777, 0777, mmdfuid, mmdfgid, mmdflogin);
	qflush (LEVEL4);
    }
    qflush (LEVEL0);

    /*  queue directory substructure */
    que (LEVEL4, hdrfmt, "Mail queue home", quedfldir);
    if (chkpath (FINAL, quedfldir, 0777, 0777, mmdfuid, mmdfgid, mmdflogin) < OK)
	que (LEVEL1, "cannot check queue further\n");
    else
    {
	que (LEVEL7, "\t(changing into queue home directory...)\n");
	if (chdir (quedfldir) < OK)
	{
	    que (LEVEL1, probfmt, "cannot chdir", errstr());
	    que (LEVEL1, "unable to check queue directory tree further\n");
	}
	else
	{
	    qflush (LEVEL7);
	    que (LEVEL5, subhdrfmt, "Queue lock", "home's parent directory");
	    if (stat ("..", &statbuf) < OK)
		que (LEVEL1P, "cannot stat home queue parent, ");
	    else
	    {
		statbuf.st_mode &= 06707;
		if (statbuf.st_mode != queprot)
		    que (LEVEL1, "Wrong mode        : (0%o) should be (0%o)\n",
				statbuf.st_mode, queprot);
	    }
	    qflush (LEVEL5);

	    que (LEVEL5, subhdrfmt, "Address-building", tquedir);
	    chkpath (FINAL, tquedir, 0777, 0777, mmdfuid, mmdfgid, mmdflogin);
	    qflush (LEVEL5);

	    que (LEVEL5, subhdrfmt, "Queued addresses", aquedir);
	    chkpath (FINAL, aquedir, 0777, 0777, mmdfuid, mmdfgid, mmdflogin);
	    qflush (LEVEL5);

	    que (LEVEL5, subhdrfmt, "Message text", mquedir);
	    chkpath (FINAL, mquedir, 0777, 0777, mmdfuid, mmdfgid, mmdflogin);
	    qflush (LEVEL5);

	    que (LEVEL5, "Queueing directories:\n");
	    for (ind = 0; ch_tbsrch[ind] != (Chan *) 0; ind++)
	    {
	    	static char path[64];

	    	(void) sprintf (path, "%s%s",
		    squepref, ch_tbsrch[ind] -> ch_queue);
		que (LEVEL5, subhdrfmt, ch_tbsrch[ind] -> ch_show, path);
		chkpath (FINAL, path, 0777, 0777, mmdfuid, mmdfgid, mmdflogin);
		qflush (LEVEL5);
	    }
	}
    }
    qflush (LEVEL0);

#ifndef NODIAL
    que (LEVEL4, hdrfmt, "Dial out ports", "");
    chk_dprts ();               /*  check the dial out ports  */
    qflush (LEVEL0);

    que (LEVEL4, hdrfmt, "Direct-connect lines", "");
    chk_dirlin ();              /*  check the direct lines  */
    qflush (LEVEL0);
#endif NODIAL

    endit (OK);
}
/**/

flaginit (argc, argv)
int argc;
char *argv[];
{
    register int argind, charind;

    for (argind = 1;  argind < argc;  argind++)
	if (argv[argind][0] == '-')
	    for (charind = 1;  argv[argind][charind] != '\0';  charind++)
		switch (argv[argind][charind])
		{
		    case 'p':           /* only note problems */
			verbosity = LEVEL1;
			break;

		    case 'v':
			verbosity = atoi (&argv[argind][++charind]);
			if (verbosity < 0)
			    verbosity = LEVEL1;
			if (verbosity == 0)
			    verbosity = LEVEL7;
			while (isdigit (argv[argind][charind]))
			    charind++;
			charind--;
			break;

		    default:
			printf ("Unknown flag %c\n", argv[argind][charind]);
			printf ("Usage:  %s [-p][-v<verbosity>]\n", argv[0]);
			exit (NOTOK);
		}
}
/**/

#ifndef NODIAL
chk_dprts ()
{
    struct ttys data;
    char desc[30];
    register int index, result;

    for (index = 0;  d_prts[index].p_port != NULL;  index++)
    {
	/*  check the dial out lines  */
	(void) sprintf (desc, "0%o %.25s", d_prts[index].p_speed,
	    d_prts[index].p_ltype);
	que (LEVEL5, subhdrfmt, d_prts[index].p_port, desc);
	qflush (LEVEL5);

	/*  verify that the device exists  */
	if (stat (d_prts[index].p_port, &statbuf) < 0)
	    que (LEVEL1, probfmt, "No port", errstr());
	else
	    que (LEVEL7, "\tPort exists\n");
	qflush (LEVEL7);

	/*  make sure it is not enabled  */
	result = getttynam (&data, &d_prts[index].p_port[5]);
	if (result == BADDATA)
	    que (LEVEL1, "No listing        : Port not in /etc/ttys\n");
	else
	    if (data.t_valid == 0)
		que (LEVEL7, "\tPort is set correctly (disabled)\n");
	    else
		que (LEVEL1, "Wrong state      : Port should not be enabled\n");
	qflush (LEVEL7);

	/*  make sure the lines dialer exists  */
	if (stat (d_prts[index].p_acu, &statbuf) < 0)
	    que (LEVEL1P, "No dialer         : (%s)\n", d_prts[index].p_acu);
	else
	    que (LEVEL7, "\tDialer (%s) exists\n", d_prts[index].p_acu);

	qflush (LEVEL5);
    }
    if (index == 0)
    {
	que (LEVEL5, subhdrfmt, "None", "");
	qflush (LEVEL5);
    }
}
/**/

chk_dirlin()
{ 
    register int index, ret;
    struct ttys ttybuf;

    for (index = 0;  d_lines[index].l_name != 0;  index++)
    {
	que (LEVEL5, subhdrfmt, d_lines[index].l_name, d_lines[index].l_tty);
	qflush (LEVEL5);

	/*  verify that the device exists  */
	if (stat (d_lines[index].l_tty, &statbuf) < 0)
	    que (LEVEL1, probfmt, "No line", errstr());
	else
	    que (LEVEL7, "\tLine exists\n");
	qflush (LEVEL7);

	/*  Is the line disabled?  */
	ret = getttynam (&ttybuf, &d_lines[index].l_tty[5]);
	if (ret == BADDATA)
	    que (LEVEL1, "No listing        : Line not in /etc/ttys\n");
	else
	    if (ttybuf.t_valid == 0)
		que (LEVEL7, "\tLine setup correctly (disabled)\n");
	    else
		que (LEVEL1,
			"Wrong state       : Line must NOT be enabled for terminal login\n");

	qflush (LEVEL5);
    }
    if (index == 0)
    {
	que (LEVEL5, subhdrfmt, "None", "");
	qflush (LEVEL5);
    }
}
#endif NODIAL
/**/

chklog ()
{

    que (LEVEL7, "\n(changing into log directory...)\n");
    qflush (LEVEL6);

    if (chdir (logdfldir) < OK)
    {
	que (LEVEL1, probfmt, "cannot chdir", errstr());
	return;
    }

    que (LEVEL6, subhdrfmt, "message-level log", msglog.ll_file);
    chkfile (msglog.ll_file, 0622, 0666, mmdfuid, mmdfgid, mmdflogin);
    qflush (LEVEL6);

    que (LEVEL6, subhdrfmt, "channel log", chanlog.ll_file);
    chkfile (chanlog.ll_file, 0622, 0666, mmdfuid, mmdfgid, mmdflogin);
    qflush (LEVEL6);

#ifndef NODIAL
    que (LEVEL6, subhdrfmt, "phone (link) log", ph_log.ll_file);
    chkfile (ph_log.ll_file, 0622, 0666, mmdfuid, mmdfgid, mmdflogin);
    qflush (LEVEL6);
#endif NODIAL
}
/**/

chktab ()
{
    int didone;
    register int ind, n;

    que (LEVEL7, "\n(changing into table directory...)\n");
    qflush (LEVEL6);

    if (chdir (tbldfldir) < OK)
    {
	que (LEVEL1, probfmt, "cannot chdir", errstr());
	return;
    }

    que (LEVEL4, "\nChannel name tables & associated system entries:\n");
    for (ind = 0; ch_tbsrch[ind] != (Chan *) 0; ind++)
    {
	if (ind > 0)
	    que (LEVEL5, "\n");
	que (LEVEL5, subhdrfmt, ch_tbsrch[ind] -> ch_show,
			ch_tbsrch[ind] -> ch_name);
	que (LEVEL6, subhdrfmt, "Queue name", ch_tbsrch[ind] -> ch_queue);
	qflush (LEVEL6);

	/*  verify that ch_name is legal, ie, less than 8 chars
	 *  long, and containing only alphanumerics or -.
	 */
	for (n = 0;  ch_tbsrch[ind] -> ch_name[n] != '\0';  n++)
	    if( ((isalpha (ch_tbsrch[ind] -> ch_name[n]) == 0) &&
		 (ch_tbsrch[ind] -> ch_name[n] != '-'      )   ) ||
		(n > 8                                          )   )
		break;
	/*  make sure the loop terminated normally  */
	if (n > 8)
	    que (LEVEL1, "Long name         : '%s' should be 8 or fewer characters\n",
			ch_tbsrch[ind] -> ch_name);
	else
	    if (ch_tbsrch[ind] -> ch_name[n] != '\0')
		que (LEVEL1, "Illegal char      : (%c) in '%s'\n",
		    ch_tbsrch[ind] -> ch_name[n], ch_tbsrch[ind] -> ch_name);

	que (LEVEL6, subhdrfmt, "  Local host name",
				    ch_tbsrch[ind] -> ch_lname);
	que (LEVEL6, subhdrfmt, "  Local domain name",
				    ch_tbsrch[ind] -> ch_ldomain);
	qflush (LEVEL6);

	cktable (ch_tbsrch[ind] -> ch_table, "Channel table");

	if (ch_tbsrch[ind] -> ch_indest != (Table *) 0)
	    cktable (ch_tbsrch[ind] -> ch_indest, "Destination filter");

	if (ch_tbsrch[ind] -> ch_outdest != (Table *) 0)
	    cktable (ch_tbsrch[ind] -> ch_outdest, "Destination filter");

	if (ch_tbsrch[ind] -> ch_insource != (Table *) 0)
	    cktable (ch_tbsrch[ind] -> ch_insource, "Source filter");

	if (ch_tbsrch[ind] -> ch_outsource != (Table *) 0)
	    cktable (ch_tbsrch[ind] -> ch_outsource, "Source filter");

	if (ch_tbsrch[ind] -> ch_known != (Table *) 0)
	    cktable (ch_tbsrch[ind] -> ch_known, "Table of known hosts");

	if (ch_tbsrch[ind] -> ch_script != 0)
	{
	    que (LEVEL6, subhdrfmt, "Dialing script",
				    ch_tbsrch[ind] -> ch_script);
	    if (stat (ch_tbsrch[ind] -> ch_script, &statbuf) < OK)
		que (LEVEL1, probfmt, "cannot stat", errstr());
	    qflush (LEVEL6);
	}

	if (ch_tbsrch[ind] -> ch_trans != (char *) DEFTRANS)
	{
	    que (LEVEL6, subhdrfmt, "Phone transcript",
				ch_tbsrch[ind] -> ch_trans );
	    qflush (LEVEL6);
	}
	if (ch_tbsrch[ind] -> ch_login != NOLOGIN)
	    chklogin (ch_tbsrch[ind]);
	qflush (LEVEL5);
    }
    qflush (LEVEL4);

    que (LEVEL4, hdrfmt, "Domain tables", "");
    for (didone = FALSE, ind = 0; dm_list[ind] != (Domain *) 0; ind++)
    {
	if (dm_list[ind] -> dm_table -> tb_fp == (FILE *) NOTOK)
	    continue;
	didone = TRUE;
	que (LEVEL5, subhdrfmt,
	     isstr(dm_list[ind] -> dm_name) ? dm_list[ind] -> dm_name : "(root)",
	     dm_list[ind] -> dm_show);

	/*  verify that spec is legal, containing only alphanumerics or -.
	 */
	for (n = 0;  dm_list[ind] -> dm_name[n] != '\0';  n++)
	    if( ((isalpha (dm_list[ind] -> dm_name[n]) == 0) &&
		 (dm_list[ind] -> dm_name[n] != '-'      )  &&
		 (dm_list[ind] -> dm_name[n] != '.'      )   ) )
		break;

	/*  make sure the loop terminated normally  */
	if (dm_list[ind] -> dm_name[n] != '\0')
	    que (LEVEL1, "Illegal char      : (%c) in '%s'\n",
		dm_list[ind] -> dm_name[n], dm_list[ind] -> dm_name);

	cktable(dm_list[ind] -> dm_table, "");
	qflush (LEVEL5);
    }
    if (!didone)
    {
	que (LEVEL5, subhdrfmt, "None", "");
	qflush (LEVEL5);
    }

    que (LEVEL4, hdrfmt, "Additional tables", "");
    for (didone = FALSE, ind = 0; tb_list[ind] != (Table *) 0; ind++)
    {
	if (tb_list[ind] -> tb_fp == (FILE *) NOTOK)
	    continue;           /* already done */

	didone = TRUE;
	que (LEVEL5, subhdrfmt, tb_list[ind] -> tb_show,
				tb_list[ind] -> tb_name);

	/*  verify that spec is legal, ie, less than 8 chars
	 *  long, and containing only alphanumerics or -.
	 */
	for (n = 0;  tb_list[ind] -> tb_name[n] != '\0';  n++)
	    if( ((isalpha (tb_list[ind] -> tb_name[n]) == 0) &&
		 (tb_list[ind] -> tb_name[n] != '-'      )   &&
		 (tb_list[ind] -> tb_name[n] != '.'      )   ) )
		break;

	/*  make sure the loop terminated normally  */
	if (tb_list[ind] -> tb_name[n] != '\0')
	    que (LEVEL1, "Illegal char      : (%c) in '%s'\n",
		tb_list[ind] -> tb_name[n], tb_list[ind] -> tb_name);

    	cktable (tb_list[ind], "");
	qflush (LEVEL5);
    }
    if (!didone)
    {
	que (LEVEL5, subhdrfmt, "None", "");
	qflush (LEVEL5);
    }
    qflush (LEVEL4);
}
/**/

cktable(tb, title)
register Table *tb;
register char *title;
{
    struct stat statbuf;

    tb -> tb_fp = (FILE *) NOTOK;	/* flag this table as processed */
    if ((tb -> tb_flags&TB_SRC) == TB_NS) {
	que (LEVEL6, subhdrfmt, title, "(via nameserver)");
    	return;
    } else {
	que (LEVEL6, subhdrfmt, title, tb -> tb_file);
	if (stat (tb -> tb_file, &statbuf) < OK)
	    que (LEVEL1, probfmt, "cannot stat", errstr());
    }
    qflush (LEVEL6);
}

chklogin (chanptr)
Chan *chanptr;
{
    register char *name;
    struct passwd *ret, *getpwnam ();
    char slaveloc[FILNSIZE];
    int mode;

    /*  transfer to a more accessable variable  */
    name = chanptr -> ch_login;

    /*  verify that there is a login for this channel  */
    ret = getpwnam (name);
    if (ret == NULL)
    {
	que (LEVEL1, "No login          : for '%s'\n", name);
	return;
    }

    que (LEVEL6, subhdrfmt, "Login name", name);
    chkalias (name);            /*  make sure the name is aliased  */
    qflush (LEVEL6);

    /*  make sure the login program is the standard slave program  */
    if ((initstr("pobox", chanptr -> ch_ppath, 5) != (-1)) &&
	((chanptr -> ch_access & DLVRPSV) != 0        )   )
	{
	strcat(strcpy(slaveloc,cmddfldir), "/slave");
	if (strcmp (ret -> pw_shell, slaveloc) != 0)
	    que (LEVEL1, "Bad login prog    : '%s' is not phonenet slave\n",
			    ret -> pw_shell);
	else
	    que (LEVEL6, subhdrfmt, "Login program", ret -> pw_shell);
	qflush (LEVEL6);
    }
    else
	que (LEVEL6_5, "\t[ Login program : %s ]\n", ret -> pw_shell);
    qflush (LEVEL6_5);

    /*  verify that the login directory exists and check protections  */
    if (stat (ret -> pw_dir, &statbuf) < OK)
    {
	que (LEVEL1P, "Bad login dir     : Couldn't stat '%s'; ",
			ret -> pw_dir);
	return;
    }

    mode = statbuf.st_mode & 04777;
    if ((mode&06707) != 0701)
	que (LEVEL1, "Wrong mode        : (0%o) on login directory; should be 711\n",
			mode, ret -> pw_dir);
}
/**/

chkalias (name)
char *name;
{
    char alias[LINESIZE];

    if (aliasfetch(TRUE, name, alias, 1) == OK) {
	que (LEVEL6, subhdrfmt, "Alias value", alias);
    } else if (aliasfetch(TRUE, name, alias, 0) == OK) {
	que (LEVEL1, "Bypassable alias  : '%s' (** normally not bypassable)\n",
		alias);
    } else {
	que (LEVEL1, "No mail alias     : for address '%s'\n", name);
    }
    qflush (LEVEL6);
}



/**/

chkcmd ()
{
    que (LEVEL7, "\n(changing into command library...)\n");
    qflush (LEVEL7);

    if (chdir (cmddfldir) < OK)
    {
	que (LEVEL1, probfmt, "cannot chdir", errstr());
	return;
    }

    que (LEVEL6, subhdrfmt, "Posting/submission", pathsubmit);
    chkfile (pathsubmit, 04711, 04755, mmdfuid, mmdfgid, mmdflogin);
    qflush (LEVEL6);

    que (LEVEL6, subhdrfmt, "Delivery management", pathdeliver);
    chkfile (pathdeliver, 04711, 04755, 0, rootgid, "root");
    qflush (LEVEL6);

    if (strcmp (pathdeliver, pathpkup) != 0)
    {
	que (LEVEL6, subhdrfmt, "P.O. Box retrieval", pathpkup);
	chkfile (pathpkup, 04711, 04755, 0, rootgid, "root");
	qflush (LEVEL6);
    }

    que (LEVEL6, subhdrfmt, "V6Mail (for notices)",
		pathmail);
    chkfile (pathmail, 0711, 0755, mmdfuid, mmdfgid, mmdflogin);
    qflush (LEVEL6);

    que (LEVEL6, subhdrfmt, "cleanque", "Queue garbage cleaner");
    chkfile ("cleanque", 04711, 04755, 0, rootgid, "root");
    qflush (LEVEL6);

    que (LEVEL6, subhdrfmt, "checkque", "Queue status checker");
    chkfile ("checkque", 04711, 04755, mmdfuid, mmdfgid, mmdflogin);
    qflush (LEVEL6);

    que (LEVEL6, subhdrfmt, "setlogs", "Log cleanup shellfile");
    chkfile ("setlogs", 0700, 0755, mmdfuid, mmdfgid, mmdflogin);
    qflush (LEVEL6);
}
/**/

chkchan ()
{
    register int ind;
    register Chan *chan;

    que (LEVEL7, "\n(changing into channel directory...)\n");
    qflush (LEVEL7);

    que (BACKGROUND,"\t[ Some channels, such as Local, need to be setuid ]\n");
    qflush (BACKGROUND);

    if (chdir (chndfldir) < OK)
	que (LEVEL1, probfmt, "cannot chdir", errstr());
    else
    {
	for (ind = 0; (chan = ch_tbsrch[ind]) != (Chan *) 0; ind++)
	{
	    que (LEVEL5, subhdrfmt, chan -> ch_show, "");
	    if (chan -> ch_ppath[0] == '\0')
		que (LEVEL1, "No channel prog\n");
	    else
	    {
		que (LEVEL6, subhdrfmt, "Channel program",
				    chan -> ch_ppath);
		if (stat (chan -> ch_ppath, &statbuf) < OK)
		    que (LEVEL1, probfmt, "Can't stat prog", errstr());
		qflush (LEVEL6);
	    }
	    if (chan -> ch_logfile) {
	    	chan -> ch_logfile = dupfpath(chan -> ch_logfile, logdfldir);
		que (LEVEL6, subhdrfmt, "logfile", chan -> ch_logfile);
		chkfile (chan -> ch_logfile, 0622, 0666, mmdfuid, mmdfgid, mmdflogin);
		qflush (LEVEL6);
	    }
	    qflush (LEVEL5);
	}
    }
}
/**/

chkfile (dirptr, maxprot, minprot, owner, group, ownname)
register char *dirptr;
int maxprot, minprot, owner, group;
char ownname[];
{
    register char *nptr;
    char partpath[128];

    /*  verify that I have been given a real path name  */
    if (!isstr(dirptr))
    {
	que (LEVEL1, "useless filename\n");
	return (NOTOK);
    }

    /*  step through the path, stopping at /'s to check the
     *  directory.
     */
    nptr = partpath;
    while ((*nptr++ = *dirptr++) != '\0')
	/*  Note that this checks the NEXT char, not the one just
	 *  transfered.
	 */
	if (*dirptr == '/')
	{
	    *nptr = '\0';
	    if (chkpath (PARTIAL, partpath, maxprot, minprot, owner, group, ownname) != OK)
		return (NOTOK);
	}

    /*  At this point the entire pathname has been transfered.
     *  Check it (errors here are more serious than above.)
     */
    if (chkpath(FINAL, partpath, maxprot, minprot, owner, group, ownname) != OK)
	return (NOTOK);
    return (OK);
}
/**/

/*  Note that a return of OK from this routine indicates only
 */
/*ARGSUSED*/
chkpath (control, name, maxprot, minprot, owner, group, ownname)
int control;
char *name;
int maxprot, minprot, owner, group;
char *ownname;
{
    struct passwd *pwdptr;
    static char pathdone[MAXPATH][50];
    register int n;

    /*  First, must check to see if the dir is statable.
     *  A failure here must return NOTOK even if this dir
     *  has been checked before.
     */
    if (stat (name, &statbuf) < OK)
    {
	que (LEVEL1, probfmt, "unable to stat", errstr());
    	if (control == PARTIAL)
	    que (LEVEL1, "Unable to proceed checking this path.\n");
	return (NOTOK);
    }

    /*  If this path has already been checked, then don't
     *  do it again.  The repeated error msgs are annoying.
     */
    for (n = 0;  ((n < MAXPATH) && (pathdone[n][0] != '\0'));  n++)
	if (strcmp (name, pathdone[n]) == 0)
	    return (OK);

    /*  save this name as one that has been checked  */
    if (n < MAXPATH)
	(void) strcpy (pathdone[n], name);

    /*  verify that the owner is correct  */
    if (statbuf.st_uid != owner)
    {
	if ((pwdptr = getpwuid (statbuf.st_uid)) != NULL)
	{
	    if (control == FINAL)
		que (LEVEL1, "Wrong owner       : (%s) for '%s'; should be %s\n",
			pwdptr -> pw_name, name, ownname);
	    else
		que (LEVEL6_5, "\t[ Wrong owner   : (%s) for '%s'; should be %s ]\n",
			pwdptr -> pw_name, name, ownname);
	}
	else
	{
	    if (control == FINAL)
		que (LEVEL1, "Wrong owner      : (uid %d) for '%s'; should be %s\n",
				statbuf.st_uid, name, ownname);
	    else
		que (LEVEL6_5, "\t[ Wrong owner  : (uid %d) for '%s'; should be %s ]\n",
				statbuf.st_uid, name, ownname);
	}
	qflush (LEVEL6_5);
    }

    /* 
     *  Ingore bits that are site choice and not critical to operation.
     */
    if (control == PARTIAL) {
    	minprot |= 0111;
    	maxprot |= 0111;
    }
    statbuf.st_mode &= 06707;
    minprot &= 06707;
    maxprot &= 06707;
    if ((statbuf.st_mode&(~minprot)) || 	/* Any extra bit on? */
        (statbuf.st_mode&maxprot) != maxprot)	/* Any bits missing? */
    {
	if (control == FINAL)
	    que (LEVEL1, "Wrong mode        : (0%o) for '%s'; should be (0%o to 0%o)\n",
			statbuf.st_mode, name, maxprot, minprot);
	else
	    que (LEVEL6_5, "\t[ Wrong mode    : (0%o) for '%s'; should be (0%o to 0%o) ]\n",
			statbuf.st_mode, name, maxprot, minprot);
	qflush (LEVEL6_5);
    }
    return (OK);
}

chktai ()
{
    if (mmtailor != (char *) 0)
    {
	if (stat (mmtailor, &statbuf) < OK)
	{
	    que (LEVEL1, "No tailor file   : %s", mmtailor);
	    endit (NOTOK);
	}
	else
	    que (LEVEL4, hdrfmt, "Tailor file", mmtailor);
	qflush (LEVEL4);
    }
}
/**/

int prtmax;
struct prtque prtque[QUESIZ];

qflush (control)
int control;
{
    register int index, prtany;

    /*  determine if any of the queue should be output */
    for (prtany = index = 0;  index < prtmax;  index++)
	if (prtque[index].q_level <= verbosity)
	{
	    prtany = 1;
	    break;
	}

    if (prtany != 0)        /*  go through and print appropriate lines  */
    {
	for (index = 0;  index < prtmax;  index++)
		printf ("%s", prtque[index].q_text);
	prtmax = 0;            /* start over */
    }
    else
    {
	for (index = 0;     /* remove excess only */
		(index < prtmax) && (prtque[index].q_level < control);
		index++)
	    ;
	prtmax = index;
    }
    return;
}

/*VARARGS1*/
que (control, msg, arga, argb, argc, argd)
int control;
char *msg, *arga, *argb, *argc, *argd;
{
    char tmp[128];

    if (prtmax < (QUESIZ-1))
	format (prtque[prtmax].q_text, control, msg, arga, argb, argc, argd);
    else
    {
	qflush (LEVEL0);
	printf ("Recompile this program with larger QUESIZ.\n");
	printf ("Queue overflow on:\n  ");
	format (tmp, control, msg, arga, argb, argc, argd);
	printf (tmp);
	exit (NOTOK);
    }
    prtque[prtmax].q_level = control & ~PERROR;
    if (prtque[prtmax].q_level <= verbosity)
	qflush (prtque[prtmax++].q_level);
    else
	prtmax++;
    return;
}
/**/

/*VARARGS2*/
format (buff, control, msg, arga, argb, argc, argd)
char buff[];
int control;
char *msg, *arga, *argb, *argc, *argd;
{
    register int n;

    switch (control&~PERROR)
    {
	case LEVEL1:
	    (void) strncpy (buff, "**    ", 6);
	    n = 6;
	    break;

	default:
	    n = 0;
    }

    (void) sprintf (&buff[n], msg, arga, argb, argc, argd);

    if ((control & PERROR) && (errno > 0)) {
    	n = strlen(buff);
	(void) sprintf(&buff[n], " (%s)\n", errstr());
    }

    return;
}

char *
errstr()
{
    static char buff[64];

    if (errno > sys_nerr || errno < 0)
	(void) sprintf (buff, "Errno %d", errno);
    else
	(void) strcpy(buff, sys_errlist[errno]);
    return (buff);
}
/**/
/*ARGSUSED*/
post_tai (argc, argv)           /* let user know about unknowns tailor info */
	int argc;
	char *argv[];
{
    char errline[LINESIZE];

    arg2vstr (0, sizeof errline, errline, argv);
    if (tai_eptr == (char *) 0)
    {
	if (errno == 0)
	    que (LEVEL1, "Unknown tailor    : '%s'\n", errline);
	else
	    que (LEVEL1, "Tailor error      : '%s'\n", errline);
    }
    else                        /* problem with subfield, not field name */
	que (LEVEL1, "Tailor error      : with '%s' in line: '%s'\n",
		    tai_eptr, errline);

    if (errno > 0 && errno <= sys_nerr)
	que (LEVEL1, "                  : [ %s ]\n", sys_errlist[errno]);

    errflg = TRUE;

    return (YES);   /* we like everthing */
}

endit (type)
    int type;
{
    qflush (LEVEL0);
    (void) fflush (stdout);
    exit (type);
}
