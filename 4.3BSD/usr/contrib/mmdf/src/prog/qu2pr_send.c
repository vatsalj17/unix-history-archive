/*
 *	MULTI-CHANNEL MEMO DISTRIBUTION FACILITY  (MMDF)
 *
 *	Department of Electrical Engineering
 *	University of Delaware
 *	Newark, Delaware  19711
 *
 *
 *	Program Channel: Take message and feed a request to a program
 *
 *
 *	Q U 2 P R _ S E N D . C
 *	========================
 *
 *	?
 *
 *	J.B.D.Pardoe
 *	University of Cambridge Computer Laboratory
 *	October 1985
 *	
 *	based on the UUCP channel by Doug Kingston (US Army Ballistics 
 *	Research Lab, Aberdeen, Maryland: <dpk@brl>)
 *
 */

#include "util.h"
#include "mmdf.h"
#include "ch.h"
#include <signal.h>
#include "ap.h"
#include "phs.h"

extern	char	*strdup();
extern	char	*ap_p2s();
extern	char	*multcat();

extern struct ll_struct   *logptr;
extern char *qu_msgfile;          /* name of file containing msg text   */
extern Chan      *chan;  /* Who we are */


int pipebroken; /* set if SIGPIPE occurs */

LOCVAR struct rp_construct
	rp_aend =
{
    RP_OK, 'p', 'r', 'o', 'g', ' ', 'e', 'n', 'd', ' ', 'o', 'f', ' ', 'a',
    'd', 'd', 'r', ' ', 'l', 'i', 's', 't', '\0'
},
	rp_mok =
{
    RP_MOK, 'm', 'o', 'k', '\0'
},
	rp_noop =
{
    RP_NOOP, 's', 'u', 'b', '-', 'l', 'i', 's', ' ', 'n', 'o', 't', ' ',
    's', 'p', 'e', 'c', 'i', 'a', 'l', '\0'
},
	rp_err =
{
    RP_NO, 'u', 'n', 'k', 'n', 'o', 'w', 'n', ' ', 'e', 'r', 'r', 'o', 'r', '\0'
},
	rp_pipe =
{
    RP_NET, 'p', 'r', 'o', 'g', 'r', 'a', 'm', ' ', 'f', 'a', 'i', 'l', 'e', 'd', '\0'
},
	rp_bhost =
{
    RP_USER, 'b', 'a', 'd', ' ', 'h', 'o', 's', 't', ' ', 'n', 'a',
    'm', 'e', '\0'
};


/* 
 * qu2pr_send : overall management for batch of msgs
 * ==========
 */
qu2pr_send ()
{
    short rp;
    char  info[LINESIZE], sender[ADDRSIZE];

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "qu2pr_send ()");
#endif

    if (rp_isbad (rp = qu_pkinit ()))
	return (rp);

    /*
     *  AP_SAME == NO header munging.  We need to read the header
     *  ourselves and the munging just gets in the way.
     */
    while (rp_gval ((rp = qu_rinit (info, sender, chan->ch_apout))) != RP_DONE)
    {                             /* get initial info for new message   */
	if (rp_gval (rp) == RP_FIO)          /* Can't open message file */
		continue;
	else if (rp_gval (rp) != RP_OK)      /* Some other error */
		break;
	phs_note (chan, PHS_WRSTRT);

	if (rp_isbad (rp = qu2pr_each (sender))) return (rp);
	qu_rend ();
    }
    qu_rend ();

    if (rp_gval (rp) != RP_DONE)
    {
	ll_log (logptr, LLOGTMP, "not DONE (%s)", rp_valstr (rp));
	return (RP_RPLY);         /* catch protocol errors              */
    }

    qu_pkend ();
    phs_note (chan, PHS_WREND);

    return (rp);
}

/**/
/*
 * qu2pr_each : execute command for one address
 * ==========
 */

static qu2pr_each (sender)
    char *sender;
{
    RP_Buf    rply;
    short     rp;
    char      host [ADDRSIZE];
    char      adr  [ADDRSIZE];
    extern    brpipe ();
    int       (*oldsig) ();

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "qu2pr_each()");
#endif

    for (;;) /* ..all addresses */
    {                        
	rp = qu_radr (host, adr);
	if (rp_isbad (rp)) return (rp);

	if (rp_gval (rp) == RP_HOK) {
	    qu_wrply ((RP_Buf *) &rp_noop, sizeof rp_noop);
	    continue;
	} else if (rp_gval (rp) == RP_DONE) {
	    qu_wrply ((RP_Buf *) &rp_aend, sizeof rp_aend);
	    return (RP_OK); /* end of address list */
	}

	pipebroken = 0;
	oldsig = signal (SIGPIPE, brpipe);

	rply.rp_val = pr_wtadr (host, adr, sender);
	switch (rply.rp_val) {
	    case RP_AOK:
	    case RP_OK:
#ifdef DEBUG
		ll_log (logptr, LLOGFTR, "Calling txtcpy()\n");
#endif
		rply.rp_val = pr_txtcpy();
		break;

	    case RP_USER:
		ll_log (logptr, LLOGFAT, "host (%s) not in table", host);
		blt(&rp_bhost, (char *) &rply, sizeof rp_bhost);
		break;

	    default:
		ll_log (logptr,LLOGFAT,"unknown error (0%o)", rply.rp_val);
		blt(&rp_err, (char *) &rply, sizeof rp_err);
		rply.rp_val = RP_NO;
	}
	if (rply.rp_val != RP_MOK) {
	    qu_wrply (&rply, sizeof(rply.rp_val) + strlen(rply.rp_line));
	} else {
	    rply.rp_val = pr_wttend ();
	    switch (rply.rp_val) {
		case RP_AOK:
		case RP_OK:
		case RP_MOK:
		    qu_wrply (&rp_mok, sizeof rp_mok);    
		    break;

		case RP_USER:
		    ll_log (logptr, LLOGFAT, "host (%s) not in table", host);
		    qu_wrply (&rp_bhost, sizeof rp_bhost);
		    break;

		case RP_LIO:
		    ll_log (logptr,LLOGTMP,"prx pipe broke");
		    qu_wrply (&rp_pipe, sizeof rp_pipe);
		    break;

		default:
		    ll_log (logptr,LLOGFAT,"unknown error on close (0%o)", rply.rp_val);
		    qu_wrply (&rp_err, sizeof rp_err);
	    }
	}
	signal (SIGPIPE, oldsig);
    }
}


brpipe ()
{
    pipebroken = 1;
    signal (SIGPIPE, SIG_IGN);
}
