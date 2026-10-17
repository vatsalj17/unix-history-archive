/* name:
	post (and various utility routines)

function:
	To post a message by interacting with submit.

parameters:
	none

globals:
	to, cc, subject, vcc, and bigbuf containing the message to be sent.
	nsent, number of successful posts

calls:
	various lm routines
	various utility routines immediately below

called by:
	input
 */

/*
**	R E V I S I O N  H I S T O R Y
**
**	03/31/83  GWH	Split the SEND program into component parts
**			This module contains post, do_message and
**			do_an_address.
**
*/

#include "./s.h"
#include "./s_externs.h"

post ()
{
    int     retval;

    if( badflg )	/*  died on previous attempt */
	if (rp_isbad (retval = mm_init ()) || rp_isbad (retval = mm_sbinit ()))
	   snd_abort ("Problem with mail restart [%s].\n", rp_valstr (retval));
    badflg = FALSE;
    do_message ();
    fflush (stdout);
}

do_message ()
{
    struct rp_bufstruct thereply;
    int	    len;
    int     retval;
    char    addr[ADDRSIZE];       /* Next address to process */

    if (to[0] != '\0')
    {
	if (bcc[0] != '\0')
	    printf (" --Open recipients--\n");
	if (rp_isbad (retval = mm_winit ((char *) 0, subargs, from)))
	    snd_abort ("Problem with mm_winit [%s].\n", rp_valstr (retval));
	if (rp_isbad (retval = mm_rrply (&thereply, &len)))
	    snd_abort ("Problem with mm_winit reply [%s].\n", rp_valstr (retval));
	switch (rp_gbval (thereply.rp_val)) {
	case RP_BNO:
	case RP_BTNO:
	    snd_abort ("mm_winit error: %s.\n", thereply.rp_line);
	}

	for (adrptr = to;; )
	    switch (next_address (addr))
	    {
		case -1:
		   goto nextto;

		case 0:
		    continue;

		 default:
		    do_an_address (addr);
	    }

    nextto:
	if (cc[0] != '\0')
	    for (adrptr = cc; ; )
		switch (next_address (addr))
		{
		    case -1:
		       goto nextcc;

		    case 0:
			continue;

		     default:
			do_an_address (addr);
		}
    nextcc:

	if (badflg)
	{
	    printf ("Make header legal and try again.\n");
	    if (rp_isbad (retval = mm_end (NOTOK)))
		    printf ("Can't terminate submission.\n");
	    longjmp (savej, 1);
	}

	if (rp_isbad (retval = mm_waend ()))
	    snd_abort ("Problem with mm_waend [%s].\n", rp_valstr (retval));
	if (rp_isbad (retval = do_text (NO)))
	    snd_abort ("Problem writing message text [%s].\n", rp_valstr (retval));
    }
    else
	printf (" no 'To:' addressees.\n");

    if (bcc[0] != '\0')
    {
	if (to[0] != '\0')
	    printf (" --Open copies posted--\n");
	printf (" --Blind recipients--\n");

	if (rp_isbad (retval = mm_winit ((char *) 0, subargs, from)))
	    snd_abort ("Problem with mail's mm_winit [%s].\n", rp_valstr (retval));
	if (rp_isbad (retval = mm_rrply (&thereply, &len)))
	    snd_abort ("Problem with mm_winit reply [%s].\n", rp_valstr (retval));
	switch (rp_gval (thereply.rp_val)) {
	case RP_BNO:
	case RP_BTNO:
	    snd_abort ("mm_winit error: %s.\n", thereply.rp_line);
	}

	for (adrptr = bcc;; )
	    switch (next_address (addr))
	    {
		case -1:
		   goto nextbcc;

		case 0:
		    continue;

		 default:
		    do_an_address (addr);
	    }

    nextbcc:

	if (badflg)
	{
	    printf ("Make header legal and try again.\n");
	    if (rp_isbad (retval = mm_end (NOTOK)))
		    printf ("Can't terminate submission.\n");
	    longjmp (savej, 1);
	}

	if (rp_isbad (retval = mm_waend ()))
	    snd_abort ("Problem with mm_waend [%s].\n", rp_valstr (retval));
	if (rp_isbad (retval = do_text (YES)))
	    snd_abort ("Problem writing message text [%s].\n", rp_valstr (retval));
    }
    if (to[0] != '\0' || bcc[0] != '\0')
	printf (" Message posted.\n\n");
    else
	printf (" *** No addresses to send to!\n\n");
}


do_an_address (addr)
char    addr[];
{
    struct rp_bufstruct thereply;
    int     retval;
    char    name[ADDRSIZE];
    char    address[ADDRSIZE];
    char    hostname[FILNSIZE];
    int     len;

    name[0] =
	address[0]=
	hostname[0] = '\0';
    parsadr (addr, name, address, hostname);

    putchar (' ');
    if (name[0] != '\0')
	printf ("%s <", name);
    printf ("%s", address);
    if (hostname[0] != '\0' && !lexequ(hostname, locname))
	printf ("@%s", hostname);
    if (name[0] != '\0')
	putchar ('>');
    printf (":  ");
    fflush (stdout);

    if (rp_isbad (retval = mm_wadr (hostname, address)) ||
	    rp_isbad (retval = mm_rrply (&thereply, &len)))
	snd_abort ("Problem in do_an_address [%s].\n", rp_valstr (retval));
    switch (rp_gval (thereply.rp_val))
    {
	case RP_AOK: 
	    printf ("address ok\n");
	    nsent++;
	    break;

	case RP_DOK: 
	    printf ("Nameserver timeout; accepted for resubmission\n");
	    nsent++;
	    break;

	case RP_NO: 
	    printf ("not deliverable; unknown problem\n");
	    badflg = TRUE;
	    break;

	case RP_USER: 
	    printf ("not deliverable; unknown address.\n");
	    badflg = TRUE;
	    break;

	case RP_NDEL: 
	    printf ("not deliverable; permanent error.\n");
	    badflg = TRUE;
	    break;

	case RP_AGN: 
	    printf ("failed, this attempt; try later\n");
	    badflg = TRUE;
	    break;

	case RP_NOOP: 
	    printf ("not attempted, this time; perhaps try later.\n");
	    badflg = TRUE;
	    break;

	default: 
	    snd_abort ("Unexpected address response:  [%s] %s\n",
			rp_valstr (thereply.rp_val), thereply.rp_line);
    }
    fflush (stdout);
}
