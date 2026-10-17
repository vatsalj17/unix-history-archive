/*
** 		D O _ *
**
**
**	R E V I S I O N  H I S T O R Y
**
**	03/31/83  GWH	Split the SEND program into component parts
**			This module contains do_text, do_hdr,
**			next_address, and snd_abort.
**
**	05/31/83  GWH	Added code for the updating of a copyfile.
*/
#include "./s.h"
#include "./s_externs.h"

int     sentfd;

do_text (isbcc)
    int isbcc;                    /* true, if bcc copy                  */
{
    extern char *cnvtdate ();
    struct header *hp;
    struct rp_bufstruct thereply;
    int     retval;
    int     i;
    int     len;
    char    date[32];
	if( cflag > 0){
	    if ((sentfd = open (copyfile, 1)) >= 0)
		lseek (sentfd, 0l, 2);    /* set up to save a copy           */
	    else
		sentfd = creat (copyfile, sentprotect);
	    if (sentfd >= 0)              /* terminate saved copy            */
		write (sentfd, delim1, strlen (delim1));
	}
    cnvtdate (TIMREG, date);      /* arpanet style date/time            */
    do_hdr (datename, date);
    do_hdr (fromname, signature);

    if (to[0] == '\0')
	do_hdr (toname, "list: ;"); /* no To: addresses, so fake it       */
    else
	do_hdr ((isbcc) ? BCtoname : toname, to);

    if (cc[0] != '\0')
	do_hdr ((isbcc) ? BCccname : ccname, cc);

    if (subject[0] != '\0')
	do_hdr (subjname, subject);

    if (isbcc && to[0] != '\0')   /* put empty Bcc: field in bcc copies */
	do_hdr (bccname, "(Private)");
				  /* when there also are To addresses   */
    for (hp = headers; hp != NULL; hp = hp->hnext)
	if (isstr(hp->hdata))
	    do_hdr (hp->hname, hp->hdata);

    mm_wtxt ("\n", 1);           /* blank line separate headers & body */
	if( cflag > 0 ) {
	    if (sentfd >= 0)
		write (sentfd, "\n", 1);
	}

    dropen (DRBEGIN);

    while ((i = read (drffd, bigbuf, BBSIZE - 1)) > 0)
    {                             /* copy the body                      */
	if (rp_isbad (retval = mm_wtxt (bigbuf, i)))
	    snd_abort ("Problem with writing text buffer [%s].\n", rp_valstr (retval));
	if( cflag > 0 ){
		if (sentfd >= 0)	  /* save the body                  */
		    write (sentfd, bigbuf, i);
	}
    }

	if(cflag > 0 ){
	    if (sentfd >= 0)              /* terminate saved copy           */
	    {
		if (bigbuf[i - 1] != '\n')
		    write (sentfd, "\n", 1);
					 /* make sure it ends with newline  */
		write (sentfd, delim2, strlen (delim2));
		close (sentfd);
	    }
	}

    if (rp_isbad (retval = mm_wtend ()))
	snd_abort ("Problem with ending write of text buffer [%s].\n", rp_valstr (retval));
    if (rp_isbad (retval = mm_rrply (&thereply, &len)))
	snd_abort ("Problem reading reply to text write [%s].\n", rp_valstr (retval));
    switch (rp_gval (thereply.rp_val))
    {                             /* how did MMDF like it?              */
	case RP_OK: 
	case RP_MOK: 
	    break;		  /* Text ok */

	case RP_NO: 
	    snd_abort ("Error in text transmission : General purpose no. RP_NO.\n");
	    break;

	case RP_NDEL: 
	    snd_abort ("Error in text transmission : Could not deliver. RP_NDEL.\n");
	    break;

	case RP_AGN: 
	    snd_abort ("Error in text transmission : Not now, maybe later. RP_AGN.\n");
	    break;

	case RP_NOOP: 
	    snd_abort ("Error in text transmission : Nothing done, this time.\n");
	    break;

	default: 
	    snd_abort (" Unexpected text response:  [%s] %s\n",
			rp_valstr (thereply.rp_val), thereply.rp_line);
    }
    return (RP_OK);
}

do_hdr (name, data)               /* send out a header field            */
    char name[],                  /* name of field                      */
	*data;                    /* value-part of field                */
{
    int retval;
    register int ind;
    register int thesize;
    register char *curptr;

    for (curptr = data, ind = 0; ind >= 0; curptr += ind + 1)
    {
	if ((ind = strindex ("\n", curptr)) >= 0)
	    curptr[ind] = '\0';   /* format lines properly              */

	sprintf (bigbuf, hdrfmt, (curptr == data) ? name : "", curptr);
				  /* begin with blank on added lines    */
	if (rp_isbad (retval = mm_wtxt (bigbuf, (thesize = strlen (bigbuf)))))
	    snd_abort ("Problem with writing header buffer [%s].\n", rp_valstr (retval));

	if(cflag > 0 ){
		if (sentfd >= 0)          /* save the message?                  */
		    write (sentfd, bigbuf, thesize);
	}

	if (ind >= 0)
	    curptr[ind] = '\n';   /* put it back                        */
    }
}


next_address (addr)
char    *addr;
{
    int     i = -1;               /* return -1 = end; 0 = empty         */

    for (;;)
	switch (*adrptr)
	{
	    default: 
		addr[++i] = *adrptr++;
		continue;

	    case '"':
		for (addr[++i] = *adrptr++; !isnull (*adrptr); )
		{
		    addr[++i] = *adrptr;
		    if (*adrptr++ == '"')
			break;
		}
		continue;

	    case '(':
		for (addr[++i] = *adrptr++; !isnull (*adrptr); )
		{
		    addr[++i] = *adrptr;
		    if (*adrptr++ == ')')
			break;
		}
		continue;

	    case '\n':
	    case ',': 
		addr[++i] = '\0';
		adrptr++;
		return (i);

	    case '\0': 
		if (i >= 0)
		    addr[++i] = '\0';
		return (i);
	}
}

/* VARARGS1 */

snd_abort (fmt, b, c, d, e, f)
char   fmt[], b[], c[], d[], e[], f[];
{
    printf ("\n\t");
    printf (fmt, b, c, d, e, f);
    fflush (stdout);
    longjmp (savej, 1);
}
