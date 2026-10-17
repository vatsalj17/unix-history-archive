/*
 *                      S M T P S R V R . C
 *
 *              SMTP Mail Server for MMDF under 4.2bsd
 *
 * Eric Shienbrood (BBN) 3 Apr 81 - SMTP server, based on BBN FTP server
 * Modified to talk to MMDF Jun 82 by Donn Neuhengen.
 * Doug Kingston, BRL: Took a machete to large chunks of unnecessary code.
 *      Reimplemented the interface to MMDF and used 4.2BSD style networking.
 *      Usage:  smtpsrvr <them> <us> <channel>
 */

#include "util.h"
#include "mmdf.h"
#include "ch.h"
#include "ap.h"
#include "phs.h"
#include "smtp.h"
#include <stdio.h>
#include <signal.h>
#include <sys/stat.h>
#include <errno.h>

#include "ns.h"

#define BYTESIZE        8

/* mmdf configuration variables */
extern LLog *logptr;
extern char *supportaddr;
extern int errno;               /* Has Unix error codes */

extern char *getline();
extern char *index();
extern char *rindex();
extern char *malloc();
extern char *strdup();

char *progname, *us, *them, *channel;   /* Arguments to program */
char *sender = 0;                       /* address of mail sender */
Chan *chanptr;                          /* pointer to incoming channel */

#define BUFL 600                /* length of buf */
char    buf[BUFL];              /* general usage */
char    netbuf[BUFL];           /* the place that has the valid characters */
int     netcount = 0;           /* number of valid characters in netbuf */
char    *netptr  = netbuf;      /* next character to come out of netbuf */
char    *arg;                   /* zero if no argument - pts to comm param */
int     dont_mung;              /* used by getline() to limit munging */
int     numrecipients = 0;      /* number of valid recipients accepted */
int     stricked;               /* force rejection of non validated hosts */
#ifdef NODOMLIT
int	themknown=TRUE;		/* do we have symbolic name for them? */
#endif NODOMLIT
char    *addrfix();

/* character defines */
#define CR      '\r'    /* carriage return */
#define LF      '\n'    /* line feed */
#define CNULL   '\0'    /* null */


/****************************************************************
 *                                                              *
 *      C O M M A N D   D I S P A T C H   T A B L E             *
 *                                                              *
 ****************************************************************/

int helo(), mail(), quit(), help(), rcpt(), confirm();
int data(), rset(), reject();

struct comarr           /* format of the command table */
{
	char *cmdname;          /* ascii name */
	int (*cmdfunc)();       /* command procedure to call */
} commands[] = {
	"helo", helo,           "noop", confirm,
	"mail", mail,           "data", data,
	"rcpt", rcpt,           "help", help,
	"quit", quit,           "rset", rset,
	NULL, NULL
};


/*
 *              M A I N
 *
 *      Takes commands from the assumed network connection (file desc 0)
 *      under the assumption that they follow the ARPA network mail
 *      transfer protocol RFC 788 and appropriate modifications.
 *      Command responses are returned via file desc 1.
 *
 *      There is a small daemon waiting for connections to be
 *      satisfied on socket 25 from any host.  As connections
 *      are completed by the ncpdaemon, the returned file descriptor
 *      is setup as the standard input (file desc 0) and standard
 *      output (file desc 1) and this program is executed to
 *      deal with each specific connection.  This allows multiple
 *      server connections, and minimizes the system overhead
 *      when connections are not in progress.  File descriptors
 *      zero and one are used to ease production debugging and
 *      to allow the forking of other relavent Unix programs
 *      with comparative ease.
 *
 *              while commands can be gotten
 *                      find command procedure
 *                              call command procedure
 *                              get next command
 *                      command not found
 *
 */
main (argc, argv)
int argc;
char **argv;
{
	register struct comarr *comp;
	char    replybuf[128];
	char *i;
	long atime;
	Chan *curchan;
	char    tmp_buf[LINESIZE];
	char    tmpstr[LINESIZE];
	char    *Ags[20];
	int     n, Agc;

	progname = argv[0];
	mmdf_init( progname );

	if (argc != 4){
		ll_log( logptr, LLOGFAT, "wrong number of args!" );
		exit(NOTOK);
	}

	/* force rejection of unknown hosts */
	if (*progname == 'r')
		stricked++;

	/*
	 * first look up address of sender. Used to be in server but
	 * now must be in this routine so that we can send a reject
	 * reply.
	 */
	them = argv[1];
	us = argv[2];
				/* A numeric address - don't like it */
	if(*them >= '0' && *them <= '9')
	    if ( stricked){
		ll_log (logptr, LLOGTMP, "smtpsrvr can't lookup '%s'", them);
		sprintf(replybuf,
		       "421 %s: Cannot resolve your address. '%s'\r\n",us,them);
		netreply (replybuf);
		exit (-1);
	    }
	    else {   /* make into [x.x.x.x] format */
		them = strdup(them);
	        strcpy(tmpstr, them);  
		sprintf(them, "[%s]", tmpstr);
#ifdef NODOMLIT
		themknown = FALSE;
#endif NODOMLIT
	    }
	    
	/*
	 * found out who you are I might even believe you.
	 */

	/*
	 * the channel arg is now a comma seperated list of channels
	 * useful for multiple sources ( As on UCL's ether )
	 */
	strcpy (tmp_buf, argv[3]);
	Agc = str2arg (tmp_buf, 20, Ags, (char *)0);
	channel = Ags[Agc-1];
	for(chanptr = (Chan *)0, n = 0 ; n < Agc ; n++){
		if ( (curchan = ch_nm2struct(Ags[n])) == (Chan *) NOTOK) {
			ll_log (logptr, LLOGTMP, "smtpsrvr (%s) bad channel",
				Ags[n]);
			continue;
		}
		/*
		 * Is this a valid host for this channel ?
		 */
		switch(tb_k2val (curchan -> ch_table, TRUE, them, tmpstr)){
		default:        /* Either NOTOK or MAYBE */
			if ((n != (Agc-1)) || stricked)
				continue;
			/* fall through so we get some channel name to use */
		case OK:
			chanptr = curchan;
			channel = curchan -> ch_name;
			break;
		}
		break;
	}
	time(&atime);

	if (chanptr == (Chan *) 0){
		ll_log (logptr, LLOGTMP, "smtpsrvr (%s) no channel for host",
								    them);
		sprintf (replybuf,
			"421 %s: Your name, '%s', is unknown to us.\r\n",
			us, them);
		netreply (replybuf);
		exit (-1);
	}
	ch_llinit (chanptr);
	ll_log( logptr, LLOGGEN, "OPEN: %s %.19s (%s)",
			them, ctime(&atime), channel);
	phs_note(chanptr, PHS_RESTRT);

	mmdfstart();

	/* say we're listening */
	sprintf (replybuf, "220 %s Server SMTP (Complaints/bugs to:  %s)\r\n",
			us, supportaddr);
	netreply (replybuf);

nextcomm:
	while (i = getline())
	{
		if (i == (char *)NOTOK)         /* handle error ??? */
			byebye( 1 );

		/* find and call command procedure */
		comp = commands;
		while( comp->cmdname != NULL)   /* while there are names */
		{
		    if (strcmp(buf, comp->cmdname) == 0) /* a winner */
		    {
			(*comp->cmdfunc)();     /* call comm proc */
			goto nextcomm;          /* back for more */
		    }
		    comp++;             /* bump to next candidate */
		}
		netreply("500 Unknown or unimplemented command\r\n" );
		ll_log(logptr, LLOGBTR, "unknown command '%s'", buf);
	}
	byebye(0);
}

/*name:
	getline

function:
	get commands from the standard input terminated by <cr><lf>.
	afix a pointer( arg ) to any arguments passed.
	ignore carriage returns
	map UPPER case to lower case.
	manage the netptr and netcount variables

algorithm:
	while we havent received a line feed and buffer not full

		if netcount is zero or less
			get more data from net
				error: return 0
		check for delimiter character
			null terminate first string
			set arg pointer to next character
		check for carriage return
			ignore it
		if looking at command name
			convert upper case to lower case

	if command line (not mail)
		null terminate last token
	manage netptr

returns:
	0 for EOF
	-1 when an error occurs on network connection
	ptr to last character (null) in command line

globals:
	dont_mung
	netcount=
	netptr =
	buf=
*/

char *
getline()
{
	register char *inp;     /* input pointer in netbuf */
	register char *outp;    /* output pointer in buf */
	register int c;         /* temporary char */
	extern char *progname;

	inp = netptr;
	outp = buf;
	arg = 0;

	do
	{
		if( --netcount <= 0 )
		{
			if (setjmp(timerest)) {
				ll_log( logptr, LLOGTMP,
					"%s net input read error (%d)",
					them, errno);
				return((char *)NOTOK);
			}
			s_alarm (NTIMEOUT);
			netcount = read (0, netbuf, 512);
			s_alarm( 0 );
			if (netcount == 0)      /* EOF */
				return( 0 );
			if (netcount < 0) {     /* error */
				ll_log( logptr, LLOGTMP,
					"%s net input read error (%d)",
					them, errno);
				return((char *)NOTOK);
			}
			inp = netbuf;
		}
		c = *inp++ & 0377;

		if (c == '\r' ||        /* ignore CR */
		    c >= 0200)          /* or any telnet codes that */
			continue;       /*  try to sneak through */
		if (dont_mung == 0 && arg == NULL)
		{
			/* if char is a delim afix token */
			if (c == ' ' || c == ',')
			{
				c = CNULL;       /* make null term'ed */
				arg = outp + 1; /* set arg ptr */
			}
			else if (c >= 'A' && c <= 'Z')
			/* do case mapping (UPPER -> lower) */
				c += 'a' - 'A';
		}
		*outp++ = c;
	} while( c != '\n' && outp < &buf[BUFL] );

	if( dont_mung == 0 )
		*--outp = 0;            /* null term the last token */

	/* scan off blanks in argument */
	if (arg) {
		while (*arg == ' ')
			arg++;
		if (*arg == '\0')
			arg = 0;        /* if all blanks, no argument */
	}
	if (dont_mung == 0)
		ll_log( logptr, LLOGFTR, "'%s', '%s'", buf,
			arg == 0 ? "<noarg>" : arg );

	/* reset netptr for next trip in */
	netptr = inp;
	/* return success */
	return (outp);
}

/*
 *  Process the HELO command
 */
helo()
{
	char replybuf[128];

	if(arg == 0 || !lexequ(arg, them))
		sprintf(replybuf, "250 %s - you are a charlatan\r\n", us);
	else 
		sprintf (replybuf, "250 %s\r\n", us);
	netreply (replybuf);
}

/*
 *      mail
 *
 *      handle the MAIL command  ("MAIL from:<user@host>")
 */
mail()
{
	char    replybuf[128];
	char    info[128];
	struct rp_bufstruct thereply;
	int	len;
	AP_ptr  domain, route, mbox, themap, ap_sender;

	if (arg == 0 || *arg == 0) {
		netreply("501 No argument supplied\r\n");
		return;
	} else if( sender ) {
		netreply("503 MAIL command already accepted\r\n");
		return;
	} else if (!equal(arg, "from:", 5)) {
		netreply("501 No sender named\r\n");
		return;
	}

	/* Scan FROM: parts of arg */
	sender = index (arg, ':') + 1;
	sender = addrfix( sender );
	/*
	 * If the From part is not the same as where it came from
	 * then add on the extra part of the route.
	 */

#ifdef NODOMLIT
	if(themknown && ((ap_sender = ap_s2tree(sender)) != (AP_ptr)NOTOK)){
#else
	if ((ap_sender = ap_s2tree(sender)) != (AP_ptr)NOTOK){
#endif NODOMLIT
		/*
		 * this must be a bit of a sledge hammer approach ??
		 */
		ap_t2parts(ap_sender, (AP_ptr *)0, (AP_ptr *)0,
						&mbox, &domain, &route);
		themap = ap_new(APV_DOMN, them);
		if(ap_dmnormalize(themap, (Chan *)0) == MAYBE)
			goto tout;
		if(route != (AP_ptr)0){
			/*
			 * only normalize the bits that we need
			 */
			if(ap_dmnormalize(route, (Chan *)0) == MAYBE)
				goto tout;
			if(lexequ(themap->ap_obvalue, route->ap_obvalue))
				ap_free(themap);
			else {
				themap->ap_chain = route;
				route = themap;
			}
		}
		else if(domain != (AP_ptr)0) {
			if(ap_dmnormalize(domain, (Chan *)0) == MAYBE)
				goto tout;
			if(lexequ(themap->ap_obvalue, domain->ap_obvalue))
				ap_free(themap);
			else
				route = themap;
		}
		sender = ap_p2s((AP_ptr)0, (AP_ptr)0, mbox, domain, route);
		if(sender == (char *)MAYBE){    /* help !! */
	tout:;
			netreply("451 Nameserver timeout during parsing\r\n");
			return;
		}
		strcpy(arg, sender);
		free(sender);
		sender = arg;
	}

	/* Supply necessary flags, "tiCHANNEL" will be supplied by winit */
	if (*sender == NULL) {
		/* No return mail */
		sprintf( info, "mvqdh%s*k%d*", them, NS_NETTIME );
		sender = "Orphanage";           /* Placeholder */
	} else
		sprintf( info, "mvdh%s*k%d*", them, NS_NETTIME );

	if( rp_isbad( mm_winit(channel, info, sender))) {
		netreply("451 Temporary problem initializing\r\n");
		mm_end( NOTOK );
		mmdfstart();
	} else if( rp_isbad( mm_rrply( &thereply, &len ))) {
		netreply( "451 Temporary problem initializing\r\n" );
		mm_end( NOTOK );
		mmdfstart();
	} else if( rp_gbval( thereply.rp_val ) == RP_BNO) {
		sprintf (replybuf, "501 %s\r\n", thereply.rp_line);
		netreply (replybuf);
		mm_end( NOTOK );
		mmdfstart();
	} else if( rp_gbval( thereply.rp_val ) == RP_BTNO) {
		sprintf (replybuf, "451 %s\r\n", thereply.rp_line);
		netreply (replybuf);
		mm_end( NOTOK );
		mmdfstart();
	} else
		netreply("250 OK\r\n");
	numrecipients = 0;
}

/*
 *  Process the RCPT command  ("RCPT TO:<forward-path>")
 */
rcpt()
{
	register char *p;
	struct rp_bufstruct thereply;
	char    replybuf[128];
	int     len;

	/* parse destination arg */
	if( sender == (char *)0 ) {
		netreply("503 You must give a MAIL command first\r\n");
		return;
	} else if (arg == (char *)0 || !equal(arg, "to:", 3)) {
		netreply("501 No recipient named.\r\n");
		return;
	}
	p = index( arg, ':' ) + 1;
	p = addrfix( p );

	if (setjmp(timerest)) {
		netreply( "451 Mail system problem\r\n" );
		return;
	}
	s_alarm (DTIMEOUT);
	if( rp_isbad( mm_wadr( (char *)0, p ))) {
		if( rp_isbad( mm_rrply( &thereply, &len )))
			netreply( "451 Mail system problem\r\n" );
		else {
			sprintf (replybuf, "451 %s\r\n", thereply.rp_line);
			netreply (replybuf);
		}
	} else {
		if( rp_isbad( mm_rrply( &thereply, &len )))
			netreply("451 Mail system problem\r\n");
		else if( rp_gbval( thereply.rp_val ) == RP_BNO) {
			sprintf (replybuf, "550 %s\r\n", thereply.rp_line);
			netreply (replybuf);
		}
		else if( rp_gbval( thereply.rp_val ) == RP_BTNO) {
			sprintf (replybuf, "451 %s\r\n", thereply.rp_line);
			netreply (replybuf);
		}
		else {
			netreply("250 Recipient OK.\r\n");
			numrecipients++;
		}
	}
	s_alarm (0);
}

/*
 *      ADDRFIX()  --  This function takes the SMTP "path" and removes
 *      the leading and trailing "<>"'s which would make the address
 *      illegal to RFC822 mailers.  Note that although the spec states
 *      that the angle brackets are required, we will accept addresses
 *      without them.   (DPK@BRL, 4 Jan 83)
 */
char *
addrfix( addrp )
char *addrp;
{
	register char   *cp;

	if( cp = index( addrp, '<' )) {
		addrp = ++cp;
		if( cp = rindex( addrp, '>' ))
			*cp = 0;
	}
	compress (addrp, addrp);
#ifdef DEBUG
	ll_log( logptr, LLOGFTR, "addrfix(): '%s'", addrp );
#endif
	return( addrp );
}

/*
 *  Process the DATA command.  Send text to MMDF.
 */
data()
{
	register char *p, *bufptr;
	time_t  tyme;
	int     errflg, werrflg;
	struct rp_bufstruct thereply;
	int     len, msglen;

	errflg = werrflg = msglen = 0;
	if (numrecipients == 0) {
		netreply("503 No recipients have been specified.\r\n");
		return;
	}

	if (setjmp(timerest)) {
		netreply( "451 Mail system problem\r\n" );
		return;
	}
	s_alarm (DTIMEOUT);
	if( rp_isbad(mm_waend())) {
		netreply("451 Unknown mail system trouble.\r\n");
		return;
	}
	s_alarm (0);

	netreply ("354 Enter Mail, end by a line with only '.'\r\n");

	dont_mung = 1;      /* tell getline only to drop cr */
#ifdef DEBUG
	ll_log( logptr, LLOGFTR, "... body of message ..." );
#endif
	while (1) {             /* forever */
		if ((p = getline()) == 0) {
			p = "\n***Sender closed connection***\n";
			mm_wtxt( p , strlen(p) );
			errflg++;
			break;
		}
		if (p == (char *)NOTOK) {
			p = "\n***Error on net connection***\n";
			mm_wtxt( p , strlen(p) );
			if (!errflg++)
				ll_log(logptr, LLOGTMP,
					"netread error from host %s", them);
			break;
		}

		/* are we done? */
		if (buf[0] == '.')
			if (buf[1] == '\n')
				break;          /* yep */
			else
				bufptr = &buf[1];       /* skip leading . */
		else
			bufptr = &buf[0];
		/* If write error occurs, stop writing but keep reading. */
		if (!werrflg) {
			if (setjmp(timerest)) {
				netreply( "451 Mail system problem\r\n" );
				return;
			}
			s_alarm (DTIMEOUT);
			msglen += (len = p-bufptr);
			if( rp_isbad(mm_wtxt( bufptr, len ))) {
				werrflg++;
				ll_log( logptr, LLOGTMP, "error from submit");
			}
			s_alarm (0);
		}
	}
	dont_mung = 0;  /* set getline to normal operation */
#ifdef DEBUG
	ll_log( logptr, LLOGBTR, "Finished receiving text." );
#endif

	if (werrflg) {
		netreply("451-Mail trouble (write error to mailsystem)\r\n");
		netreply("451 Please try again later.\r\n");
		byebye( 1 );
	}
	if (errflg) {
	    time (&tyme);
	    byebye(1);
	}

	if (setjmp(timerest)) {
		netreply( "451 Mail system problem\r\n" );
		return;
	}
	s_alarm (DTIMEOUT);
	if( rp_isbad(mm_wtend()) || rp_isbad( mm_rrply( &thereply, &len)))
		netreply("451 Unknown mail trouble, try later\r\n");
	else if( rp_isgood(thereply.rp_val)) {
		sprintf (buf, "250 %s\r\n", thereply.rp_line);
		netreply (buf);
		phs_msg(chanptr, numrecipients, (long) msglen);
	}
	else if( rp_gbval(thereply.rp_val) == RP_BNO) {
		sprintf (buf, "554 %s\r\n", thereply.rp_line);
		netreply (buf);
	}
	else {
		sprintf (buf, "451 %s\r\n", thereply.rp_line);
		netreply (buf);
	}
	s_alarm (0);
	sender = (char *) 0;
	numrecipients = 0;
}

/*
 *  Process the RSET command
 */
rset()
{
	mm_end( NOTOK );
	sender = (char *)0;
	mmdfstart();
	confirm();
}

mmdfstart()
{
	if( rp_isbad( mm_init() ) || rp_isbad( mm_sbinit() )) {
		ll_log( logptr, LLOGFAT, "can't reinitialize mail system" );
		netreply("421 Server can't initialize mail system (mmdf)\r\n");
		byebye( 2 );
	}
	numrecipients = 0;
}

/*
 *  handle the QUIT command
 */
quit()
{
	time_t  timenow;

	time (&timenow);
	sprintf (buf, "221 %s says goodbye to %s at %.19s.\r\n",
		us, them, ctime(&timenow));
	netreply(buf);
	byebye( 0 );
}

byebye( retval )
int retval;
{
	if (retval == OK) {
		mm_sbend();
		phs_note(chanptr, PHS_REEND);
	}
	mm_end( retval == 0 ? OK : NOTOK );
	exit( retval );
}

/*
 *  Reply that the current command has been logged and noted
 */
confirm()
{
	netreply("250 OK\r\n");
}

/*
 *  Process the HELP command by giving a list of valid commands
 */
help()
{
	register i;
	register struct comarr *p;
	char    replybuf[128];

	netreply("214-The following commands are accepted:\r\n214-" );
	for(p = commands, i = 1; p->cmdname; p++, i++) {
		sprintf (replybuf, "%s%s", p->cmdname, ((i%10)?" ":"\r\n214-") );
		netreply (replybuf);
	}
	sprintf (replybuf, "\r\n214 Send complaints/bugs to:  %s\r\n", supportaddr);
	netreply (replybuf);
}

/*
 *  Send appropriate ascii responses over the network connection.
 */

netreply(string)
char *string;
{
	if (setjmp(timerest)) {
		byebye( 1 );
	}
	s_alarm (NTIMEOUT);
	if(write(1,string, strlen(string)) < 0){
		s_alarm( 0 );
		ll_log( logptr, LLOGFST,
			"(netreply) error (%d) in writing [%s] ...",
			errno, string);
		byebye( 1 );
	}
	s_alarm( 0 );
#ifdef DEBUG
	ll_log( logptr, LLOGFTR, "%s", string);
#endif DEBUG
}
