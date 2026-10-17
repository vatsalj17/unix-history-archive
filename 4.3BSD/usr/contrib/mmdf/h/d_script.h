#
/*  script file command codes.  These values have been knocked
 *  up to start at 10, as opposed to the previous 1, in order to
 *  guarentee that they don't conflict with any of the return
 *  values (D_...) which, for some reason, start at 2.
*/

# define  S_DIAL        10      /*  dial the remote system  */
# define  S_XMIT        12      /*  transmit a string  */
# define  S_RECV        13      /*  watch for a string  */
# define  S_GO          14      /*  start the protocol  */
# define  S_END         15      /*  drop the connection  */
# define  S_XILL        16      /*  set illegal transmit characters  */
# define  S_RILL        17      /*  set illegal receive characters  */
# define  S_XPCK        18      /*  set max transmit packet size  */
# define  S_RPCK        19      /*  set max receive packet size  */
# define  S_RETR        20      /*  set number of retran. retries  */
# define  S_TOAK        21      /*  set Time Out limit for AcK  */
# define  S_TODA        22      /*  set Time Out limit for Data  */
# define  S_SELST	23      /*  select begin  */
# define  S_SELEND      24      /*  select end  */
# define  S_ALT         25      /*  alternate  */
# define  S_REM		26	/*  a comment (remember)  */
# define  S_LOG		27      /*  log a message  */
# define  S_ABORT       28      /*  abort the script  */
# define  S_REPLAY      29      /*  Reuse old input  */
# define  S_MARK	30	/*  Mark the place to start a reuse  */
# define  S_PRTTY	31	/*  set tty status bits for protocol mode  */
# define  S_SCTTY	32	/*  set tty status bits for script mode  */
# define  S_DBLBUF	33	/*  status of double buffering  */
# define  S_USEFILE	34	/*  Read script commands from another file  */
# define  S_EOF		35      /*  not actually a command.  This is returned
				 *  by 'getcmd' to indicate an EOF on the
				 *  script src file.
				 */
# define  S_PROMPT	36	/*  check for a prompt from the host to which */
				/*  the master is speaking */
# define  S_BILL	37	/*  start call logging on given number */
