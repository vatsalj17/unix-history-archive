#include "util.h"
#include "mmdf.h"

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
/*                  SEND FROM DELIVER TO NIFTP CHANNEL
 *
 *      Adapted by Steve Kille from module to communicate with local channel
 *      July 1982
 *
 *      Mar 82  Steve Kille    Modify to use new MMDF - in particular the new
 *                                sub-list structuring
 *
 */

#include "ch.h"
#include "phs.h"
#include "ap.h"

extern struct ll_struct   *logptr;
extern char *qu_msgfile;          /* name of file containing msg text   */
extern Chan *chanptr;

LOCVAR int nadrs;               /* to keep a count of the addresses     */

LOCVAR struct rp_construct
			    rp_adr  =
{
    RP_AOK, 'a', 'd', 'd', 'r', 'e', 's', 's', ' ', 'o', 'k', '\0'
},

			    rp_hend     =
{
    RP_MOK, 'P', ' ', 'N', 'i', 'f', 't', 'p', ' ', 'e', 'n', 'd', ' ',
     'o', 'f', ' ', 'a',  'd', 'd', 'r', ' ', 'l', 'i', 's', 't', '\0'
};

LOCVAR struct rp_construct
			    rp_temp     =
{
    RP_LIO, 'P', ' ', 'N', 'i', 'f', 't', 'p', ',', ' ', 't', 'e', 'm',
     'p', ' ', 'f', 'a', 'i', 'l', 'u', 'r', 'e', '\0'
};

LOCVAR struct rp_construct
			    rp_perm     =
{
    RP_NDEL, 'P', ' ', 'N', 'i', 'f', 't', 'p', ' ',
    'p', 'e', 'r', 'm', 'a', 'n', 'a', 'n', 't', ' ',
    'f', 'a', 'i', 'l', 'u', 'r', 'e', '\0'
};

LOCVAR struct rp_construct
			    rp_bhst     =
{
    RP_BHST, 'B', 'a', 'd', ' ', 'H', 'o', 's', 't', '\0'
};


LOCVAR struct rp_construct
			   rp_noadr     =
{
    RP_NDEL, 'N', 'o', ' ', 'v', 'a', 'l', 'i', 'd', ' ', 'a', 'd', 'd',
    'r', 'e', 's', 's', 's', 'e', 's', '\0'
};

LOCVAR struct rp_construct
			  rp_noop       =
{
    RP_NOOP, 'E', 'n', 'd', ' ', 'o', 'f', ' ', 'h', 'o', 's', 't', ' ',
    'i', 'g', 'n', 'o', 'r', 'e', 'd', '\0'
};

/**/

qu2pn_send (chname)                /* overall mngmt for batch of msgs    */
char    chname [];
{
    short     result;
    char    info[LINESIZE],
	    sender[LINESIZE];

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "qu2pn_send ()");
#endif

    if (rp_isbad (result = qu_pkinit ()))
	return (result);

    if (rp_isbad (result = pn_sbinit ()))
    {
	printx ("problem with niftp submission... ");
	return (result);
    }

    for(;;){                /* get initial info for new message   */
	result = qu_rinit (info, sender, chanptr -> ch_apout);
	if(rp_gval(result) == RP_NS){
	    qu_rend();
	    continue;
	}
	else if(rp_gval(result) != RP_OK)
	    break;
	if (rp_isbad (result =
		    pn_winit (chname, sender, qu_msgfile)))
	    return (result);      /* ready to write message out         */

	if (rp_isbad (result = qu2pn_each ()))
				/* Now process the addresses and text   */
				/* for this message                     */
	    return (result);
	qu_rend();
    }
    qu_rend();

    if (rp_gval (result) != RP_DONE)
    {
	ll_log (logptr, LLOGTMP, "not DONE (%s)", rp_valstr (result));
	return (RP_RPLY);         /* catch protocol errors              */
    }

    qu_pkend ();                  /* done getting messages              */
    if (rp_isbad (result = pn_sbend ()))
				  /* done sending messages              */
    {
	printx ("bad ending for NIFTP delivery... ");
	fflush (stdout);
    }

    return (result);
}
/**/


LOCFUN
	qu2pn_each ()             /* send copy of text per address      */
{
    short     result;
    short     txt_result;
    char    host[LINESIZE];
    char    adr[LINESIZE];

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "qu2sm_each()");
#endif

    phs_note (chanptr, PHS_WRSTRT);

    nadrs = 0;
    if (rp_isbad (result = pn_wainit()))
	return (result);

    FOREVER                     /* iterate thru list                    */
    {
	result = qu_radr (host, adr);
	if (rp_isbad (result))
	    return (result);      /* get address from Deliver           */

/*  check for end of address sub-list.  different
 *  sub-lists reference different hosts to connect to.
 */
	switch (rp_gval (result))
	{
	    case RP_HOK:          /* end of sub-list */
		if (chanptr -> ch_host != NORELAY)
		{
				/* send all addresses to relay at once */
		    qu_wrply ((struct rp_bufstruct *) &rp_noop,
						rp_conlen (rp_noop));
		    continue;
		}
	    case RP_DONE:         /* end of list so fire it off */
#ifdef DEBUG
	ll_log (logptr, LLOGFTR, "End of list or sublist (%s)",
			rp_valstr (result));
#endif

		if (nadrs == 0)
		{
		    pn_wtend (RP_NO);  /* terminate current message    */
		    qu_wrply ((struct rp_bufstruct *) &rp_noadr,
				sizeof rp_noadr);
					/* assume strange temp error    */
		}
		else
		{
		    nadrs = 0;      /* reset, in case this is only a hend */
		    if (rp_isbad (txt_result = qu2pn_txtcpy ()))
		    {
					/* Tell other side of problem    */
			if (rp_gbval (txt_result) == RP_BTNO)
					/*  temp error                  */
			    qu_wrply ((struct rp_bufstruct *) &rp_temp,
				rp_conlen (rp_temp));
			else
			    qu_wrply ((struct rp_bufstruct *) &rp_perm,
				rp_conlen (rp_perm));
					/* no reason to abort though    */
		    }
		    else
			qu_wrply ((struct rp_bufstruct *) &rp_hend,
				rp_conlen (rp_hend));
		}
		if (rp_gval (result) == RP_DONE)
		    return (RP_OK);
				/* end of list - so return          */
				/* otherwise sort prepare for another one */
		if (rp_isbad (result = pn_wainit ()))
		    return (result);
		continue;

	    default:            /* actually have an address */
		if (rp_isbad (result = pn_wadr (host, adr)))
		{
		    pn_wtend (result);  /* clean up and get out         */
		    return (result);
		}

		nadrs++;

		qu_wrply ((struct rp_bufstruct *) &rp_adr, rp_conlen (rp_adr));
		continue;
	}
    }
    /* NOTREACHED */
}

/**/

LOCFUN
	qu2pn_txtcpy ()             /* copy the text of the message       */
{
    short   result;
    int     len;
    char    buffer[BUFSIZ];

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "qu2pn_txtcpy()");
#endif

    phs_note (chanptr, PHS_WRMSG);

    if (rp_isbad (result = pn_wtinit ()))
	return (result);

    qu_rtinit (0L);              /* ready to read the text             */
    len = sizeof(buffer);
    while ((rp_gval (result = qu_rtxt (buffer, &len))) == RP_OK)
    {                             /* send the text                      */
	if (rp_isbad (result = pn_wtxt (buffer, len)))
	    break;
	if (rp_gval (result) != RP_OK)
	{
	    result = RP_RPLY;
	    break;
	}
	len = sizeof(buffer);
    }


    if (rp_isgood (result) && rp_gval (result) != RP_DONE)
	result = RP_RPLY;        /* didn't get it all across?          */

    if (rp_isbad (result = pn_wtend (result)))
	return (result);          /* flag end of message                */
				  /* and then it can be sent off        */

    phs_note (chanptr, PHS_WREND);

    return (RP_MOK);               /* got the text out                   */
}
