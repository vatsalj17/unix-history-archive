/*
 *  Copyright University College London - 1984
 *
 *  Channel to submit EAN Mail
 *
 *  Steve Kille - November 1984
 */
#include "util.h"
#include "mmdf.h"
#include "phs.h"
#include "ap.h"
#include "ch.h"
#include <pwd.h>
#include <sys/stat.h>
#include "adr_queue.h"

extern Chan     *chanptr;
extern LLog     *logptr;

extern int      errno;
extern int      ap_outtype;

extern char *strcpy ();
extern char *index();
extern char *rindex();
extern char *ap_p2s();

LOCVAR char en_info[LINESIZE];
LOCVAR char en_sender[LINESIZE];
LOCVAR char en_adr[LINESIZE];

LOCVAR int gotone;

LOCVAR struct rp_construct
	rp_adr  =
{
    RP_AOK, 'a', 'd', 'd', 'r', 'e', 's', 's', ' ', 'o', 'k', '\0'
},
	rp_gdtxt =
{
    RP_MOK, 't', 'e', 'x', 't', ' ', 's', 'e', 'n', 't', ' ', 'o', 'k', '\0'
},
	rp_lio =
{
    RP_LIO, 'l', 'o', 'c', 'a', 'l', ' ', 'g', 'l', 'i', 't', 'c', 'h', '\0'
}          ,
	rp_bdtxt =
{
   RP_NDEL, 't', 'e', 'x', 't', ' ', 'c', 'o', 'p', 'y', ' ', 'f', 'a', 'i',
	'l', 'u', 'r', 'e', ' ', 't', 'o', ' ', 'E', 'A', 'N', '\0'
},
rp_hend  =
{
	RP_NOOP, 'e', 'n', 'd', ' ', 'o', 'f', ' ', 'h', 'o', 's', 't', ' ',
	'i', 'g', 'n', 'o', 'r', 'e', 'd', '\0'
};

/**/

qu2en_send ()                       /* overall mngmt for batch of msgs    */
{
	short     result;
	AP_ptr ap;
	AP_ptr local,
		domain,
		route;
	char *sender;
	char *mydomain;

#ifdef DEBUG
	ll_log (logptr, LLOGBTR, "qu2en_send ()");
#endif

	if (rp_isbad (result = qu_pkinit ()))
		return (result);

	/* While there are messages to process... */
	for(;;){
		result = qu_rinit (en_info, en_sender, chanptr -> ch_apout);
		if(rp_gval(result) == RP_NS){
		    /* qu_rend(); */
		    continue;
		}
		if(rp_gval(result) != RP_OK)
		    break;
		ap_outtype = AP_733;
		ap = ap_s2tree (en_sender);
		if (ap != (AP_ptr) NOTOK)
		{
			ap_t2parts (ap, (AP_ptr *) 0, (AP_ptr *) 0,
				&local, &domain, &route);
			sender = ap_p2s ((AP_ptr) 0, (AP_ptr) 0,
				local, domain, route);
			if(sender == (char *)MAYBE)
			    return (RP_NS);
			ap_sqdelete (ap);
			ap_free (ap);
		}
		else
		{
			ll_log (logptr,  "Duff sender '%s'", en_sender);
			return (RP_NO);
		}

		ll_log (logptr, LLOGFST, "Message from '%s'", sender);

		if ((mydomain = rindex (sender, '.')) == (char *) 0)
		{
			ll_log (logptr, LLOGTMP, "Sender with no dots '%s'");
			return (RP_NO);
		}
		mydomain++;

		if (en_init (sender, mydomain) != OK)
		{
		    ll_log (logptr, LLOGTMP, "Failed to initialise EAN");
		    return (RP_LIO);
		}
		free (sender);

		phs_note (chanptr, PHS_WRSTRT);

		if (rp_isbad (result = qu2en_each ()))
			return (result);
	}

	if (en_end() != OK)
	{               /* Make sure all is still happy */
		ll_log (logptr, LLOGTMP, "Bad  EAN termination");
		return (RP_RPLY);
	}

	if (rp_gval (result) != RP_DONE)
	{
		ll_log (logptr, LLOGTMP, "not DONE (%s)", rp_valstr (result));
		return (RP_RPLY);         /* catch protocol errors            */
	}

	qu_pkend ();                  /* done getting messages              */
	phs_note (chanptr, PHS_WREND);

	return (result);
}
/**/

LOCFUN
qu2en_each ()               /* send addresses and then text */
{
	RP_Buf  replyval;
	short   result;
	char    host[LINESIZE];
	AP_ptr ap;
	AP_ptr local,
		domain,
		route;
	char    *adr;

#ifdef DEBUG
	ll_log (logptr, LLOGBTR, "qu2en_each()");
#endif
	ap_outtype = AP_733;
	gotone = FALSE;

	/* while there are addessses to process... */
	FOREVER
	{
		result = qu_radr (host, en_adr);
		if (rp_isbad (result))
			return (result);      /* get address from Deliver */

		if (rp_gval (result) == RP_HOK)
		{                       /* no-op the sub-list indication */
			qu_wrply ((RP_Buf *) &rp_hend, rp_conlen (rp_hend));
			continue;
		}

		if (rp_gval (result) == RP_DONE)
		{

		    if (!gotone)
		    {
			ll_log (logptr, LLOGFST, "NO valid addrs so no text");
			qu_wrply ((RP_Buf *) &rp_gdtxt, rp_conlen (rp_gdtxt));
			return (RP_OK);
		    }

		    if (rp_isgood (result = qu2en_txtcpy()))
			qu_wrply ((RP_Buf *) &rp_gdtxt, rp_conlen (rp_gdtxt));
		    else
		    {
			if (result == RP_LIO)
			    qu_wrply ((RP_Buf *) &rp_lio,
					rp_conlen (rp_lio));
			else
			    qu_wrply ((RP_Buf *) &rp_bdtxt,
					rp_conlen (rp_bdtxt));
		    }
		    return (RP_OK);       /* end of message  */
		}

		ap = ap_s2tree (en_adr);
		if (ap != (AP_ptr) NOTOK)
		{
			ap_t2parts (ap, (AP_ptr *) 0, (AP_ptr *) 0,
				&local, &domain, &route);
			if (route != (AP_ptr) 0)
			    route = route -> ap_chain;
			adr = ap_p2s ((AP_ptr) 0, (AP_ptr) 0,
				local, domain, route);
			if(adr == (char *)MAYBE)
			    return (RP_NS);
#ifdef DEBUG
			ll_log (logptr, LLOGFST, "Fixed adr to '%s'",
				adr);
#endif
			ap_sqdelete (ap);
			ap_free (ap);
		}
		else
		{
			ll_log (logptr, LLOGTMP, "Bad format adr '%s'",
					en_adr);
			return (RP_NDEL);
		}

		if (en_wadr (adr,  replyval.rp_line) == OK)
		{
			gotone = TRUE;
			qu_wrply ((RP_Buf *) &rp_adr, rp_conlen (rp_adr));
		}
		else
		{
			replyval.rp_val = RP_USER;
			ll_log (logptr, LLOGFST, "EAN failed on addr '%s'",
				en_adr);
			qu_wrply (&replyval, (sizeof replyval.rp_val));
		}
		free (adr);
	}
}

/**/
char *template = "/tmp/enn.XXXXXX";


LOCFUN
qu2en_txtcpy ()                 /* copy the text of the message       */
{
	register int offset;
	int     len;
	int     result;
	char    buffer[BUFSIZ];
	int     fd;
	FILE    *fp;

#ifdef DEBUG
	ll_log (logptr, LLOGBTR, "qu2en_txtcpy()");
#endif



	mktemp (template);
	if ((fd = creat (template, 0666)) < 0)
	{
		ll_log (logptr, LLOGTMP, "Failed to create temp file %s",
			template);
		return (RP_LIO);
	}

	qu_rtinit (0L);
	len = sizeof(buffer);
	while (rp_gval (result = qu_rtxt (buffer, &len)) == RP_OK)
	{
		if (write (fd, buffer, len) != len)
		{
			ll_err (logptr, LLOGTMP, "error writing out text");
			return (RP_LIO);
		}
		len = sizeof(buffer);
	}

	if (rp_gval (result) != RP_DONE)
		return (RP_LIO);         /* didn't get it all across? */

	if (close (fd) != OK)
	{
		ll_log (logptr, LLOGTMP, "Failed to close file '%s'",
			template);
		return (RP_LIO);
	}

	if ((fp = fopen (template, "r")) == NULL)
	{
		ll_log (logptr, LLOGTMP, "Failed to reopne '%s'",
			template);
		return (RP_LIO);
	}

	if ((result = en_txt (fp)) != OK)
	{
		ll_log (logptr, LLOGTMP, "EAN copy of file '%s' failed",
			template);
		fclose (fp);
		return (RP_NO);
	}

	fclose (fp);
	unlink (template);

	return (RP_MOK);              /* got the text out                   */
}
