#
/*
 *     MULTI-CHANNEL MEMO DISTRIBUTION FACILITY  (MMDF)
 *
 *     Copyright (C) 1979,1980,1981  University of Delaware
 *
 *     Department of Electrical Engineering
 *     University of Delaware
 *     Newark, Delaware  19711
 *
 *     Phone:  (302) 738-1163
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
/*                SEND FROM DELIVER TO SUBMIT
 *
 *  Feb 83  Doug Kingston       Initial version of list processor
 *  Nov 85  Phil Cockcroft      Hacked into a delay channel
 *  Apr 86  Craig Partridge     Diddled for efficiency reasons
 */

#include "util.h"
#include "mmdf.h"
#include "phs.h"
#include "ch.h"
#include "ap.h"
#include "ns.h"

extern struct ll_struct   *logptr;
extern Chan *chanptr;

extern char *rindex();
extern char *strdup();
extern char *multcat();
extern char *blt();

LOCVAR int nadrs;
LOCVAR char sender[ADDRSIZE];

LOCVAR struct rp_construct
	rp_bdrem =
{
    RP_BHST, 'B', 'a', 'd', ' ', 'r', 'e', 's', 'p', 'o', 'n', 's', 'e',
    ' ', 'f', 'r', 'o', 'm', ' ', 's', 'u', 'b', 'm', 'i', 't', '\0'
},
	rp_adr =
{
    RP_AOK, 'a', 'd', 'd', 'r', 'e', 's', 's', ' ', 'o', 'k', '\0'
},
	rp_gdtxt =
{
    RP_MOK, 't', 'e', 'x', 't', ' ', 's', 'e', 'n', 't', ' ', 'o', 'k', '\0'
},
	rp_nadr =
{
    RP_BTNO, 'N', 'o', ' ', 'v', 'a', 'l', 'i', 'd', ' ', 
    'a', 'd', 'r', 'e', 's', 's', 'e', 's', '\0'
},
	rp_noop =
{
    RP_NOOP, 's', 'u', 'b', '-', 'l', 'i', 's', ' ', 'n', 'o', 't', ' ',
    's', 'p', 'e', 'c', 'i', 'a', 'l', '\0'
};

/**/

qu2ds_send ()             /* overall mngmt for batch of msgs    */
{
    int		started = FALSE;
    short       result;
    char        info[LINESIZE];

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "qu2ds_send ()");
#endif

    if (rp_isbad (result = qu_pkinit ()))
	return (result);

    /*
     *  While there are messages to process ...
     */
    for(;;){                /* get initial info for new message   */
	result = qu_rinit (info, sender, chanptr -> ch_apout);

	switch (rp_gval(result)) {

	    case RP_NS:		/* keep plugging */
	    case RP_FIO:	/* couldn't open this message file */
		qu_rend();
		continue;

	    case RP_OK:
		break;

	    default:
		goto done;
	}

	/* make sure we have a submit there */
	if ((!started) && (rp_isbad(mm_init())  || rp_isbad(mm_sbinit()))) {
	    /* do we want to tell deliver what happened? */
	    goto done;
	}
	started = TRUE;
	    
	/* now send message */
	switch(rp_gval(result = qu2ds_each ())) {

	    case RP_OK:		/* good! */
	    case RP_MECH:	/* bad queue file */
		break;

	    case RP_BTNO:	/* submit was cranky */
		started = FALSE;
		break;

	    case RP_BNO:	/* deliver hassled us */
	    default:		/* something worse happened */
		goto done;
	}

	qu_rend();
    }

done:
    qu_rend();

    if (rp_gval (result) != RP_DONE)
    {
	ll_log (logptr, LLOGFAT, "not DONE (%s)", rp_valstr (result));
	return (RP_RPLY);         /* catch protocol errors              */
    }

    /* shut down submit nicely */
    if (started) {
	mm_sbend();
	mm_end(OK);
    }


    qu_pkend ();                  /* done getting messages              */
    return (result);
}
/**/

LOCFUN
	qu2ds_each ()             /* send copy of text per address      */
{
    struct rp_bufstruct thereply;
    short   result;
    int     len;
    char    host[ADDRSIZE];
    char    adr[ADDRSIZE];
    char    *dchan, *dhost;
    char    info[ADDRSIZE];
    char    *index();
    int     ndone = 0;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "qu2ds_each()");
#endif

    nadrs = 0;

    /*
     *  For every address in the message ...
     */
    while( 1 ) {
	result = qu_radr (host, adr);
	if (rp_isbad (result))
	    return (RP_BNO);      /* deliver goofed */

	switch (rp_gval (result))
	{
	    case RP_HOK:          /* end of sub-list */
		qu_wrply ((RP_Buf *)&rp_noop, sizeof rp_noop);
		break;

	    case RP_DONE:         /* end of full address list           */
		/* only send text if got some addresses done */
		if(ndone != 0)
		    qu2ds_txtcpy(&thereply);
		else 
		    blt((char *)&rp_nadr, (char *)&thereply, sizeof(rp_nadr));

		qu_wrply(&thereply, sizeof (thereply.rp_val)
					+ strlen (thereply.rp_line));

		if(ndone == 0 || rp_isbad(thereply.rp_val)) {
		    mm_end(NOTOK);
		    return(RP_BTNO);
		}
		return (RP_OK);         /* END for this message */

	    default:            /* actually have an address */

		if (nadrs++ == 0)
		{
		    if((dhost = index(host, '&')) != NULL){
			*dhost++ = 0;
			dchan = host;
			if(*dchan == '\0'){ /* no channel must be local */
			    dchan = 0;
			    sprintf(info, "vmjlk%d*",NS_DELTIME);
			}
			else if(*dhost == '\0')
			    sprintf(info, "vmjlk%d*",NS_DELTIME);
			else
			{
			    /* reset the host as before
			    * dchan is already set */
			    sprintf(info, "vmjlh%s*k%d", dhost,NS_DELTIME);
			}

		    if (domsg)
			strcat(info,"W");
#ifdef DEBUG
		    ll_log (logptr, LLOGBTR, "d2s dchan = '%s' info = '%s'",
				dchan ? dchan : "NULL", info);
#endif
		    } else {
#ifdef DEBUG
			ll_log (logptr, LLOGFAT, "Missing '&' in delay");
#endif
			return(RP_MECH);
		    }

		    /* tell submit about this message */
		    if (rp_isbad (mm_winit (dchan, info, sender))
			  || rp_isbad (mm_rrply(&thereply, &len))) {
			mm_end (NOTOK);
			printx ("Error in submit startup\n");
			fflush (stdout);
			qu_wrply((RP_Buf *)&rp_bdrem, sizeof rp_bdrem);
			return(RP_BTNO);
		    }

		    switch (rp_gbval (thereply.rp_val)) {
		    case RP_BNO:
		    case RP_BTNO:
			mm_end (NOTOK);
			printx ("Error in submit startup (%s)\n", thereply.rp_line);
			fflush (stdout);
			qu_wrply(&thereply, len);
			return(RP_BTNO);

		    default:
			break;
		    }

		    if (rp_isbad (mm_wadr ("", adr))
			  || rp_isbad (mm_rrply(&thereply, &len))) {
			mm_end (NOTOK);
			printx ("Error in passing address\n");
			fflush (stdout);
			qu_wrply((RP_Buf *)&rp_bdrem, sizeof rp_bdrem);
			return(RP_BTNO);
		    }
		} /* naddrs = 0 */
		else
		{
		    if (rp_isbad (mm_wadr ("", adr))
			   || rp_isbad (mm_rrply(&thereply, &len)))
		    {
			mm_end (NOTOK);
			printx ("Error in passing address\n");
			fflush (stdout);
			qu_wrply((RP_Buf *)&rp_bdrem, sizeof rp_bdrem);
			return(RP_BTNO);
		    }
		}  /* naddrs != 0 */
	} /* switch */

	switch (rp_gval (thereply.rp_val))
	{                 /* was address acceptable?            */
	    case RP_AOK:
		qu_wrply((RP_Buf *)&rp_adr, sizeof  rp_adr);
		ndone++;
		continue;

	    case RP_NAUTH:	/* errg !! */
		thereply.rp_val = RP_USER;
		break;

	    case RP_PARM:
	    case RP_USER:
	    case RP_NO:
	    case RP_NS:
		break;    /* report failure and continue        */

	    case RP_RPLY:
		ll_log (logptr, LLOGTMP, "unusual return: (%s)%s",
			rp_valstr (thereply.rp_val), thereply.rp_line);
		break;
	}
	/* tell deliver about address */
	qu_wrply (&thereply, len);
    }
}

/**/

LOCFUN
	qu2ds_txtcpy (rp)     /* copy the text of the message       */
RP_Buf *rp;
{
    int       len;
    short     result;
    char      buffer[BUFSIZ];

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "qu2ds_txtcpy()");
#endif

    qu_rtinit (0L);

    if (rp_isbad (result = mm_waend())) {
	blt ((char *)&rp_bdrem, (char *) rp, sizeof rp_bdrem);
	return;
    }

    printx ("sending...:");
    fflush (stdout);

    len = sizeof(buffer);
    while ((rp_gval (result = qu_rtxt (buffer, &len))) == RP_OK)
    {
	result = mm_wtxt (buffer, len);
	if (rp_isbad (result))
	    break;
	printx (".");
	fflush (stdout);
	if (rp_gval (result) != RP_OK) {
	    blt ((char *)&rp_bdrem, (char *) rp, sizeof rp_bdrem);
	    return;
	}
    	len = sizeof(buffer);
    }

    if (rp_isbad (result) || rp_gval (result) != RP_DONE) {
	blt ((char *)&rp_bdrem, (char *) rp, sizeof rp_bdrem);
	return;
    }

    if (rp_isbad(mm_wtend()) || rp_isbad(mm_rrply(rp,&len))) {
	rp->rp_val = RP_RPLY;
	strcpy (rp->rp_line, "Unknown Problem");
	return;
    }
    
    blt ((char *)&rp_gdtxt, (char *)rp, sizeof rp_gdtxt);
}
