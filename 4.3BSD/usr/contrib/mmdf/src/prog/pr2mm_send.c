/*
 *	MULTI-CHANNEL MEMO DISTRIBUTION FACILITY  (MMDF)
 *
 *	Department of Electrical Engineering
 *	University of Delaware
 *	Newark, Delaware  19711
 *
 *
 *	Program Channel (inbound): Pass message into MMDF
 *
 *
 *	P R 2 M M _ S E N D . C
 *	=======================
 *
 *	pr2mm_send [-M?] [-D] [-c <channel>] [-h|-v <host>]
 *						[-s <sender>] [address...]
 *
 *	specify -h if host in US order, -v (for Via:) if in UK order!
 *	specify -Mj if input is JNT mail file.
 *	-Ms is reserved for Batch-SMTP input format (implementation coming)
 *	(default mode: give recipients on command line)
 *
 *	J.B.D.Pardoe
 *	University of Cambridge Computer Laboratory
 *	October 1985
 *	
 */

#include <stdio.h>
#include <errno.h>
#ifdef V4_2BSD
#include <sys/wait.h>
#include <sys/file.h>
#endif V4_2BSD
#include <signal.h>
#include "util.h"
#include "mmdf.h"
#include "ch.h"
#include "ap.h"


#define EX_OK	0 /* everything successful */
#define EX_ADDR	1 /* bad addresses */
#define EX_MECH	2 /* more than just bad addresses */
#define EX_FAIL	3 /* the program failed to behave */

#define PR_COMM '#'
#define PR_ERR  '*'

/* 
 * Define  SAFEFORK  if  you  want  to  be ultra careful that no mail is
 * lost;  we try and notify the  sender  or  support  that  we  couldn't
 * deliver  some  mail,  but  if  we  can't then the mail will disappear
 * without trace.  Other problems such as  invalid  arguments  may  also
 * cause  this.  If  you  define  SAFEFORK the process doing the work is
 * monitored by another so  that  (as  long  as  mail  can  be  sent  to
 * support) some notification of the error is made.
 */
#define SAFEFORK 


#define TRY(OPERATION, WHAT) { \
    int rp  = OPERATION; \
    if (rp_isbad (rp)) terminate (WHAT, rp); \
}

#define invalid_address(ADR, RPLY) { \
    reportf ("address: %s\nproblem: %s\n", ADR, (RPLY)->rp_line); \
}


extern FILE *errmsg_file;
extern long msg_start;
extern char *mmdflogin, *supportaddr;
extern int errno;
extern char *ap_dmflip ();
extern int ap_outtype;


static char 
    *basic[]  = {"success", "partial", "temporary", "error"},
    *domain[] = {"syntax", "general", "transfer", "authentication", 
                 "mail", "file system", "<illegal>", "<illegal>"};


int prog_debug;
char *sender;

char prog_mode;



terminate (s, rp)
   char *s; int rp;
{
    printf ("%cfailed to %s\n%c  %s [%03o = %s: %s %o]\n", 
	PR_ERR, s, PR_ERR, rp_valstr (rp), rp & 0377,
	basic[rp_gbbit (rp)], domain[rp_gcbit (rp)], rp_gsbit (rp));
    mm_end (rp);
    exit (EX_MECH);
}	


/* *
 * monitoring of process which does the work
 */


#ifdef SAFEFORK

/*
 * monitor a forked process which does the work
 */

char *signame (sig)
    int sig;
{
    extern char *sys_siglist[];
    static char buff[24];

    if (sig >= 0 && sig < NSIG) {
	return (sys_siglist [sig]);
    } else {
	sprintf (buff, "unknown signal %d", sig);
	return (buff);
    }
}


main (argc, argv) 
    int argc; char **argv;
{
    char *divider = "-------------------------------------------------------";
    int child_pid, pp[2];
    char **argv_old = argv;

    mmdf_init (*argv);
    
    if (pipe (pp) != 0 || (child_pid = fork ()) == -1) {
	mopen (supportaddr, "Problems with `recvprog' program");
	mprintf ("Failed to start up mail reception program.\n");
	mprintf ("The arguments were:\n");
	while (argc-- != 0) mprintf ("  %s", *argv++);
	mprintf ("\n\nThe message follows\n\%s\n", divider);
	minclude (stdin);
	exit (mclose () == 0 ? EX_MECH : EX_FAIL);
    }
    
    if (child_pid == 0) {
	dup2 (pp[1], 1);
	dup2 (pp[1], 2);
	close (pp[0]);
	child (argc, argv);
    } else {
	int rc, stopped;
	int do_msg = 0;
#ifdef V4_2BSD
	union wait status;
#else
    	int status;
#endif V4_2BSD

	close (pp[1]);

	do {
	    rc = wait (&status);
	} while (rc != child_pid && (rc != -1 || errno == EINTR));

	if (rc != child_pid) {
	    int err = errno;	/* save errno in case open changes... */
	    mopen (supportaddr, "Problems with `recvprog' program");
	    mprintf ("Failed to collect child's status (pid %d, rc %d, \
error %d).\n\n", child_pid, rc, err);
	    do_msg = 1;
#ifdef WIFSTOPPED
	} else if ((stopped = WIFSTOPPED (status)) || WIFSIGNALED (status)) {
	    mopen (supportaddr, "Problem with `recvprog' program");
	    mprintf ("recvprog was %sed by a signal: %s\n\n",
		    (stopped? "stopp":"kill"), signame (status.w_termsig));
	    do_msg = 1;
	} else if (status.w_retcode != EX_OK && status.w_retcode != EX_ADDR) {
	    int mech = (status.w_retcode == EX_MECH);
#else
	} else if ((status>>8)&0xff != EX_OK && (status>>8)&0xff != EX_ADDR) {
	    int mech = ((status>>8)&0xff == EX_MECH);
#endif WIFSTOPPED
	    mopen (supportaddr, (mech? 
		"recvprog exited with EX_MECH":
		"recvprog exited with unexpected status"));
	    if (!mech) 
		mprintf ("recvprog exited with return/status 0x%x.\n\n", 
							status);
	    do_msg = 1;
	}

	if (do_msg) {
	    FILE *f;
	    mprintf ("The arguments were:\n");
	    while (argc-- != 0) mprintf ("  %s", *argv++);

	    mprintf ("\n\nThe program said the following:\n%s\n", divider);
	    f = fdopen (pp[0], "r");
	    if (f == NULL) 
		mprintf ("[FAILED TO OPEN PIPE]\n"); 
	    else 
		minclude (f);

	    mprintf ("\n%s\n\nThe message follows%s:",
		divider, (fseek (stdin, 0L, 0) < 0) ? 
				" (couldn't rewind the file)":"");
	    mprintf ("\n%s\n", divider);
	    minclude (stdin);
	    exit (mclose () == 0 ? EX_MECH : EX_FAIL);
	}

	if (strcmp (*(argv_old+1), "-D") == 0) {
	    char buff [512]; int n;
	    while ((n = read (pp[0], buff, 512)) > 0) write (1, buff, n);
	}
	exit (status.w_retcode);
    }
}
#endif SAFEFORK		


/* *
 * pass a message into MMDF
 */


#ifdef SAFEFORK
#define MAIN child
#else
#define MAIN main
#endif SAFEFORK


#define HT_NONE 0 /* no host specified */
#define HT_HOST 1 /* host specified by -h */
#define HT_VIA  2 /* host specified by -v */


MAIN (argc, argv)
    int argc; char **argv;
{
    char *channel, *host, *arg;
    struct rp_bufstruct rply; int rply_len;
    int rp, ex = EX_OK, n, hosttype = HT_NONE;
    char buf[BUFSIZ], subargs[32]; 

    errmsg_file = NULL;
    sender = 0;
    prog_mode = '\0';

#ifndef SAFEFORK
    mmdf_init (*argv);
#endif SAFEFORK

    TRY (mm_init (), "initialize MMDF");
    TRY (mm_sbinit (), "initialize submission");

    while (--argc > 0 && *(arg = *++argv) == '-') {
	char opt = *++arg;
	char **where = 0;
	switch (opt) {
	    case 'D': prog_debug = 1;   break;
	    case 'c': where = &channel; break;
	    case 's': where = &sender;  break;

	    case 'h': 
	    case 'v': 
		if (hosttype != HT_NONE) {
		    printf ("%chost specified twice\n", PR_ERR);
		    exit (EX_MECH);
		}
		hosttype = (opt == 'h' ? HT_HOST : HT_VIA);
		where = &host; break;

	    case 'M':
		prog_mode = *arg; break;

	    default:
		printf ("%cunknown argument -%c\n", PR_ERR, *arg);
		exit (EX_MECH);
	}
	if (where != 0) {
	    if (--argc <= 0) {
		printf ("%cno value for -%c\n", PR_ERR, *arg);
		exit (EX_MECH);
	    }
	    *where = *++argv;
	}
    }

    if (prog_mode == 'j' && argc > 0) {		/* JNT mail mode */
	printf ("%c-Mj and recipients", PR_ERR);
	exit (EX_MECH);
    } else if (prog_mode == 's' && argc > 0) {	/* Batch-SMTP mail mode */
	printf ("%c-Ms and recipients", PR_ERR);
	exit (EX_MECH);
    } else if (argc == 0) {
	printf ("%cno recipients\n", PR_ERR);
	exit (EX_MECH);
    }
	    
    strcpy (subargs, "tmlv");
    if (channel) {
    	Chan *curchan;

	if ((curchan = ch_nm2struct (channel)) == (Chan *) NOTOK)
	    err_abrt (RP_PARM, "unknown channel name '%s'", argv[0]);
    	ch_llinit(curchan);
	sprintf (buf, "%si%s*", subargs, channel);
	strcpy (subargs, buf);
    }
    if (hosttype != HT_NONE) {
	sprintf (buf, "%sh%s*", subargs, 
			    (hosttype == HT_VIA ? ap_dmflip (host) : host));
	strcpy (subargs, buf);
    }
    if (!sender) {
	strcat (subargs, "s");
    }

    if (prog_mode == 'j') {
	ex = JNTextract_recipients (&argc, &argv);
	if (ex != EX_OK) {
	    if (errmsg_file != NULL) {
		errmsg_send ();
	    }
	    exit (ex);
	}
    } else if (prog_mode == 's') {
    	/* Batch SMTP - not implemented yet */
    	printf ("%cBatch SMTP not supported yet", PR_ERR);
    	exit(EX_MECH);
    }

#ifdef DEBUG
    if (prog_debug) {
	printf ("%csubargs: %s", PR_COMM, subargs);
	printf ("; sender %s", sender ? sender : "(unspecified)");
	printf ("; %d recipient(s)\n", argc);
    }
#endif

    TRY (mm_winit ((char *) 0, subargs, sender), "initialize for message");
    TRY (mm_rrply (&rply, &rply_len), "get result of mm_winit");

    switch (rp_gbval (rply.rp_val))
    {			  /* was source acceptable?            */
	case RP_BNO:
    	case RP_BTNO:
	    terminate(rply.rp_line, rply.rp_val);
    }
#ifdef DEBUG
    if (prog_debug) {
	printf ("%csubmitting addresses\n", PR_COMM); fflush (stdout);
    }
#endif
    while (argc--) { 
	char *adr = *argv++;
#ifdef DEBUG
	if (prog_debug) {
	    printf ("%c  %s:\t", PR_COMM, adr); fflush (stdout);
	}
#endif DEBUG
	TRY (mm_wadr ((char *) 0, adr), "write address");
	TRY (mm_rrply (&rply, &rply_len), "get result of writing address");
#ifdef DEBUG
	if (prog_debug) {
	    printf ("[%o] %s\n", rply.rp_val & 0377, rply.rp_line); 
	    fflush (stdout);
	}
#endif DEBUG
	switch (rp = rp_gval (rply.rp_val)) {
	    case RP_AOK:
		break;
	    case RP_USER:
		invalid_address (adr, &rply);
		break;
	    default:
		{
		    char buff [128];
		    sprintf (buff, "understand reply to %s:\n%c%s",
						adr, PR_ERR, rply.rp_line);
		    terminate (buff, rp);
		}
	}
    }

    TRY (mm_waend (), "finish writing addresses");

    while (n = fread (buf, sizeof (char), BUFSIZ, stdin)) {
	TRY (mm_wtxt (buf, n), "write text");
    }

    TRY (mm_wtend (), "finish writing text");
    TRY (mm_rrply (&rply, &rply_len), "get result of submission");

    switch (rp = rp_gval (rply.rp_val)) {
	case RP_MOK:
	    break;

	case RP_NDEL:
	    if (errmsg_file == NULL) {
		printf ("%csubmit returned NDEL but no error message\n",
		    PR_ERR);
		exit (EX_MECH);
	    }
	    break;

	default:
	    if (errmsg_file != NULL) {
		printf ("%csending error message\n", PR_COMM);
		errmsg_send ();
	    }
	    terminate ("understand reply from submit", rp);
	    break;
    }

    mm_sbend ();
    mm_end (RP_OK);

    if (errmsg_file != NULL) {
	errmsg_send ();
	ex = EX_ADDR;
    }
    exit (ex);
	
}


/* *
 * reading of addresses from JNT mail file header
 *
 * (Strictly we should be using the MMDF parsing routines in this
 * code, but I'm fed up trying to get the buggers to work!!  This 
 * works more or less...!)
 */

JNTextract_recipients (argc, argv)
    int *argc; char ***argv;
{
    struct alist {
	struct alist *ad_next;
	char ad_addr[1]; /* well, as long as we need */
    } *alist = 0;
    int naddrs = 0;
    char **pp;

    rewindable_msg ();

    for (;;) {
	int ch;
	int lit  = 0; /* in a domain literal */
	int lay  = 0; /* just had layout */
	enum { D_NO, D_ADR, D_END } done = D_NO;
	char buff[ADDRSIZE];
	register char *p;
	struct alist *al;

	p = &buff[0];

	while (done == D_NO) {
	    enum { T_CHAR, T_SEP, T_LAY } type = T_CHAR; /* char type */
	    switch (ch = getchar ()) {
		case '\\' :
		    *p++ = '\\'; ch = getchar (); break;
		case '[' :
		    lit = 1; break;
		case ']' :
		    lit = 0; break;
		case ',' :
		    if (!lit) type = T_SEP; break;
		case '\r' :
		    ch = getchar ();
		    if (ch == '\n') {
			type = T_LAY;
		    } else {
			ungetc (ch, stdin);
			ch = '\r';
		    }
		    break;
		case '\n' :
		    type =T_LAY; break;
		case EOF :
		    reportf ("%cJNT file ended prematurely\n", PR_COMM);
		    return (EX_MECH); 
		default :
		    break;
	    }
	    
	    switch (type) {
		case T_SEP :
		    done = D_ADR; break;
		case T_CHAR :
		    *p++ = ch;
		    lay = 0; 
		    break;
		case T_LAY :
		    if (lay) done = D_END; else lay = 1;
		    break;
	    }
	}

	*p = '\0';
	al = (struct alist *) malloc (
	    sizeof (*al) - sizeof ((*al).ad_addr) + 
	    strlen (buff) + 1);
	al->ad_next = alist; alist = al;
	strcpy (al->ad_addr, buff);
	naddrs++;
#ifdef DEBUG
	if (prog_debug) printf ("%caddress %d: %s%s\n", 
		    PR_COMM, naddrs, buff, (done==D_END? " !":""));
#endif
	if (done == D_END) break;
    }

    msg_start = ftell (stdin);
    
    *argc = naddrs;
    *argv = pp = (char **) malloc (naddrs * sizeof (char *));

    while (naddrs-- > 0) {
	*pp++ = alist->ad_addr; alist = alist->ad_next;
    }
    return (EX_OK);
}

/* *
 * report an invalid address or other problem
 */

    
reportf (f, a0, a1, a2, a3, a4, a5, a6, a7)
    char *f;
{
#ifdef DEBUG
    if (prog_debug) {
	printf (f, a0, a1, a2, a3, a4, a5, a6, a7);
    }
#endif

    if (errmsg_file == NULL) errmsg_open ();
    fprintf (errmsg_file, f, a0, a1, a2, a3, a4, a5, a6, a7);
    fprintf (errmsg_file, "\n");
}

