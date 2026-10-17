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
/* for channel Q NIFTP returning list of duff addresses to sender       */
/*                                                                      */
/*      Steve Kille     August 1982                                     */

#include "msg.h"
#include "adr_queue.h"


extern char *multcat();
extern struct ll_struct *logptr;

extern char *supportaddr;        /* supoport address for mail messages */
extern char *locname;
extern char *locdomain;
#ifdef UCL_TSNAME
extern char TSname[];
#endif UCL_TSNAME

LOCVAR char rtn_sndel[] = "Failed mail";


struct rtn_str
{
    struct rtn_str  *r_link;
    char            *r_buf;
};

LOCVAR struct rtn_str
	*rtn_bad = (struct rtn_str *) 0,
	*rtn_ok = (struct rtn_str *) 0;
/**/

rtn_init ()
				/* clean up any debris                  */
{
    struct rtn_str *ptr;

    while (rtn_ok != 0)
    {
	ptr = rtn_ok;
	free (ptr -> r_buf);
	rtn_ok = rtn_ok -> r_link;
	free ( (char *)ptr);
    }

    while (rtn_bad != 0)
    {
	ptr = rtn_bad;
	free (ptr -> r_buf);
	rtn_bad = rtn_bad -> r_link;
	free ( (char *)ptr);
    }
}

/**/

rtn_adr (adr, good)
char    *adr;
{
    struct rtn_str *rptr;
    extern char *malloc();

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "rtn_adr (%s, %d)", adr, good);
#endif

    rptr = (struct rtn_str *) malloc (sizeof (struct rtn_str));
    if (rptr == (struct rtn_str *) 0)
	return (RP_NO);

    rptr -> r_buf = multcat ("\t", adr, "\n", (char *)0);
    if (rptr ->  r_buf == (char *) NOTOK)
	return (RP_NO);

    if (good)
    {
	rptr -> r_link =  rtn_ok;
	rtn_ok = rptr;
    }
    else
    {
	rptr -> r_link = rtn_bad;
	rtn_bad = rptr;
    }

    return (RP_OK);
}



/**/

rtn_errmsg (sender, rtn_fp)     /* Fire of list ofbad addresses to sender */
char    *sender;                /* sender of message                    */
FILE    *rtn_fp;                /* pointer to message file              */
{

    if (rtn_bad  == (struct rtn_str *) 0)    /* No error addresses built up */
	return (OK);


#ifdef DEBUG
    ll_log (logptr, LLOGPTR, "rtn_errmsg (%s)", sender);
#endif

				/* If sender not specified              */
				/* generate default                     */
				/* May throw away later if this becomes */
				/* a problem                            */
    if (sender[0] == '\0')
	strcpy (sender, supportaddr);

    if (rtn_mlinit (rtn_sndel, sender) != OK)
	return (NOTOK);           /* set up for returning               */

    rtn_nogood ();               /* give list of bad addresses     */

    rtn_cite (rtn_fp, 500);

    return (ml_end (OK));
}
/**/

rtn_ack (ackadr, rtn_fp)     /* Fire of list ofbad addresses to ackadr */
char    *ackadr;                /* ackadr of message                    */
FILE    *rtn_fp;                /* pointer to message file              */
{



#ifdef DEBUG
    ll_log (logptr, LLOGPTR, "rtn_ack (%s)", ackadr);
#endif

				/* If ackadr not specified              */
				/* generate default                     */
				/* May throw away later if this becomes */
				/* a problem                            */
    if (ackadr == (char *) 0 || ackadr[0] == '\0')
	return (OK);    /* no address to ack to */

    if (rtn_mlinit ("Message Acknowledgement", ackadr) != OK)
	return (NOTOK);           /* set up for returning               */

    rtn_good ();
    rtn_nogood ();

    rtn_cite (rtn_fp, 25);

    return (ml_end (OK));
}
/**/

LOCFUN
    rtn_nogood ()
			/*  handle bad addresses                        */
{
    struct rtn_str *ptr;

    if (rtn_bad == 0)
	return;

#ifdef	UCL_TSNAME
    if(*TSname == '+') {
	ml_txt("\nWARNING: Cannot reverse translate: <");
	ml_txt(TSname);
	ml_txt("> from the NRS database\n");
    }
#endif	UCL_TSNAME
    ml_txt ("\nYour message was not delivered to the following addresses: \n\n");

    for (ptr = rtn_bad; ptr != 0; ptr = ptr -> r_link) {
	to80 (ptr -> r_buf);
	ml_txt (ptr -> r_buf);
    }

    ml_txt ("\n\n");
}

to80(from)
char *from;
{
	register char   *p;
	char    *lastspace = NULL, *lastnl = from;

	for(p = from ; *p ; p++){
		if(*p != ' ' && *p != '\t')
			continue;
		if( p - lastnl > 80){
			if(lastspace != NULL){
				*lastspace = '\n';
				lastnl = lastspace;
				if(p - lastnl <= 80){
					 lastspace = p;
					continue;
				}
			}
			lastspace = NULL;
			*p = '\n';
			lastnl = p;
			continue;
		}
		lastspace = p;
	}
	if(p - lastnl > 80 && lastspace != NULL)
		*lastspace = '\n';
}

LOCFUN
    rtn_good ()
			/* handle good addresses for ack        */
{
    struct rtn_str *ptr;

    if (rtn_ok == 0)
	return;

    ml_txt ("\nYour message was accepted by: ");
    ml_txt (locname);
    ml_txt (".");
    ml_txt (locdomain);
    ml_txt ("\n    for delivery to the following addresses:\n\n");

    for (ptr = rtn_ok; ptr != 0; ptr = ptr -> r_link) {
	to80 (ptr -> r_buf);
	ml_txt (ptr -> r_buf);
    }

    ml_txt ("\n\n");
}


/**/

LOCFUN
	rtn_mlinit (subject, sender)
char    subject[];
char    sender[];
{
    extern char *sitesignature;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "rtn_mlinit (%s)", subject);
#endif

    if (isnull (sender[0]))     /* no return address                  */
    {
	ll_log (logptr, LLOGTMP, "no return address");
	return (NOTOK);
    }
    if (ml_1adr (NO, NO, sitesignature, subject, sender) != OK)
    {                             /* no return, no Sender:              */
	ll_err (logptr, LLOGTMP, "ml_1adr");
	return (NOTOK);
    }
    return (OK);
}
/**/

rtn_cite (rtn_fp, count)
FILE    *rtn_fp;
int     count;
{
    short     lines;
    char    linebuf[LINESIZE];

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "rtn_cite ()");
#endif

    ml_txt ("\n    Your message begins as follows:\n\n");

    rewind (rtn_fp);

				/* Skip JNT mail header                 */
    while (fgets (linebuf, sizeof linebuf, rtn_fp) != NULL)
    {
	if (linebuf[0] == '\n')
	    break;
    }

    while (fgets (linebuf, sizeof linebuf, rtn_fp) != NULL)
    {
	ml_txt (linebuf);         /* do headers                           */
	if (linebuf[0] == '\n')
	    break;
    }

    if (ferror (rtn_fp))
	err_abrt (RP_FIO, "Error reading text for return-to-msg_sender.");

    if (!feof (rtn_fp))
    {
	for (lines = count; --lines > 0 &&
		fgets (linebuf, sizeof linebuf, rtn_fp) != NULL;)
	{
	    if (linebuf[0] == '\n')
		lines++;          /* truly blank lines don't count      */
	    ml_txt (linebuf);
	}
	if (!feof (rtn_fp))       /* if more, give an elipses           */
	    ml_txt ("...\n");
    }
}

