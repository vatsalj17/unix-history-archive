# include  "util.h"
#ifdef SYS5
# include <termio.h>
#else
# include  <sgtty.h>
# include  "d_proto.h"
#endif SYS5
# include  <stdio.h>
# include  "d_structs.h"



struct dialports  *d_prtpt;    /*  pointer to currently user port in  */
				/*  'd_prts'                          */

struct directlines  *d_ptline;  /*  pointer to currently used line in  */
                                /*  'd_lines'                          */
FILE    *d_prtfp = (FILE *) EOF; /*  port stdio file pointer  */

int  d_lckfd = -1,             /*  lock file descriptor  */
     d_baudrate,                /*  index of baud rate for this connection  */
     d_didial,                  /*  set to non-zero if this is a dial-up  */
                                /*  connection.                           */
     d_errno;                   /*  error code  */

char  d_uname[10],           /*  login name of user  */
      d_upath[50];           /*  login directory path name of user  */


/*
 *     this structure defines the baud rate strings that will be accpeted
 *     in phone number specifications.
 */

struct speedtab  d_spdtab[] =
  {
  "0",       B0,
  "50",      B50,
  "75",      B75,
  "110",     B110,
  "134.5",   B134,
  "134",     B134,
  "150",     B150,
  "200",     B200,
  "300",     B300,
  "600",     B600,
  "1200",    B1200,
  "1800",    B1800,
  "2400",    B2400,
  "4800",    B4800,
#ifdef SYS5
  "7200",    B7200,
#endif SYS5
  "9600",    B9600,
#ifdef SYS5
  "19200",   B19200,
#else
  "19200",   EXTA,
#endif SYS5
  "EXTA",    EXTA,
  "exta",    EXTA,
  "38400",   EXTB,
  "EXTB",    EXTB,
  "extb",    EXTB,
  0,         0,
  };




/*
 *     array of ascii baud rate names indexed by the speed.  used to print
 *     understandable diagnostics.
 */

int d_xretry = NSENDTRY;     /*  Init # of retries to make  */
int d_toack = DACKWAIT;      /*  Init time out time for ack */
int d_todata = DATAWAIT;     /*  Init time out for data  */

#ifdef SYS5
unsigned short d_prbitc = PORTPONC;	/* terminal protocol bit on */
unsigned short d_prbiti = PORTPONI;
unsigned short d_prbito = PORTPONO;
unsigned short d_prbitl = PORTPONL;

unsigned short d_scbitc = PORTSONC;	/* terminal script bits on */
unsigned short d_scbiti = PORTSONI;
unsigned short d_scbito = PORTSONO;
unsigned short d_scbitl = PORTSONL;

#else

int d_pron = PORTPON;       /*  terminal protocol bit on  */
int d_proff = PORTPOFF;     /*  terminal protocal bit off */
int d_scon = PORTSON;       /*  terminal script bits on   */
int d_scoff = PORTSOFF;     /*  terminal script bits off  */

#endif SYS5

int d_nbuff = 1;	    /*  The number of messages to send out before
			     *  requiring an ACK.  Initially, get an ACK
			     *  immediately.
			     */
int d_wpack;		    /*  Non-zero if the window specification
			     *  packet should be sent.
			     */
