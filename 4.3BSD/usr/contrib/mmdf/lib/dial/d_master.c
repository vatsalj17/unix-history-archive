# include  "util.h"
# include  "d_proto.h"
# include  "d_returns.h"
# include  <signal.h>
# include  "d_syscodes.h"
# include  "d_clogcodes.h"
# include  "ll_log.h"

/*
 *     D_MASCONN
 *
 *     this routine is called by the master protocl module to initiate a
 *     connection to a remote system.  the scriptfile used contains all
 *     of the information necessary to dial the remote machine, set
 *     parameters.
 *
 *     scriptfile -- name of the script file
 *
 *     logfile -- name of the file to be used for logging.  if this arg
 *                is 0, then logging is done on the standard output
 *
 *     tson -- this argument should be non-zero if the protocol module
 *             should keep a transcript of the connection
 *
 *     tsfile -- name of the file in which the transcript should be kept.
 *
 *     debug -- set to non-zero to enable more extensive logging
 *
 */

d_masconn(scriptfile, logfile, tson, tsfile, debug)
  char  *scriptfile, *tsfile;
  struct ll_struct * logfile;
  int  tson, debug;
    {
    extern int  d_debug, d_master, d_snseq, d_rcvseq;
    register int  result;

    /*  open the log and transcript file  */
    d_debug = debug;

    if ((result = d_opnlog(logfile)) < 0)
      return(result);

    d_init();

    if (tson)
      if ((result = d_tsopen(tsfile)) < 0)
        return(result);

    d_master = 1;
    /* numbers will be incremented before use, thus effectively start at 0 */
    d_snseq = 3;
    d_rcvseq = 3;

    /*  open the script file and start interpreting it  */
    if ((result = d_scopen(scriptfile, 0, (char **) 0)) < 0)
      return(result);

    if ((result = d_script()) < 0)
      return(result);

    /*  when the script file pauses, start the protocol  */
    result = d_masstart();
    return(result);
}

/*
 *     D_MASDROP
 *
 *     this routine is called by the master to drop the connection to the
 *     other side.  the port, lock, transcript, and log files are closed.
 *
 *     sndquit -- non-zero if a QUIT sequence should be initiated with
 *                the other side
 *
 *     doscript -- non-zero if the script should be resumed
 */

d_masdrop(sndquit, doscript)
  register int  sndquit, doscript;
    {
    register int  result;

    d_dbglog("d_masdrop", "dropping connection, sndquit %d, doscript %d",
        sndquit, doscript);

    result = 0;

    /*  go through the QUIT sequence if we're supposed to  */
    if (sndquit)
      d_snquit();

    /*  resume the script, if enabled  */
    if (doscript)
      {
#ifdef D_LOG
      d_log("d_masdrop", "Resuming script file.");
#endif D_LOG
      result = d_script();
      }

#ifdef D_LOG
    d_log("d_masdrop", "master dropped connection");
#endif D_LOG

    /*  drop the connection.  close the transcript and log  */
    d_drop();
    d_tsclose();
    d_clslog();

    return(result);
}

/*
 *     D_MASSTART
 *
 *     this routine is called in the master to start up the protocol once
 *     the matching slave routine has been started.  this function exchanges
 *     RESET's with the slave and sets up the escape characters for the
 *     encoding.
 */

d_masstart()
    {
    extern int  d_lxmax, d_lrmax, d_rxmax, d_rrmax, d_maxtext, d_nbuff;
    extern int d_wpack;
    extern int d_errno;
    extern char  d_snesc, d_rcvesc;
    extern unsigned short  d_lxill[], d_lrill[], d_rxill[], d_rrill[],
	d_lcvec[];
    register int  result;

#ifdef D_DBGLOG
    d_dbglog("d_masstart", "Beginning master host startup sequence");
#endif D_DBGLOG

    /*  get the XPATH and RPATH packets from the slave. then build
     *  the local transmit code vector.
     */
    if ((result = d_getpath(XPATH, &d_rxmax, d_rxill)) < 0)
      return(result);

#ifdef D_DBGLOG
    d_dbglog("d_masstart", "XPATH received ok.");
#endif D_DBGLOG

    if ((result = d_getpath(RPATH, &d_rrmax, d_rrill)) < 0)
      return(result);

#ifdef D_DBGLOG
    d_dbglog("d_masstart", "RPATH received ok.");
#endif D_DBGLOG

    /*  If appropriate, send the packet describing how we are buffering
     *  packets.  This cannot be sent to some sites, as they do not
     *  recognize the packet type.
     */
    if (d_wpack != 0)
	d_setbuff (d_nbuff);

    d_orbitvec(d_lxill, d_rrill, d_lcvec);

    d_maxtext = d_minimum(d_lxmax, d_rrmax);
#ifdef D_DBGLOG
    d_dbglog("d_masstart", "Maximum transmit packet size set to %d", d_maxtext);
#endif D_DBGLOG
    d_maxtext -= (LHEADER + LTERM);

    /*  now send the XPATH and RPATH to the slave  */
    if ((result = d_snpath(XPATH, d_lxmax, d_lxill)) < 0)
      return(result);

#ifdef D_DBGLOG
    d_dbglog("d_masstart", "XPATH sent ok.");
#endif D_DBGLOG

    if ((result = d_snpath(RPATH, d_lrmax, d_lrill)) < 0)
      return(result);

#ifdef D_DBGLOG
    d_dbglog("d_masstart", "RPATH sent ok.");
#endif D_DBGLOG

    /*  get the escape code we're to use for encoding text sent
     *  to the remote host.
     */
    if ((result = d_getescape()) < 0)
      return(result);

#ifdef D_DBGLOG
    d_dbglog("d_masstart", "ESCAPE received ok.  Transmit escape '%c'",
	d_snesc);
#endif D_DBGLOG

    if ((result = d_snfescape()) < 0)
      return(result);

#ifdef D_DBGLOG
    d_dbglog("d_masstart", "ESCAPE sent ok.  Receive escape '%c'", d_rcvesc);
#endif D_DBGLOG

#ifdef D_LOG
    d_log("d_masstart", "Master protocol startup completed ok.");
#endif D_LOG

    return(D_OK);
}
