#include "util.h"
#include "mmdf.h"

/* send RP_DHST replies back to Deliver, if foreign site goes down */

/*  The following state diagram is for the qu2??_send functions.  The concern
 *  is for making sure that Deliver properly receives replies, in the event
 *  that the remote host goes dead.  For simplicity, a modification of the
 *  following is used, to keep the state variable local to this file.
 *
 *      The parenthesized tokens are error-condition actions, to be taken
 *  within qu_fakrply():  ABORT=abort/return, RINIT=qu_rinit, RDADR=qu_radr,
 *                        ARPLY=address ??_wrply, TRPLY=text ??_wrply
 *
 *              (ABORT)
 *                 |
 *            qu_init
 *                 \---->--(RINIT)--->  ??_init
 *            qu_pkinit <--(ABORT)---<---/
 *                 \---->--(RINIT)--->  ??_sbinit
 *                                       |
 *     /->-->   (ABORT) <------------<---/
 *     |           |
 *     |      qu_rinit
 *     |           |
 *     |         [ok] -->--(RDADR)--->  ??_winit ->-\
 *     |         [end]                               |
 *     |           |                                 |
 *     |      qu_pkend                  ----------   |
 *     |           \====>============>  |??_sbend|   |
 *     |                                ----------   |
 *     |  /->   (ABORT) <------------<-------------<-/
 *     |  |        |
 *     |  |     QU_RADR
 *     |  |        |
 *     |  |      [end]-->--(TRPLY)--->  ??_waend ->-\
 *     |  |        |                                 |
 *     |  |      [ok] -->--(ARPLY)--->  ??_wadr      |
 *     |  |                              |           |
 *     |  |                             ??_rrply     |
 *     |  |    QU_WRPLY <--(ABORT)-------/           |
 *     |  \-<------/                                 |
 *     |                                             |
 *     |      qu_rinit <------------<-------------<-/
 *     |           |
 *     |      qu_rtxt  <------------<-------------<-\
 *     |           |                                 |
 *     |         [ok] -->------------>  ??_wtxt  ->-/
 *     |           |
 *     |         [end]-->------------>  ??_wend
 *     |                                 |
 *     |                                ??_rrply
 *     |       QU_WRPLY <------------<---/
 *     \-<---------/
 */

#include "chan.h"

extern struct ll_struct *logptr;

LOCVAR struct rp_construct
			    rp_fdone =
{
    RP_DHST, 'D', 'O', 'N', 'E', ' ', '&', ' ', 's', 't', 'i', 'l',
    'l', ' ', 'g', 'o', 'n', 'e', '\0'
}          ,
			    rp_fstill =
{
    RP_DHST, 's', 't', 'i', 'l', 'l', ' ', 'g', 'o', 'n', 'e', '\0'
};

qu_fakrply (snd_state)            /* send RP_DHST's to Deliver          */
    int snd_state;
{				  /* behavior depends on state          */
    char    host[LINESIZE],
	    adr[LINESIZE],
            info[LINESIZE],
            sender[LINESIZE];

#ifdef DEBUG
    ll_log (logptr, LLOGPTR, "qu_fakrply");
#endif

    switch (snd_state)
    {
	case SND_ABORT:
#ifdef DEBUG
	    ll_log (logptr, LLOGBTR, "SND_ABORT");
#endif
	    return;

	case SND_RINIT:
	    goto rinit;

	case SND_ARPLY:
	    goto arply;

	case SND_TRPLY:
	    goto mrply;
    }

    FOREVER
    {
	switch (rp_gval (qu_radr (host, adr)))
	{
	    case RP_DONE:         /* list done, say ok to text          */
	    mrply:
		qu_wrply ((struct rp_bufstruct *) &rp_fdone,
						    rp_conlen (rp_fdone));
	    rinit:
		if (rp_gval (qu_rinit (info, sender)) != RP_OK)
		{
#ifdef DEBUG
		    ll_log (logptr, LLOGBTR, "fake DONE");
#endif
		    return;
		}
		break;

	    case RP_OK:           /* say ok to an address               */
	    arply:
#ifdef DEBUG
		ll_log (logptr, LLOGFTR, "faking '%s'", adr);
#endif
		qu_wrply ((struct rp_bufstruct *) &rp_fstill,
						    rp_conlen (rp_fstill));
		break;

	    default: 
		ll_log (logptr, LLOGTMP, "error value in loop");
		return;
	}
    }
}
