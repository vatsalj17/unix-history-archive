
/* timeouts for smtpsrvr */
#define NTIMEOUT        180     /*  3 minute timeout on net I/O  */
#define DTIMEOUT        600     /*  10 minute timeout on submit I/O */

/* smtp timeouts */
#define SM_TIME		20		/* default timeout */
#define SM_HTIME        SM_TIME		/* Time allowed for HELO command */
#define SM_OTIME        40	     	/* Time allowed for a netopen */

/*
 * for waiting for first response after connection
 * give a lot of time to first MX, then back off
 */

#define SM_ATIME	240		/* Time allowed for answer: 1st open */
#define SM_ATMIN	90		/* Time allowed for answer: minimum */
#define SM_ATINC	30		/* backoff increment */

#define SM_STIME        120      	/* Time allowed for MAIL command */
#define SM_TTIME        300      	/* Time allowed for RCPT command */
#define SM_QTIME        SM_TIME		/* Time allowed for QUIT command */

#define SM_RTIME        SM_TIME		/* Time allowed for a RSET command */
#define SM_DTIME        SM_TIME		/* Time allowed for a DATA command */
#define SM_PTIME        120     	/* Time allowed for the "." command */
#define SM_BTIME        120     	/* Time allowed for a block of text */


struct sm_rstruct {      /* save last reply obtained           */
    int     sm_rval;            /* rp.h value for reply               */
    int     sm_rlen;            /* current lengh of reply string      */
    char    sm_rgot;            /* true, if have a reply              */
    char    sm_rstr[LINESIZE];  /* human-oriented reply text          */
};
