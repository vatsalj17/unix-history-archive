#include "util.h"
#include "mmdf.h"
#include "ch.h"
#include "adr_queue.h"
#include "msg.h"
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

/*      Jun 80  D. Crocker      RP_USER -> RP_NDEL on no valid users
 *      Nov 80  D. Crocker      Reduce verbosity if more than 12 addrs.
 *      Jun 82  D. Crocker      null chan -> source spec
 *      Jul 82  D. Crocker      qf_ -> mq_
 */
extern struct ll_struct *logptr;
extern Chan **ch_tbsrch;         /* for sorting addrs by channel       */
extern char *strdup ();
extern char *mgt_return;         /* sender's return address            */
extern short tracing;
extern int  lnk_listsize;

struct lnk_struct
{
    struct lnk_struct   *lnk_nxt; /* where the next entry is            */
    struct ch_struct    *lnk_ch;  /* what delivery pgm to run           */
    short     lnk_chpri;            /* "priority" of chan in ch_tbsrch[]  */
    char   *lnk_host;             /* host part of address               */
    char   *lnk_mbox;             /* mailbox part of address            */
};

short  lnk_nadrs;            /* current number of addresses        */
short  lnk_nentries;         /* current number of entries in list  */

/* ***************  (lnk_)  ADDRESS LINKED-LIST PRIMITIVES  *********** */

LOCVAR struct lnk_struct    lnk_strt;
				  /* 1st address in list                */

lnk_adinfo (thechan, hostr, mbox) /* given constituents, add to list    */
Chan *thechan;                 /* internal chan name/code            */
char *hostr,                   /* official name of host              */
     *mbox;                    /* name of mailbox                    */
{
    extern char *ut_alloc ();
    register struct lnk_struct *tstadr;
    register short ch_ind;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "lnk_adinfo ()");
#endif

    /*NOSTRICT*/
    tstadr = (struct lnk_struct *) ut_alloc (sizeof (struct lnk_struct));
    if ((tstadr -> lnk_ch = thechan) == (Chan *) 0)
	ch_ind = -1;            /* really a source specification        */
    else
    {
#ifdef DEBUG
	ll_log (logptr, LLOGFTR, "lnk_adinfo (%s(->%s), %s, %s)",
		thechan -> ch_name, thechan -> ch_queue,
		hostr, mbox);
#endif
    	if (tracing)
	    printf("queueing for %s: via '%s': '%s'\n",
		thechan -> ch_name, hostr, mbox);
	for (ch_ind = 0; ch_tbsrch[ch_ind] != 0 &&
		ch_tbsrch[ch_ind] != thechan; ch_ind++);
			      /* chan's "priority" is its position  */
    }
    tstadr -> lnk_chpri = ch_ind;
    tstadr -> lnk_host = strdup (hostr);
    tstadr -> lnk_mbox = strdup (mbox);

    return (lnk_insrt (tstadr));
}
/**/

LOCFUN
        lnk_insrt (newadr)	  /* insert addr into list, sorted      */
register struct lnk_struct *newadr;
{
    register struct lnk_struct *curadr,
                               *lastptr;

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "lnk_insrt ()");
#endif
    for (curadr = lnk_strt.lnk_nxt, lastptr = &lnk_strt;
	    curadr != 0; lastptr = curadr, curadr = curadr -> lnk_nxt)
    {				  /* skip until new entry is "smaller"  */
	switch (lnk_cmpr (newadr, curadr))
	{
	    case 1: 		  /* newadr > curadr                    */
		continue;

	    case 0: 		  /* duplicate entries are ignored      */
		return (OK);      /* noop                               */
	}
	break;			  /* returned -1 => curadr > newadr     */
    }				  /* otherwise tack on end of list      */
    lnk_nentries++;               /* any kind of entry                  */
    if (newadr -> lnk_ch != (Chan *) 0)
	lnk_nadrs++;              /* actual addresses                   */
    lastptr -> lnk_nxt = newadr;  /* insert after last one smaller      */
    newadr -> lnk_nxt = curadr;   /*    than new one                    */
    return (DONE);                /* really did something               */
}

/*
 *	In order to avoid O(n*2) sorting behavior, this comparison
 *	routine will cause the addresses to be sorted in reverse
 *	alphabetical order.  The address list is reversed before
 *	being written out.
 */
LOCFUN
        lnk_cmpr (adr1, adr2)     /* determine sort relationship        */
register struct lnk_struct *adr1,
                           *adr2;
{				  /* sort on chan, host, then mailbox   */
    register short    adr_rel;      /* relationship between addresses     */

    adr_rel = adr2 -> lnk_chpri - adr1 -> lnk_chpri;
				  /* compare "priorities" of channels   */
    if (adr_rel < 0)
	return (-1);		  /* chan2 > chan1                      */

    if (adr_rel > 0)
	return (1);		  /* chan1 > chan2                      */

    if ((adr_rel = lexrel (adr2 -> lnk_host, adr1 -> lnk_host)) != 0)
	return (adr_rel);	  /* host spec was different            */

    return (lexrel (adr2 -> lnk_mbox, adr1 -> lnk_mbox));
				  /* well, maybe different mailboxes    */
}
/**/

LOCFUN struct lnk_struct   *
                            lnk_1free (adr)
				  /* free storage for an entry          */
register struct lnk_struct *adr;
{
    register struct lnk_struct *nxtadr;

    if (adr == 0)
	return (0);		  /* ain't nothin' here                 */
    free (adr -> lnk_mbox);
    free (adr -> lnk_host);
    nxtadr = adr -> lnk_nxt;
    free ((char *) adr);
    return (nxtadr);
}

lnk_freeall ()                    /* step through list and free storage */
{
    register struct lnk_struct *curadr;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "lnk_freeall()");
#endif
    for (curadr = lnk_strt.lnk_nxt; curadr != 0; curadr = lnk_1free (curadr));
    lnk_strt.lnk_nxt = 0;         /* indicate end-of-list at base       */
    lnk_nadrs = 0;
}

lnk_filall (flags)         /* copy the list out to the file      */
{
    register struct lnk_struct *prev;
    register struct lnk_struct *cur;
    register struct lnk_struct *next;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "lnk_filall:  %d addrs...", lnk_nadrs);
#endif

    if (lnk_nadrs <= 0 || lnk_strt.lnk_nxt == 0)
	err_msg (RP_NDEL, "no addressees!");

    if (lnk_nadrs > lnk_listsize)      /* long lists => less verbosity     */
	mq_adinit (NO, flags | ADR_CITE, mgt_return);
    else                            /* initialize address list queue    */
	mq_adinit (YES, flags, mgt_return); /* permit delay warning message */

    /*  Invert the list to get it back in alphabetical order  */
    prev = lnk_strt.lnk_nxt;
    cur = prev->lnk_nxt;
    prev->lnk_nxt = 0;
    while (cur != 0) {
	next = cur->lnk_nxt;
    	cur->lnk_nxt = prev;
    	prev = cur;
    	cur = next;
    }
    lnk_strt.lnk_nxt = prev;

				/* step through list & store entries  */
    for (cur = lnk_strt.lnk_nxt; cur != 0; cur = cur -> lnk_nxt)
	if (cur -> lnk_ch != (Chan *) 0)
	    mq_adwrite (cur -> lnk_ch, cur -> lnk_host, cur -> lnk_mbox);
}
