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
/*                SEND FROM DELIVER TO ARPANET MACHINE
 *
 *  Jun 81  D. Crocker    Officially added SRI's header-transform code
 *                        Removed MLFL-related code
 *                        Add HDRMOD conditional compilation for code
 *                        that transforms offnet addresses
 *                        Moved unlink just after reopen, in d2a_send()
 *  Jul 81  D. Crocker    Only call am_cend if txtcpy was successful
 *  Aug 81  D. Crocker    d2a_send() had fclose's w/out fp validity test
 *
 *      -----------------------------------------------------------
 *
 *  Feb 83  Doug Kingston       *** Total Rewrite, Some Fragments Used ***
 *  Nov 83  Steve Kille         Add throughput and UCL changes
 */

#include "util.h"
#include "mmdf.h"
#include "ch.h"
#include "ap.h"
#include "smtp.h"

extern struct ll_struct   *logptr;
extern Chan *chanptr;
extern char *supportaddr;

extern char *sm_curname;

extern long qu_msglen;
extern long sm_hostid();
extern struct sm_rstruct sm_rp;
extern char *strdup();
extern char *strncpy();
extern char *ap_p2s();
extern char *blt();
extern time_t time();

extern int ap_outtype;

LOCVAR time_t start_time;
LOCVAR short sm_fromsent;
LOCVAR int nadrs;
LOCVAR char *sender = (char *)NULL;

LOCVAR struct rp_construct
	rp_bdrem =
{
    RP_BHST, 'B', 'a', 'd', ' ', 'r', 'e', 's', 'p', 'o', 'n', 's', 'e',
    ' ', 'f', 'r', 'o', 'm', ' ', 'r', 'e', 'm', 'o', 't', 'e', ' ',
    's', 'i', 't', 'e', '\0'
}          ,
	rp_adr  =
{
    RP_AOK, 'a', 'd', 'd', 'r', 'e', 's', 's', ' ', 'o', 'k', '\0'
},
	rp_gdtxt =
{
    RP_MOK, 't', 'e', 'x', 't', ' ', 's', 'e', 'n', 't', ' ', 'o', 'k', '\0'
},
	rp_noop =
{
    RP_NOOP, 's', 'u', 'b', '-', 'l', 'i', 's', ' ', 'n', 'o', 't', ' ',
    's', 'p', 'e', 'c', 'i', 'a', 'l', '\0'
},
	rp_dhst =
{
    RP_DHST, 'R', 'e', 'm', 'o', 't', 'e', ' ', 'h', 'o', 's', 't', ' ',
    'u', 'n', 'a', 'v', 'a', 'i', 'l', 'a', 'b', 'l', 'e', '\0'
},
	rp_kild =
{
    RP_DHST, 'n', 'o', ' ', 'v', 'a', 'l', 'i', 'd', ' ', 'a', 'd', 'd',
    'r', 'e', 's', 's', 'e', 's', '\0'
};

/**/

qu2sm_send ()             /* overall mngmt for batch of msgs    */
{
    short   result;
    char    info[LINESIZE];
    char    sendbuf[ADDRSIZE];

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "qu2sm_send ()");
#endif

    if (rp_isbad (result = qu_pkinit ()))
	return (result);

    sm_curname = (char *) 0;

    /*
     *  While there are messages to process ...
     */
    for(;;){
	AP_ptr  loctree, domtree, routree, sendtree;
	AP_ptr  ap_s2tree(), ap_normalize();

	result = qu_rinit (info, sendbuf, chanptr -> ch_apout);
	if(rp_gval(result) == RP_NS){
	    qu_rend();
	    continue;
	}
	else if(rp_gval(result) == RP_DONE)
	    break;
	if (rp_gval(result) == RP_FIO)          /* Can't open message file */
		continue;
	else if (rp_gval(result) != RP_OK)      /* Some other error */
		break;

#ifdef DEBUG
	ll_log (logptr, LLOGBTR, "from qu_rinit (%s)", sender);
#endif
	if (sender != (char *)NULL)
		free (sender);

	if ( sendbuf[0] == '\0' ||
	    (sendtree = ap_s2tree( sendbuf )) == (AP_ptr) NOTOK) {
		printx("return address unparseable, using Orphanage\n");
		fflush(stdout);
		sender = strdup (supportaddr);
	} else {
		sendtree = ap_normalize (chanptr -> ch_lname,
				chanptr  -> ch_ldomain, sendtree, chanptr);
		if(sendtree == (AP_ptr)MAYBE){
		    result = RP_NS;
		    goto bugout;
		}
		ap_t2parts (sendtree, (AP_ptr *)0, (AP_ptr *)0, &loctree, &domtree, &routree);
		sender = ap_p2s ((AP_ptr)0, (AP_ptr)0, loctree, domtree, routree);
		if(sender == (char *)MAYBE){
		    result = RP_NS;
		    goto bugout;
		}
		ap_outtype = chanptr -> ch_apout;
		ap_sqdelete( sendtree, (AP_ptr) 0 );
		ap_free( sendtree );
	}

	if (rp_isbad (result = qu2sm_each ()))
	    break;

	qu_rend();              /* Cleans up after header conversion... */
    }
    qu_rend();

    if (sm_curname != (char *) 0)
	sm_nclose (OK);

    if (rp_gval(result) != RP_DONE)
    {
	ll_log (logptr, LLOGFAT, "not DONE (%s)", rp_valstr (result));
	return (RP_RPLY);         /* catch protocol errors              */
    }

    qu_pkend ();                  /* done getting messages              */
    return (result);

bugout:
    qu_rend();
    if (sm_curname != (char *) 0)
	sm_nclose (OK);
    qu_pkend ();                  /* done getting messages              */
    return (result);
}
/**/

LOCFUN
	qu2sm_each ()             /* send copy of text per address      */
{
    RP_Buf  thereply;
    short   result;
    short   len;
    char    host[ADDRSIZE];
    char    adr[ADDRSIZE];
    char     *newname;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "qu2sm_each()");
#endif

    time (&start_time);

    sm_fromsent = FALSE;
    nadrs = 0;

    /*
     *  For every address in the message ...
     */
    while( 1 ) {
	result = qu_radr (host, adr);
	if (rp_isbad (result))
	    return (result);      /* get address from Deliver           */

/*  check for end of address sub-list.  if channel is relay, then
 *  sub-list reference is not meaningful.  otherwise, different
 *  sub-lists reference different hosts to connect to.
 */
	switch (rp_gval (result))
	{
	    case RP_HOK:          /* end of sub-list */
		if (chanptr -> ch_host != NORELAY)
		{                 /* different host refs => diff dests  */
		    qu_wrply ((RP_Buf *) &rp_noop, sizeof rp_noop);
		    break;
		}                 /* else DROP ON THROUGH */
	    case RP_DONE:         /* end of full address list           */
		if (nadrs == 0)
		{
		    /*
		     *  Theoretically we would break the connection here since
		     *  the next host is a different host, except that it may
		     *  be flaged and the host after that might be this one.
		     *  Basic rule: don't break a connection until you have to.
		     */
		    qu_wrply ((RP_Buf *) &rp_kild, sizeof rp_kild);

		    if (sm_curname != (char *) 0)
		    {
			if (rp_isbad (sm_cmd ("RSET", SM_RTIME)))
			    break;
/********* should be:   if (sm_rp.sm_rval != 250) *******/
			if (sm_rp.sm_rval < 200 || sm_rp.sm_rval > 299)
			    sm_nclose( NOTOK ); /* Give up on it for now */
		    }
		}
		else if (sm_curname != (char *) 0)
		{
		    nadrs = 0;      /* reset, in case this is only a hend */
		    qu2sm_txtcpy(&thereply);
		    ll_log(logptr, LLOGFTR, "txtcpy reply (%o) (%s)",
			    thereply.rp_val, thereply.rp_line);
		    qu_wrply (&thereply, sizeof (thereply.rp_val)
			      + strlen(thereply.rp_line));
		} else {
		    /* Some addrs were send, but connection went bad */
		    qu_wrply((RP_Buf *)&rp_bdrem, sizeof rp_bdrem);
		}
		sm_fromsent = FALSE;
		if (rp_gval (result) == RP_HOK)
		    continue;           /* more to process  */

		return (RP_OK);         /* END for this message */

	    default:            /* actually have an address */
		if(( newname =  (chanptr->ch_host == NORELAY ?
		   host : chanptr->ch_host)) == (char *) 0)
		{
		    qu_wrply ((RP_Buf *)&rp_dhst, sizeof rp_dhst);
		    break;
		}

		if (!lexequ (sm_curname, newname))
		{
		    if (sm_curname != (char *) 0)
			sm_nclose (OK);

		    if( rp_isbad(sm_nopen( newname)))
		    {
			qu_wrply((RP_Buf *)&rp_dhst, sizeof rp_dhst);
			break;
		    }

		}

		if( sm_fromsent == FALSE ) {
		    if (rp_isbad (sm_wfrom (sender))) {
			if (rp_isbad (sm_rpcpy(&thereply, &len)))
			    qu_wrply((RP_Buf *)&rp_bdrem, sizeof rp_bdrem);
			else
			    qu_wrply (&thereply, len);
			break;
		    }
		    sm_fromsent = TRUE;
		}

		if (sm_wto (host, adr) == RP_DHST
		  || rp_isbad (sm_rpcpy (&thereply, &len))) {
		    qu_wrply ((RP_Buf *) &rp_bdrem, sizeof rp_bdrem);
		    break;
		}
		switch (rp_gval (thereply.rp_val))
		{                 /* was address acceptable?            */
		    case RP_AOK:
			nadrs++;
			qu_wrply ((RP_Buf *) &rp_adr, sizeof rp_adr);
			continue;

		    case RP_PARM:
		    case RP_USER:
		    case RP_NO:
			break;    /* report failure and continue        */

		    case RP_RPLY:
			ll_log (logptr, LLOGFAT, "unusual return: (%s)%s",
				rp_valstr (thereply.rp_val), thereply.rp_line);
			break;    /* notify deliver */
		}

		qu_wrply (&thereply, len);
				  /* notify deliver */
	}
    }
    /* NOTREACHED */
}
/**/

LOCFUN
	qu2sm_txtcpy (rp)     /* copy the text of the message       */
RP_Buf *rp;
{
    int       len;
    short     result;
    char      buffer[BUFSIZ];
    long      bytecount;
    time_t    start_txt,
		end_txt,
		time_eff,
		time_txt;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "qu2sm_txtcpy()");
#endif

    qu_rtinit (0L);
    sm_fromsent = FALSE;
    nadrs = 0;
    if (rp_isbad (sm_cmd ("DATA", SM_DTIME))) {
	blt ((char *)&rp_bdrem, (char *) rp, sizeof rp_bdrem);
	return;
    }

    switch( sm_rp.sm_rval ) {
    case 354:
	break;          /* Go ahead and send mail */

    case 500:
    case 501:
    case 503:
    case 554:
	rp->rp_val = RP_NDEL;
	if (sm_rp.sm_rgot)
		strncpy (rp->rp_line, sm_rp.sm_rstr, sm_rp.sm_rlen);
	else
		strcpy (rp->rp_line, "Unknown Problem");
	return;

    case 421:
    case 451:
    default:
	rp->rp_val = RP_AGN;
	if (sm_rp.sm_rgot)
		strncpy (rp->rp_line, sm_rp.sm_rstr, sm_rp.sm_rlen);
	else
		strcpy (rp->rp_line, "Unknown Problem");
	return;
    }

    printx ("sending...:");
    fflush (stdout);

    bytecount = 0;
    time (&start_txt);

    len = sizeof(buffer);
    while ((rp_gval (result = qu_rtxt (buffer, &len))) == RP_OK)
    {
	if (setjmp(timerest)) {
	    break;
	}
	s_alarm (SM_BTIME);
	result = sm_wstm (buffer, len);
	s_alarm (0);
	bytecount = bytecount + len;
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

    bytecount = bytecount +  len;

    if (rp_isbad (result) || rp_gval (result) != RP_DONE) {
	blt ((char *)&rp_bdrem, (char *) rp, sizeof rp_bdrem);
	return;
    }

    /* Kludge: the sm_wstm function will make sure the last char is a NL */
    if (setjmp(timerest)) {
	blt ((char *)&rp_bdrem, (char *) rp, sizeof rp_bdrem);
	return;
    }
    s_alarm (SM_TTIME);
    result = sm_wstm ((char *)0, 0);    /* MAGIC options */
    s_alarm( 0 );
    if (rp_isbad (result)) {
	blt ((char *)&rp_bdrem, (char *) rp, sizeof rp_bdrem);
	return;
    }

    printx (" ");
    if( rp_isbad( sm_cmd( ".", SM_PTIME + (3*nadrs) ))) {
	rp->rp_val = RP_BHST;
	strcpy (rp->rp_line, "Bad response to final dot");
	return;
    }

    time (&end_txt);
    time_txt = end_txt - start_txt;
    time_eff = end_txt - start_time;

    ll_log (logptr, LLOGFST,
		"tpt: data = (%ld) bytes/sec, eff =  (%ld) bytes/sec",
		bytecount / (time_txt ? time_txt : 1), 
		bytecount / (time_eff ? time_eff : 1));


    time (&start_time);

    switch( sm_rp.sm_rval ) {
    case 250:
    case 251:
	blt ((char *)&rp_gdtxt, (char *)rp, sizeof rp_gdtxt);
	return;

    case 552:
    case 554:
	rp->rp_val = RP_NDEL;
	if (sm_rp.sm_rgot)
		strncpy (rp->rp_line, sm_rp.sm_rstr, sm_rp.sm_rlen);
	else
		strcpy (rp->rp_line, "Unknown Problem");
	return;

    case 421:
    case 451:
    case 452:
    default:
	rp->rp_val = RP_AGN;
	if (sm_rp.sm_rgot)
		strncpy (rp->rp_line, sm_rp.sm_rstr, sm_rp.sm_rlen);
	else
		strcpy (rp->rp_line, "Unknown Problem");
	return;
    }
}
