# include  "util.h"
# include  "ll_log.h"
# include  "d_proto.h"
# include  "d_returns.h"
# include  "d_structs.h"

/*	May 82  D. Crocker  added startup newline, to bypass some noise
 */

extern int errno;
extern int d_errno;

/*
 *     D_SLVCONN
 *
 *     this routine is called by the slave protocol module to acknowledge
 *     a connection by the remote system.  it sets the port characteristics
 *     and performs the protocl startup sequence.
 *
 *     logfile -- name of the log file
 *
 *     tson -- set to non-zero to indicate that a transcript of the connection
 *             should be kept
 *
 *     tsfile -- name of the file in which the transcript should be kept
 *
 *     debug -- set to non-zero to enable debug logging
 *
 *     portfd -- file descriptor of the port
 *
 *     maxrecv -- maximum receive packet length
 *
 *     maxxmit -- maximum transmit packet length
 *
 */

d_slvconn (logfile, tson, tsfile, debug, portfd, maxrecv, maxxmit)
struct ll_struct *   logfile;
char   tsfile[];
int     tson,
        debug,
        portfd,
        maxrecv,
        maxxmit;
{
    extern int  d_master,
                d_debug,
		d_snseq,
		d_rcvseq,
		d_lrmax,
		d_lxmax;
    extern  FILE * d_prtfp;
    register int    result;

    /*  open the log file and transcript  */
    if ((result = d_opnlog (logfile)) < 0)
	return (result);

    if (tson)
	if ((result = d_tsopen (tsfile)) < 0)
	    return (result);

    /*  set up globals  */
    d_debug = debug;
    d_master = 0;
    d_snseq = 3;		/* these are incremented before any ... */
    d_rcvseq = 3;		/* ... packets are expected */
    d_lrmax = maxrecv;
    d_lxmax = maxxmit;

    /* use stdio for input and write() calls for output */
    d_prtfp = fdopen (portfd, "r");

    /*  save current tty parameters and and set tty for protocol mode       */
    if (d_ttsave (d_prtfp, NULL) < 0 || d_ttproto (0) < 0)
    {                             /* don't abort; may be ok             */
#ifdef D_LOG
	d_log ("d_slvconn", "error getting port parameters (errno=%d)",
			errno);
#endif D_LOG
	d_errno = D_PRTSGTTY;
    }


    /*  do the protocol startup sequence  */
    result = d_slvstart ();
    return (result);
}

/*
 *     D_SLVDROP
 *
 *     this routine is called to cause the connection from the master to
 *     be dropped.
 *
 *     sndquit -- called with non-zero value if the slave should attempt a
 *                a QUIT exchange.
 */

d_slvdrop (sndquit)
int     sndquit;
{
    register int    result;

    if (sndquit)
	result = d_snquit ();

#ifdef D_LOG
    d_log ("d_slvdrop", "slave dropped connection");
#endif D_LOG

    if (d_ttrestore () < 0)
    {                             /* restory tty characteristics        */
	d_errno = D_PRTSGTTY;
    }
    d_tsclose ();
    d_clslog ();

    return (result);
}

/*
 *     D_SLVSTART
 *
 *     this routine is called in the slave to start the protocol.
 */

d_slvstart ()
{
    extern int  d_lxmax,
		d_lrmax,
		d_rxmax,
		d_rrmax,
                d_maxtext;
    extern char d_snesc,
		d_rcvesc;
    extern  unsigned short d_lxill[],
	    d_lrill[],
	    d_rxill[],
	    d_rrill[],
	    d_lcvec[];
    register int    result;

#ifdef D_DBGLOG
    d_dbglog ("d_slvstart", "Beginning slave host startup sequence");
#endif D_DBGLOG

    sleep (2);  /* wait for line-settling */

    d_wrtport ("\n", 1);  /* hack to bypass initial noise */

    /*  send XPATH and RPATH to the master  */
    if ((result = d_snpath (XPATH, d_lxmax, d_lxill))
	    < 0)
	return (result);

#ifdef D_DBGLOG
    d_dbglog ("d_slvstart", "XPATH sent ok.");
#endif D_DBGLOG

    if ((result = d_snpath (RPATH, d_lrmax, d_lrill))
	    < 0)
	return (result);

#ifdef D_DBGLOG
    d_dbglog ("d_slvstart", "RPATH sent ok.");
#endif D_DBGLOG

    /*  get XPATH and RPATH from the master.  then form the
     *  local transmission encoding vector.
     */
    if ((result = d_getpath (XPATH, &d_rxmax, d_rxill)) < 0)
	return (result);

#ifdef D_DBGLOG
    d_dbglog ("d_slvstart", "XPATH received ok.");
#endif D_DBGLOG

    if ((result = d_getpath (RPATH, &d_rrmax, d_rrill)) < 0)
	return (result);

#ifdef D_DBGLOG
    d_dbglog ("d_slvstart", "RPATH received ok.");
#endif D_DBGLOG

    d_orbitvec (d_lxill, d_rrill, d_lcvec);

    d_maxtext = d_minimum (d_lxmax, d_rrmax);
#ifdef D_DBGLOG
    d_dbglog ("d_slvstart", "Maximum transmit packet size set to %d", d_maxtext);
#endif D_DBGLOG
    d_maxtext -= LHEADER;

    /*  send our receive escape code to the master and get his  */
    if ((result = d_snfescape ()) < 0)
	return (result);

#ifdef D_DBGLOG
    d_dbglog ("d_slvstart", "ESCAPE sent ok.  Receive escape '%c'", d_rcvesc);
#endif D_DBGLOG

    if ((result = d_getescape ()) < 0)
	return (result);

#ifdef D_DBGLOG
    d_dbglog ("d_slvstart", "ESCAPE received ok.  Transmit escape '%c'",
	    d_snesc);
#endif D_DBGLOG

#ifdef D_LOG
    d_log ("d_slvstart", "Slave protocol startup completed ok.");
#endif D_LOG

    return (D_OK);
}
