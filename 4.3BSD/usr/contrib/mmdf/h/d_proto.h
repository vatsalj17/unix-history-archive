
/*
 *     message type codes
 */

# define  DATA           '0'     /*  data packet  */
# define  DATAACK        '1'     /*  acknowledge DATA packet  */
# define  XPATH          '2'     /*  transmit path packet  */
# define  XPATHACK       '3'     /*  acknowledge XPATH packet  */
# define  RPATH          '4'     /*  receive path packet  */
# define  RPATHACK       '5'     /*  acknowledge RPATH packet  */
# define  ESCAPE         '6'     /*  escape packet  */
# define  ESCAPEACK      '7'     /*  acknowledge ESCAPE packet  */
# define  QUIT           '8'     /*  quit packet  */
# define  QUITACK        '9'     /*  acknowledge QUIT packet  */
# define  NBUFF		 'A'	 /*  buffering specification packet  */
# define  NBUFFACK	 'B'	 /*  acknowledge NBUFF pack  */




/*
 *     lengths of the fixed length packets
 */

# define  LHEADER         6     /*  length of the header on all packets  */
				/*  If changed, change TEXTOFF also  */
# define  LTERM		  2     /*  length of the terminator for all pack  */
# define  LACK            6     /*  length of the ACK packet  */
# define  LRESET         40     /*  length of the RESET packet  */
# define  LRESETACK       6     /*  length of the RESETACK packet  */
# define  LESCAPE         8     /*  length of the ESCAPE packet  */
# define  LNBUFF          8     /*  length of the NBUFF packet  */
# define  LQUIT           6     /*  length of the QUIT packet  */
# define  LQUITACK        6     /*  length of the QUITACK packet  */
# define  LPATH          40     /*  length of an XPATH or RPATH packet  */




/*
 *     protocol constants and packet header offsets
 */

# define  CHKOFF           0     /*  start of 4 character checksum  */
# define  TYPEOFF          4     /*  type character  */
# define  FLAGOFF          5     /*  flag offset  */
# define  TEXTOFF          6     /*  text offset  */
# define  MAXPOFF          6     /*  maximum packet length field offset  */
# define  ESCOFF           6     /*  escape character field offset  */
# define  NBOFF            6     /*  buffer limit field offset  */
# define  LEGALOFF        10     /*  legal input character set field  */






/*
 *     bits in the flag byte
 */

# define  MASTERBIT    010     /*  set of the packet is being sent by the  */
                               /*  host which originated the call          */
# define  EOFBIT        04     /*  set if the last packet in a stream  */








/*
 *     various timeouts
 */

# define  CONNWAIT        45     /*  number of seconds to wait for a direct  */
                                 /*  line open to succeed.                   */
# define  DIALWAIT       120     /*  number of seconds to wait for a dial    */
                                 /*  line open to succeed.                   */
# define  ACUWAIT        300     /*  number of seconds to wait for a child   */
				 /*  to succeed opening via the acu     */
# define  XMITWAIT        60     /*  number of seconds to wait for a packet  */
                                 /*  to be transmitted.  this is for the     */
                                 /*  alarm around the port write calls       */
# define  QACKWAIT        10     /*  number of seconds to wait for response  */
                                 /*  to a QUIT packet before retransmission  */
# define  EACKWAIT        10     /*  number of seconds to wait for response  */
                                 /*  to an ESCAPE packet before resending    */
# define  DACKWAIT        10     /*  number of seconds to wait for ACK  */
                                 /*  before retransmission              */
# define  ESCAPEWAIT      60     /*  number of seconds to wait for an  */
                                 /*  ESCAPE packet during protocol     */
                                 /*  startup                           */
# define  NBUFFWAIT       60     /*  number of seconds to wait for NBUFF */
# define  DATAWAIT       180     /*  amount of time to wait for a DATA  */
                                 /*  packet                             */
# define  XPATHWAIT       60     /*  number of seconds to wait for XPATH  */
# define  RPATHWAIT       60     /*  number of seconds to wait for RPATH  */
# define  XPAWAIT         10     /*  number of seconds to wait for XPATHACK  */
# define  RPAWAIT         10     /*  number of seconds to wait for RPATHACK  */
# define  NBACKWAIT       10     /*  number of seconds to wait for NBUFFACK */




/*
 *     manifest configuration constants
 */

# define  MAXPACKET      255     /*  maximum length of packet excluding  */
                                 /*  <cr>                                */
# define  MINPACKET        6     /*  minimum length of packet excluding  */
                                 /*  <cr>                                */
# define  MINPATHSIZ      40     /*  minimum packet size that a system must   */
                                 /*  be capable of receiving or transmitting  */
                                 /*  excluding <cr>                           */
# define  MAXSCRLINE     256     /*  maximum length of line in script file  */
# define  MAXFIELDS       10     /*  maximum fields on a script line */
# define  MATCHLEN       128     /*  maximum length of string to be matched */
				 /*  in the script file                     */
# define  NSENDTRY         5     /*  number of times to attempt message  */
                                 /*  transmission                        */

# define  NBUFFMAX         2     /*  d_nbuff upper limit  */



#ifdef SYS5

/* PORT*I is for c_iflag field in termio */
/* PORT*O is for c_oflag field in termio */
/* PORT*C is for c_cflag field in termio */
/* PORT*L is for c_lflag field in termio */

# define PORTSONI	(IXON|ICRNL|ISTRIP|IGNBRK|IGNPAR)
# define PORTSONO	(0)
# define PORTSONC	(CS7|PARENB|CREAD|HUPCL)
# define PORTSONL	(0)

			/* bits to turn on in script mode */
# define PORTPONI	(IXON|ICRNL|ISTRIP|IGNBRK|IGNPAR)
# define PORTPONO	(0)
# define PORTPONC	(CS7|PARENB|CREAD|HUPCL)
# define PORTPONL	(ICANON)

#else

# define  PORTSON       (EVENP|ODDP|RAW)
				 /*  bits to turn on in script mode    */
# define  PORTSOFF      (ALLDELAY|CRMOD|ECHO|LCASE)
				 /*  bits to turn off in script mode     */
# define  PORTPON       (EVENP|ODDP)
				 /*  bits to turn on in protocol mode    */
# define  PORTPOFF      (ALLDELAY|CRMOD|ECHO|RAW|LCASE)
				 /*  bits to turn off in protocol mode   */
#endif SYS5
