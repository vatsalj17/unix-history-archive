#include "util.h"
#include "mmdf.h"
#include "cmd.h"
#include "ch.h"
#include "dm.h"
#include "ap.h"

/*
 *  This module applies authorization on the basis of channel
 *  sending host, receiving host,  sender, and recipient.
 *
 *  Initial coding Steve Kille.  Jan 84
 */

extern struct ll_struct *logptr;

extern long tx_msize;
extern int adruid;

extern char     *authfile;              /* file with warning message    */
extern char     *authrequest;           /* where to send error request to */

extern char *locdomain;
extern char *locname;
extern char *adr_fulldmn;
extern char *supportaddr;
extern char *mgt_return;

Table *authtable;

char auth_msg [LINESIZE*2];

				/* space for auth error message   */
				/* to be passed upwards by submit */
extern char *logdfldir;

extern struct ll_struct authlog;
struct ll_struct *alogptr = &authlog;

char *user_string = "username";

extern char *index();
extern char *rindex();
extern long atol();
extern char *dupfpath();
extern char *strdup();
extern char *multcat();

char *get_route();


/*
 *  Structures needed for by user authorization
 */

				/* values for user auth */

#define AUTH_SEND 1             /* send only            */
#define AUTH_RECV 2             /* receive only         */
#define AUTH_BOTH 3             /* send + receive       */
#define AUTH_LIST 4             /* mailing list         */
#define AUTH_BAD  -1            /* not authorized       */
#define AUTH_EXPIRE -2          /* authorization expired*/

#define AUTH_NCHANS 15          /* max channels can have */

struct auth_struct
{
    int         a_type;         /* auth type            */
    int         a_nchans;       /* number of channels   */
    Chan        *a_chans [AUTH_NCHANS];
				/* channel list         */
    char        *a_addr;        /* the address          */
    char        *a_reason;      /* rason for expiry     */
};

LOCVAR
struct auth_struct auth_sender, /* sender authorization         */
		   auth_rcvr;   /* current receiver auth        */


LOCVAR
int  auth_do;                   /* is this mesage being authorized?     */
				/* prevents logging on local and other  */
				/* uninteresting traffic                */

LOCVAR
char *route_in;                 /* route assocaited with incoming       */
				/* address                              */
LOCVAR
char *route_out;

LOCVAR
char *cn_in, *cn_out;           /* storage for channel names            */
				/* needed for bad auth logging          */


/**/
				/* Authorization initialisation policy  */
				/* Store data on sender                 */

auth_init (sender, trust)
char *sender;                   /* Message sender (normalized)          */
int trust;                      /* can we trust this? (NRMFROM)         */
{
     char *p;
#ifdef DEBUG
     ll_log (logptr, LLOGBTR, "auth_init (%s)", sender);
#endif

     auth_sender.a_type = AUTH_BAD;
     auth_do = FALSE;
			      /* SEK this is needed so that WARN/LOG  */
			      /* work correctly for host auth         */

     if (alogptr -> ll_file[0] != '/')
	 alogptr -> ll_file = dupfpath (alogptr -> ll_file, logdfldir);

				/* strip  local domain for loocup      */
     auth_sender.a_addr= strdup (sender);
     auth_sender.a_nchans = 0;    /* make sure this has safe value        */
     auth_sender.a_reason = (char *) 0;

     if (isstr(sender))
	 route_in = get_route (sender);
     else
	 route_in = (char *)0;

     if ((authtable = tb_nm2struct ("auth")) == (Table *) NOTOK)
	return;

     if ( auth_sender.a_addr && auth_sender.a_addr[0] != '@')
     {
	if ((p = index (auth_sender.a_addr, '@')) != (char *) 0)
	   if (lexequ (p + 1, adr_fulldmn))
		*p = '\0';
     }
				  /* Table exists, so look up sender      */
     if (!trust && (adruid != 0))
				/* This can't be authorized             */
	auth_sender.a_type = AUTH_BAD;
     else
	auth_fetch (&auth_sender);
}
/**/
			/* initialise user check, as user may be */
			/*  checked in the context of a number of */
			/* channels                               */
auth_uinit (adr)
char *adr;               /* address being authorized             */
{
    char        *p,
		*q;
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "auth_uinit (%s)", adr);
#endif
    auth_msg [0] = '\0';
    route_out = (char *) 0;

    if (authtable == (Table *) NOTOK)
    {
	auth_rcvr.a_addr = strdup (adr);
	auth_rcvr.a_type= AUTH_BAD;
			/* Assume no authorization as default   */
	return;
    }

			/* this jiggling is to map local names */
			/* on other local machines to just      */
			/* user names                           */
			/* e.g.  ~foo@44c.ucl-cs.ac.uk -> foo   */
    if (adr[0] != '@')
    {
	p = index (adr, '@');
	if (p != (char *) 0)
	{
	    q = index (p + 1, '.');
	    if (lexequ (p+1, adr_fulldmn) ||
		    ((q != 0) && lexequ (q+1, adr_fulldmn)))
	    {
		*p = '\0';
		if (adr[0] == '~')
		    auth_rcvr.a_addr = strdup (&adr[1]);
		else
		    auth_rcvr.a_addr = strdup (adr);
		*p= '@';
	    }
	    else
		auth_rcvr.a_addr = strdup (adr);
	}
	else
	    auth_rcvr.a_addr = strdup (adr);
    }
    else
	auth_rcvr.a_addr = strdup (adr);


				/* Do not look up until needed - just mark */
    auth_rcvr.a_nchans = -1;
    auth_rcvr.a_reason = (char *) 0;
}

				/* clean up user structures             */
auth_uend ()
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "auth_uend (), %d", authtable);
#endif
    free (auth_rcvr.a_addr);
    if (authtable != (Table *) NOTOK)
    {
	if (auth_rcvr.a_reason != (char *) 0)
	    free (auth_rcvr.a_reason);
    }
    if (route_out != (char *) 0)
	free (route_out);
}


/**/
				/* Perform checks for a given address  */
LOCVAR char *a_fmt = "i='%s' o='%s' a='%s' r='%s' r='%s'";
LOCVAR char *h_fmt = "i='%s' o='%s' a='%s' r='%s' hi='%s' ho='%s'";

#define AOKNUM 50

auth_user (h_in, h_out, chan_in, chan_out)
char *h_in;                     /* host (not domain) directly from      */
char *h_out;                    /* host (not domain) being connected to */
Chan *chan_in;                  /* incoming channel                     */
Chan *chan_out;                 /* outgoing channel                     */
{
    char ch_in_auth;            /* user auth needed for incomin chan    */
    char ch_out_auth;           /* ditto for outgoing chan              */
    char in_hostanduser;         /* flag if both host AND user auth needed */
    char out_hostanduser;        /* flag if both host AND user auth needed */
    int gotinbound;
    int gotoutbound;
    char linebuf [LINESIZE];
    int argc;
    char *argv[AOKNUM];
    int ind;
    char *in_auth_str;
    char *out_auth_str;
    char *host_in;
    char *host_out;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "auth_user (hin='%s', hout='%s', cin='%s', cout='%s'",
		h_in, h_out, chan_in -> ch_show, chan_out -> ch_show);
#endif
    ch_in_auth = chan_in -> ch_auth & CH_IN_AUTH;
    ch_out_auth = chan_out -> ch_auth & CH_OUT_AUTH;

    /* return immediately if no checks needed */
    if ((ch_in_auth == 0) && (ch_out_auth == 0))
	return (TRUE);

    cn_in = chan_in -> ch_name;
    cn_out = chan_out -> ch_name;
    in_hostanduser = FALSE;
    out_hostanduser = FALSE;

    if ((chan_out -> ch_auth & CH_DHO) == CH_DHO)
    {
	if (route_out == (char*) 0)
	    route_out = get_route (auth_rcvr.a_addr);
	host_out = route_out;
    }
    else
	host_out = h_out;

    if ((chan_in -> ch_auth & CH_DHO) == CH_DHO)
	host_in = route_in;
    else
	host_in = h_in;

    if ((chan_out -> ch_auth & CH_HAU) == CH_HAU)
	out_hostanduser = TRUE;
    if ((chan_in -> ch_auth & CH_HAU) == CH_HAU)
	in_hostanduser = TRUE;

				/* initallialy check hostwise agreements */
				/* log any messages so authorized        */

				/* 1 outbound host, ch_out              */
				/* h_out:c_in/h_in                      */
    if (chan_out -> ch_outdest != (Table *) 0)
    {
	if (tb_k2val (chan_out -> ch_outdest, TRUE, host_out, linebuf)==OK)
	{                         /* this destination is listed         */
	    argc = str2arg (linebuf, AOKNUM, argv, (char *)0);
	    if (argc == 0)        /* authorized for all senders         */
	    {
		auth_log (h_fmt, chan_in -> ch_name, chan_out -> ch_name,
			auth_rcvr.a_addr, "OH", "", host_out);
		if (out_hostanduser)
		    goto douser;
		return (TRUE);
	    }
	    for (ind = 0; ind < argc; ind++)
				  /* all hosts on channel authorized?  */
		if (lexequ (chan_in -> ch_name, argv[ind]))
		{
		    auth_log (h_fmt, chan_in -> ch_name, chan_out -> ch_name,
				auth_rcvr.a_addr, "CH", "", host_out);
		    if (out_hostanduser)
			goto douser;
		    return (TRUE); /*  this pair is authorized      */
		}
	    for (ind = 0; ind < argc; ind++)
				       /* host, itself, authorized?    */
		if (lexequ (host_in, argv[ind]))
		{
		    auth_log (h_fmt, chan_in -> ch_name, chan_out -> ch_name,
			auth_rcvr.a_addr, "HH", host_in, host_out);
		    if (out_hostanduser)
			goto douser;
		    return (TRUE);
		}
	}
    }

				/* 2 inbound hosts, ch_in               */
				/* h_in:c_out/h_out                     */

    if (chan_in -> ch_insource != (Table *) 0)
    {
	if (tb_k2val (chan_in -> ch_insource, TRUE, host_in, linebuf)==OK)
	{                         /* this destination is listed         */
	    argc = str2arg (linebuf, AOKNUM, argv, (char *)0);
	    if (argc == 0)        /* authorized for all senders         */
	    {
		auth_log (h_fmt, chan_in -> ch_name, chan_out -> ch_name,
			auth_rcvr.a_addr, "IH", host_in, "");
		if (in_hostanduser)
		    goto douser;
		return (TRUE);
	    }
	    for (ind = 0; ind < argc; ind++)
				  /* all hosts on channel authorized?  */
		if (lexequ (chan_out -> ch_name, argv[ind]))
		{
		    auth_log (h_fmt, chan_in -> ch_name, chan_out -> ch_name,
				auth_rcvr.a_addr, "HC", host_in, "");
		    if (in_hostanduser)
			goto douser;
		    return (TRUE); /*  this pair is authorized      */
		}
	    for (ind = 0; ind < argc; ind++)
				       /* host, itself, authorized?    */
		if (lexequ (host_out, argv[ind]))
		{
		    auth_log (h_fmt, chan_in -> ch_name, chan_out -> ch_name,
			auth_rcvr.a_addr, "HH", host_in, host_out);
		    if (in_hostanduser)
			goto douser;
		    return (TRUE);
		}
	}
    }


					/* 3 inbound hosts, ch_out      */
					/* c_in:h_out                   */
					/* h_in:h_out                   */


    if (chan_out -> ch_outsource != (Table *) 0)
    {
	if (tb_k2val (chan_out -> ch_outsource, TRUE,
					chan_in -> ch_name, linebuf)==OK)
	{                         /* source channel is listed           */
	    argc = str2arg (linebuf, AOKNUM, argv, (char *)0);
	    if (argc == 0)        /* authorized for all receivers       */
	    {
		auth_log (h_fmt, chan_in -> ch_name, chan_out -> ch_name,
				auth_rcvr.a_addr, "CC", "", "");
		if (out_hostanduser)
		    goto douser;
		return (TRUE);
	    }
	    for (ind = 0; ind < argc; ind++)
		if (lexequ (host_out, argv[ind]))
		{
		    auth_log (h_fmt, chan_in -> ch_name, chan_out -> ch_name,
			auth_rcvr.a_addr, "CH", "", host_out);
		    if (out_hostanduser)
			goto douser;
		    return (TRUE); /* this channel is authorized        */
		}
	}
	if (tb_k2val (chan_out -> ch_outsource, TRUE,
				host_in, linebuf)==OK)
	{                         /* this source is listed              */
	    argc = str2arg (linebuf, AOKNUM, argv, (char *)0);
	    if (argc == 0)        /* authorized for all receivers       */
	    {
		auth_log (h_fmt, chan_in -> ch_name, chan_out -> ch_name,
			auth_rcvr.a_addr, "IH", host_in, "");
		if (out_hostanduser)
		    goto douser;
		return (TRUE);
	    }
	    for (ind = 0; ind < argc; ind++)
		if (lexequ (host_out, argv[ind]))
		{
		    auth_log (h_fmt, chan_in -> ch_name, chan_out -> ch_name,
			auth_rcvr.a_addr, "HH", host_in, host_out);
		    if (out_hostanduser)
			goto douser;
		    return (TRUE); /* this pair is authorized           */
		}
	}
    }

				/* 4, outbound hosts, ch_in               */
				/* h_out:h_in                             */
				/* c_out:h_in                             */

    if (chan_in -> ch_indest != (Table *) 0)
    {
	if (tb_k2val (chan_in -> ch_indest, TRUE,
					chan_out -> ch_name, linebuf)==OK)
	{                         /* source channel is listed           */
	    argc = str2arg (linebuf, AOKNUM, argv, (char *)0);
	    if (argc == 0)        /* authorized for all receivers       */
	    {
		auth_log (h_fmt, chan_in -> ch_name, chan_out -> ch_name,
				auth_rcvr.a_addr, "CC", "", "");
		if (in_hostanduser)
		    goto douser;
		return (TRUE);
	    }
	    for (ind = 0; ind < argc; ind++)
		if (lexequ (host_in, argv[ind]))
		{
		    auth_log (h_fmt, chan_in -> ch_name, chan_out -> ch_name,
			auth_rcvr.a_addr, "HC", host_in, "");
		    if (in_hostanduser)
			goto douser;
		    return (TRUE); /* this channel is authorized        */
		}
	}
	if (tb_k2val (chan_in -> ch_indest, TRUE,
				host_out, linebuf)==OK)
	{                         /* this source is listed              */
	    argc = str2arg (linebuf, AOKNUM, argv, (char *)0);
	    if (argc == 0)        /* authorized for all receivers       */
	    {
		auth_log (h_fmt, chan_in -> ch_name, chan_out -> ch_name,
			auth_rcvr.a_addr, "OH", "", host_out);
		if (in_hostanduser)
		    goto douser;
		return (TRUE);
	    }
	    for (ind = 0; ind < argc; ind++)
		if (lexequ (host_in, argv[ind]))
		{
		    auth_log (h_fmt, chan_in -> ch_name, chan_out -> ch_name,
			auth_rcvr.a_addr, "HH", host_in, host_out);
		    if (in_hostanduser)
			goto douser;
		    return (TRUE); /* this pair is authorized           */
		}
	}
    }

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "Checking auth:  need in =%o out =%o",
		ch_in_auth, ch_out_auth);
#endif

    if (in_hostanduser || out_hostanduser)
    {
	auth_log ("NO HOST AUTH i='%s' o='%s' s='%s' r='%s' msg='%s'",
	   cn_in, cn_out, auth_sender.a_addr, auth_rcvr.a_addr, auth_msg);
	return (FALSE);
    }


douser:
    in_auth_str = "";
    out_auth_str = "";

    if (ch_in_auth == 0)
	gotinbound = TRUE;
    else
	gotinbound = FALSE;

    if (ch_out_auth ==  0)
	gotoutbound = TRUE;
    else
	gotoutbound = FALSE;


			/* lookup value if needed               */
    if ((auth_rcvr.a_nchans < 0) && (authtable != (Table *) NOTOK))
    {
	auth_rcvr.a_nchans = 0;
	auth_fetch (&auth_rcvr);
    }


			/* now see if we can find any user authorization */
			/* if recipeint is authorized list, charge by pref */
    if (auth_rcvr.a_type == AUTH_LIST)
    {
	if (!gotinbound)
	    if (gotinbound = auth_ok (chan_in, &auth_rcvr, AUTH_RECV))
	    {
		in_auth_str = "IL";
		if (gotoutbound)
		{
		     auth_log (a_fmt, chan_in -> ch_name, chan_out -> ch_name,
			auth_rcvr.a_addr, in_auth_str, out_auth_str);
		    return (TRUE);
		}
	    }
	if (!gotoutbound)
	    if (gotoutbound = auth_ok (chan_out, &auth_rcvr, AUTH_RECV))
	    {
		out_auth_str ="OL";
		if (gotinbound)
		{
		     auth_log (a_fmt, chan_in -> ch_name, chan_out -> ch_name,
			auth_rcvr.a_addr, in_auth_str, out_auth_str);
		    return (TRUE);
		}
	    }
    }
			/* Now  check sender                            */
    if (!gotinbound)
	if (gotinbound = auth_ok (chan_in, &auth_sender, AUTH_SEND))
	{
	    in_auth_str = "IS";
	    if (gotoutbound)
	    {
		 auth_log (a_fmt, chan_in -> ch_name, chan_out -> ch_name,
		    auth_rcvr.a_addr, in_auth_str, out_auth_str);
		return (TRUE);
	    }
	}
	else if (auth_rcvr.a_type == AUTH_LIST)
	    switch (ch_in_auth)
	    {
		case  CH_IN_BLOCK:
		    return (FALSE);
		case  CH_IN_WARN:
		    auth_warn ();
		default:
		    in_auth_str = "*I";
		    if (gotoutbound)
		    {
			auth_log (a_fmt, chan_in -> ch_name,
			    chan_out -> ch_name,
			    auth_rcvr.a_addr, in_auth_str, out_auth_str);
			return (TRUE);
		    }
		    gotinbound = TRUE;
	    }

    if (!gotoutbound)
	if (gotoutbound = auth_ok (chan_out, &auth_sender, AUTH_SEND))
	{
	    out_auth_str ="OS";
	    if (gotinbound)
	    {
		 auth_log (a_fmt, chan_in -> ch_name, chan_out -> ch_name,
			auth_rcvr.a_addr, in_auth_str, out_auth_str);
		return (TRUE);
	    }
	}
	else if (auth_rcvr.a_type == AUTH_LIST)
	    switch (ch_out_auth)
	    {
		case  CH_OUT_BLOCK:
		    return (FALSE);
		case  CH_OUT_WARN:
		    auth_warn ();
		default:
		    out_auth_str = "*O";
		    auth_log (a_fmt, chan_in -> ch_name, chan_out -> ch_name,
			auth_rcvr.a_addr, in_auth_str, out_auth_str);
		    return (TRUE);
	    }

				/* if not done aready, check receiver   */
    if (auth_rcvr.a_type == AUTH_LIST)
	 return (FALSE);


    if (!gotinbound)
	if (gotinbound = auth_ok (chan_in, &auth_rcvr, AUTH_RECV))
	{
	    in_auth_str = "IR";
	    if (gotoutbound)
	    {
		 auth_log (a_fmt, chan_in -> ch_name, chan_out -> ch_name,
			auth_rcvr.a_addr, in_auth_str, out_auth_str);
		return (TRUE);
	    }
	}
	else
	    switch (ch_in_auth)
	    {
		case  CH_IN_BLOCK:
		    return (FALSE);
		case  CH_IN_WARN:
		    auth_warn ();
		default:
		    in_auth_str = "*I";
		    if (gotoutbound)
		    {
			  auth_log (a_fmt, chan_in -> ch_name,
				chan_out -> ch_name,
				auth_rcvr.a_addr, in_auth_str, out_auth_str);
			  return (TRUE);
		    }
		    gotinbound = TRUE;
	    }

    if (!gotoutbound)
	if (gotoutbound = auth_ok (chan_out, &auth_rcvr, AUTH_RECV))
	{
	    out_auth_str  = "OR";
	    if (gotinbound)
	    {
		 auth_log (a_fmt, chan_in -> ch_name, chan_out -> ch_name,
			auth_rcvr.a_addr, in_auth_str, out_auth_str);
		 return (TRUE);
	    }
	}
	else
	    switch (ch_out_auth)
	    {
		case  CH_OUT_BLOCK:
		    return (FALSE);
		case  CH_OUT_WARN:
		    auth_warn ();
		default:
		    auth_log (a_fmt, chan_in -> ch_name, chan_out -> ch_name,
			auth_rcvr.a_addr, in_auth_str, "*O");
		    return (TRUE);
	    }

			/* SEK shouldn't get here - I hope              */
#ifdef DEBUG
    ll_log (logptr, LLOGTMP,  "Auth_user - did the impossible");
#endif
    return (FALSE);
}
/**/
LOCFUN auth_warn ()
{
    FILE *fp;
    char buf [LINESIZE];
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "auth_warn ()");
#endif
    if (!isstr(mgt_return))
	return;
     sprintf (buf, "Authorization Warning Daemon <%s>", authrequest);
     ml_1adr (NO, YES, buf, "Authorization Warning",
		mgt_return);
     ml_txt ("\nNot authorized to send mail to address: ");
     ml_txt (auth_rcvr.a_addr);
     ml_txt ("\n\n");
     if ((fp = fopen (authfile, "r")) == 0)
     {
	ml_txt ("Message  sent anyhow\n\n");
	ll_log (logptr, LLOGTMP,  "Auth warning file '%s' not found",
		authfile);
     }
     else
	 ml_file (fp);
     if (ml_end (OK) != OK)
	ll_log (logptr, LLOGTMP, "Failed to send auth warning to '%s'",
		auth_sender.a_addr);
}
/**/
				/* what to do if bad address            */
auth_bad ()
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "auth_bad()");
#endif
				/* build string for submit to pass up */
    if (auth_sender.a_type == AUTH_EXPIRE)
    {
	if (auth_sender.a_reason  == (char *) 0)
	    sprintf (auth_msg, "sender '%s' no longer authorized for address '%s'",
		auth_sender.a_addr, auth_rcvr.a_addr);
	else
	    sprintf (auth_msg, "sender '%s' no authorization for address '%s' for reason: (%s)",
		auth_sender.a_addr,
		auth_rcvr.a_addr,
		auth_sender.a_reason);
    }
    else if (auth_rcvr.a_type == AUTH_EXPIRE)
    {
	if (auth_rcvr.a_reason == (char *) 0)
	    sprintf (auth_msg, "recipient '%s' no longer authorized",
			auth_rcvr.a_addr);
	else
	    sprintf (auth_msg, "recipient '%s' no authorization for reason: (%s)",
		auth_rcvr.a_addr,
		auth_rcvr.a_reason);
    }
    else
	sprintf (auth_msg, "sender '%s' not authorized to send to address '%s'",
		     auth_sender.a_addr, auth_rcvr.a_addr);

    auth_log ("BAD AUTH i='%s' o='%s' s='%s' r='%s' msg='%s'",
	cn_in, cn_out, auth_sender.a_addr, auth_rcvr.a_addr, auth_msg);
#ifdef  UCL
    strcat (auth_msg, " : send textless msg to auto-mailbox ");
    strcat (auth_msg, authrequest);
    strcat (auth_msg, " for info");
#else   UCL
    strcat (auth_msg, " : send queries to ");
    strcat (auth_msg, authrequest);
#endif
}


/**/
				/* Perform any authorization ending policy */

auth_end ()
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "auth_end()");
#endif
    if (auth_do)
	auth_log ("END size='%ld', sender='%s'", tx_msize, auth_sender.a_addr);
    if (auth_sender.a_addr != (char *)0)
	free (auth_sender.a_addr);
    if (auth_sender.a_reason != (char *) 0)
	free (auth_sender.a_reason);
    if (route_in != (char *)0)
	free (route_in);
}

/**/

				/* check if chan matches auth           */
LOCFUN
auth_ok (chan, auth, direction)
Chan    *chan;
struct auth_struct *auth;
int     direction;              /* send or receive                      */
{
    int i;

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "auth_ok (c='%s', u='%s', d='%s')",
	chan -> ch_name, auth -> a_addr,
	(direction == AUTH_RECV) ? "receive" : "send");
#endif

    if (auth  -> a_type < 0)
	return (FALSE);

    if (direction == AUTH_RECV)
    {
	switch (auth -> a_type)
	{
	    case AUTH_SEND:
		return (FALSE);
	}
    }
    else
    {
	switch (auth -> a_type)
	{
	    case AUTH_RECV:
	    case AUTH_LIST:
		return (FALSE);
	}
    }

    for (i = 0; i < auth -> a_nchans; i++)
	if (chan == auth -> a_chans [i])
	{
#ifdef DEBUG
	    ll_log (logptr,  LLOGFTR, "Got valid authorization");
#endif
	    return (TRUE);
	}
    return (FALSE);
}

/**/

LOCVAR Cmd
	authmap [] =
{
	"both", AUTH_BOTH,      0,
	"send", AUTH_SEND,      0,
	"recv", AUTH_RECV,      0,
	"list", AUTH_LIST,      0,
	"expire", AUTH_EXPIRE,  0,
	0,      0,              0,
};


				/* Get user authorization from table        */
LOCFUN
auth_fetch (auth)
struct auth_struct *auth;       /* where to put the data                    */
{
    char        buf [LINESIZE];
    char        *argv [AUTH_NCHANS + 1];
    int         i,
		argc;
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "auth_fetch (%s)", auth -> a_addr);
#endif
				/* implictly auth table exists             */
    if (tb_k2val (authtable, TRUE, auth -> a_addr, buf)!=OK)
    {
	auth -> a_type = AUTH_BAD;
	return;
    }

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "user '%s' has auth '%s'", auth -> a_addr, buf);
#endif
				/* authorization ha form of type  */
				/* followed by a list of channels */
    if ((argc = str2arg (buf, AUTH_NCHANS + 1, argv, (char *) 0))
		== NOTOK)
    {
	ll_log (logptr, LLOGTMP, "User '%s' has more that %d chans",
			auth -> a_addr, AUTH_NCHANS);
	auth -> a_type = AUTH_BAD;
	return;
    }
				/* get auth type                */
    switch (auth -> a_type = cmdsrch (argv[0], 0, authmap))
    {
	case AUTH_BOTH:
	case AUTH_SEND:
	case AUTH_RECV:
	case AUTH_LIST:
	    break;
	case  AUTH_EXPIRE:
	    break;
	default:
	    auth -> a_type = AUTH_EXPIRE;
	    auth -> a_reason = strdup (argv [0]);
#ifdef DEBUG
	    ll_log (logptr, LLOGFTR, "User '%s' expire reason '%s'",
			auth -> a_addr, argv[0]);
#endif
	    break;
    }


			/* now fill in the chans                */
    auth -> a_nchans = 0;
    for (i = 1; i < argc; i++)
    {
	if ((auth -> a_chans [auth -> a_nchans] = ch_nm2struct (argv[i]))
		== (Chan *) NOTOK)
	    ll_log (logptr, LLOGBTR,
		"Unknown channel name '%s' for user '%s'", argv[i], auth -> a_addr);
	else
	     auth -> a_nchans++;
    }
#ifdef  DEBUG
    ll_log (logptr, LLOGFTR, "%d channels in total type = %d",
		auth ->  a_nchans, auth -> a_type);
#endif
}

/**/

				/* this loggin might be done differnetly */
				/* later                                 */

extern char *mq_munique;

				/* Log authorization info                  */
/*VARARGS1*/
LOCFUN
auth_log (format, a1, a2, a3, a4, a5, a6, a7, a8, a9)
char *format,
	*a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8, *a9;
{
	char fbuf [LINESIZE];

	auth_do = TRUE;         /* we now care about this message       */
	sprintf (fbuf, "%%s: %s", format);
	ll_log (&authlog, LLOGFST, fbuf, mq_munique, a1, a2, a3, a4, a5,
		a6, a7, a8, a9);
}

/**/

/*
 * get_route is used to determine the route assocaited with
 * a given address.  This allows authorization on the whole
 * route and not just the connect host.
 * It works by:
 * 1) breaking the address into parts
 * 2) remove the username from the local part, assiming a mix of percent
 *      and UUCP routes.
 *      The username starts right of the rightmost "!", and is
 *      terminated by "%".
 * 3) Put it all back together again
 */
char *get_route (adr)
char *adr;
{
    char        *p, *q, *r;
    AP_ptr      local,          /* pointers for parse coponents         */
		domain,
		route;
    AP_ptr      ap;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "get_route (%s)", adr);
#endif

    ap = ap_s2tree (adr);

    ap_t2parts (ap, (AP_ptr *) 0, (AP_ptr *) 0, &local, &domain, &route);

#ifdef DEBUG
   ll_log (logptr, LLOGFTR, "local part = '%s'", local -> ap_obvalue);
#endif
				/* jiggle the local part                */
    if ((p = rindex (local -> ap_obvalue, '!')) == (char *) 0)
    {
	if ((p = index (local -> ap_obvalue, '%')) != (char *) 0)
	{
	     q = multcat (p, user_string, (char *)0);
	     free (local -> ap_obvalue);
	     local -> ap_obvalue = q;
	}
	else
	{
				/* local part really sseems to be user */
	    free (local -> ap_obvalue);
	    local -> ap_obvalue = strdup (user_string);
	}
    }
    else
    {
				/* p points to last "!"                 */
	q = index (p, '%');
	p++;
	*p = '\0';
	if (q != (char *) 0)
	    r = multcat (local -> ap_obvalue, user_string, q, (char *)0);
	else
	    r  = multcat (local -> ap_obvalue, user_string, (char *)0);
	*p =  '%';
	free (local -> ap_obvalue);
	local -> ap_obvalue = r;
    }

#ifdef DEBUG
   ll_log (logptr, LLOGFTR, "Munged local part = '%s'", local -> ap_obvalue);
#endif

    p = ap_p2s ((AP_ptr) 0, (AP_ptr) 0, local, domain, route);

    ap_sqdelete (ap, (AP_ptr)0);
    ap_free (ap);

#ifdef DEBUG
    ll_log (logptr,LLOGFTR, "Auth ROUTE is '%s'",(p==(char*)MAYBE)?"MAYBE":p);
#endif
    return (p);
}
