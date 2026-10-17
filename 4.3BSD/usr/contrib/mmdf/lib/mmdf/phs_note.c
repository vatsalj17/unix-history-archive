#include "util.h"
#include "mmdf.h"
#include <sys/stat.h>
#include "ch.h"
#include "phs.h"

/*      note mmdf channel activity phases               */

extern Llog *logptr;
extern char *phsdfldir;                /* directory for timestamps */

#define PHS_NUL 0
#define PHS_SND 1
#define PHS_RCV 2

LOCVAR  int phs_mode;                   /* are sending or receiving */
LOCVAR time_t starttime;

LOCVAR char
	    cnstrt[] =  "%s/%s/cstrt",
	    cngot[] =   "%s/%s/cgot",
	    cnend[] =   "%s/%s/cend",
	    restrt[] =  "%s/%s/rstrt",
	    remsg[] =   "%s/%s/rmsg",
	    reend[] =   "%s/%s/rend",
	    wrstrt[] =  "%s/%s/wstrt",
	    wrmsg[] =   "%s/%s/wmsg",
	    wrend[] =   "%s/%s/wend";
/**/
phs_note (thechan, phase)               /* make a timestamp */
    Chan *thechan;
    int phase;
{
    char stamploc[LINESIZE];
    char stampdir[LINESIZE];
    char *fmt;

    switch (phase)
    {
	case PHS_CNSTRT:
		ll_log (logptr, LLOGPTR, "strt");
		fmt = cnstrt;
		/*NOSTRICT*/
		starttime = 0L;
		break;

	case PHS_CNGOT:
		ll_log (logptr, LLOGPTR, "conn");
		fmt = cngot;
		time (&starttime);
		break;

	case PHS_CNEND:
		phs_mode = PHS_NUL;
		/*NOSTRICT*/
		starttime = 0L;
		fmt = cnend;
		break;

	case PHS_RESTRT:
		ll_log (logptr, LLOGPTR, "rcv");
		fmt = restrt;
		phs_mode = PHS_RCV;
		break;

	case PHS_REMSG:
		fmt = remsg;
		break;

	case PHS_REEND:
		ll_log (logptr, LLOGPTR, "rend");
		fmt = reend;
		phs_mode = PHS_NUL;
		break;

	case PHS_WRSTRT:
		ll_log (logptr, LLOGPTR, "writ");
		fmt = wrstrt;
		phs_mode = PHS_SND;
		break;

	case PHS_WRMSG:
		fmt = wrmsg;
		break;

	case PHS_WREND:
		ll_log (logptr, LLOGPTR, "wend");
		fmt = wrend;
		phs_mode = PHS_NUL;
		break;
    }

    sprintf (stamploc, fmt, phsdfldir, thechan -> ch_name);

    /* We rely on umask() == 0 */
    if (close (creat (stamploc, 0666)) < 0) {
      sprintf(stampdir, "%s/%s", phsdfldir, thechan -> ch_name);
      if ( creatdir (stampdir, 0777, 0, 0) != OK ||
           (close (creat (stamploc, 0666)) < 0) )
          return (NOTOK);
    }
    return(0);
}
/**/

phs_msg  (thechan, naddrs, len)     /* note trasmission of 1 message    */
    Chan *thechan;
    int naddrs;
    long len;
{
    if (naddrs <= 0 && len <= 0L)
	return;                     /* nothing to record                */

    switch (phs_mode)
    {
	case PHS_RCV:
	    ll_log (logptr, LLOGFST, "rmsg %4da %10ldc", naddrs, len);
	    phs_note (thechan, PHS_REMSG);      /* make a timestamp */
	    break;

	case PHS_SND:
	    ll_log (logptr, LLOGFST, "wmsg %4da %10ldc", naddrs, len);
	    phs_note (thechan, PHS_WRMSG);      /* make a timestamp */
	    break;
    }
}
/**/

phs_end  (thechan, status)      /* note end of session */
    Chan *thechan;
    int status;                 /* mmdf end value */
{
    time_t endtime;

    if (starttime == 0L)
	ll_log(logptr, LLOGBST, "end (%s)", rp_valstr (status));
    else
    {
	time (&endtime);
	ll_log (logptr, LLOGBST, "end (%s)\t%lds", rp_valstr (status),
				 (long) (endtime - starttime));
	/*NOSTRICT*/
	starttime = 0L;
    }

    phs_note (thechan, PHS_CNEND);      /* make a timestamp */
}

/**/

time_t
	phs_get (thechan, phase)           /* read a timestamp */
    Chan *thechan;
    int phase;
{
    struct stat statbuf;
    char stamploc[LINESIZE];
    char *fmt;

    switch (phase)
    {
	case PHS_CNSTRT:
		fmt = cnstrt;
		break;

	case PHS_CNGOT:
		fmt = cngot;
		break;

	case PHS_CNEND:
		fmt = cnend;
		break;

	case PHS_RESTRT:
		fmt = restrt;
		break;

	case PHS_REMSG:
		fmt = remsg;
		break;

	case PHS_REEND:
		fmt = reend;
		break;

	case PHS_WRSTRT:
		fmt = wrstrt;
		break;

	case PHS_WRMSG:
		fmt = wrmsg;
		break;

	case PHS_WREND:
		fmt = wrend;
		break;
    }

    sprintf (stamploc, fmt, phsdfldir, thechan -> ch_name);

    if (stat (stamploc, &statbuf) < 0)
	return (0L);

    /*NOSTRICT*/
    return (statbuf.st_mtime);
}
