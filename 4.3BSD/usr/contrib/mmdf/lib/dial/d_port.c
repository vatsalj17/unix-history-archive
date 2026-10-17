# include  "util.h"
# include  "d_proto.h"
# include  "d_returns.h"
# include <signal.h>
# include  "d_syscodes.h"
# include  <stdio.h>
# include  "d_structs.h"

/*  Jun 81    D. Crocker    Major reworking of alarm usage, trying to
 *                          deal with fact that there may be multiple
 *                          calls to d_rdport trying to get one good packet
 *                          d_rdport fixed to handle 8th bit on
 *                          Converted some d_dbglogs to d_log
 *  Jul 81    D. Crocker    Skip initial non-hex text on line.
 *
 *  Sep 83    M. Laubach    added procedure d_waitprompt to interface
 *                          to HP and IBM software prompting
 *
 *  Mar 84    D. Rockwell   Added select call to d_wrtport, removed alarm
 */

extern int  d_errno, errno;
extern  int     d_prompt;         /* prompt char from script            */
LOCVAR  unsigned d_alrmtim;        /* time to wait for i/o to complete   */


d_alarm (thetime)
    unsigned thetime;
{
    d_alrmtim = thetime;
}


/*
 *
 *     D_RDPORT
 *
 *     this routine is called to read a packet from the port.  the packets
 *     are checked for length errors, bad checksums, and to make sure that
 *     we aren't receiving packets that we have sent ourselves.
 *
 *     packet -- place to load te incoming packet.  should be large enough to
 *               contain a packet one character longer than the maximum.  the
 *               newline is replaced by a null.
 *
 *     returns the length of the incoming packet, minus the newline.
 *
 *     D_FATAL -- fatal error from read call
 *
 *     D_NONFATAL -- eof from port
 */

d_rdport (packet)
char   *packet;
{
    extern  FILE * d_prtfp;
    extern int  d_master,
		d_rcvseq;
    int     flags,
	    seqnum;
    register char  *op;
    register int    count;
    register int   c;

    if (d_alrmtim == 0)
    {
#ifdef D_LOG
	d_log("d_rdport", "*** no more alarm time left");
#endif D_LOG
	d_errno = D_TIMEOUT;
	return(D_INTRPT);
    }

    if (setjmp (timerest)) {
#ifdef D_LOG
	d_log("d_rdport", "read alarm");
#endif D_LOG
	d_errno = D_TIMEOUT;
	return(D_INTRPT);
    }
    for (s_alarm (d_alrmtim); ; )
    {                             /* get valid packet                   */
	for (op = packet, count = 0; count <= MAXPACKET + 1; )
	{                         /* get a line                         */
	    if ((c = d_getc (d_prtfp)) == EOF)
	    {                     /* truly an end of file?              */
		if (ferror (d_prtfp))
		{
		    d_alrmtim = s_alarm (0);
		    if (errno == EINTR)
		    {
#ifdef D_LOG
			d_log("d_rdport", "read alarm");
#endif D_LOG
			d_errno = D_TIMEOUT;
			return(D_INTRPT);
		    }
#ifdef V4_2BSD
		/*
		 * This is a special for 4.2 systems -- a bug in
		 * the tty driver causes EWOULDBLOCK to be returned
		 * instead of an EOF on the tty.  So, we fake it.
		 * The messages are distinguishable.
		 */
		    else if (errno == EWOULDBLOCK)
		    {
#ifdef D_LOG
			d_log("d_rdport", "port read EOF");
#endif D_LOG
			d_errno = D_PORTEOF;
			return(D_NONFATAL);
		    }
#endif V4_2BSD
		    else
		    {
#ifdef D_LOG
			d_log("d_rdport",
				"port read errno %d", errno);
#endif D_LOG
			d_errno = D_PORTRD;
			return(D_FATAL);
		    }
		}
		if (feof (d_prtfp))
		{
		    d_alrmtim = s_alarm (0);
#ifdef D_LOG
		    d_log("d_rdport", "port read eof");
#endif D_LOG
		    d_errno = D_PORTEOF;
		    return(D_NONFATAL);
		}
	    }

	    c = toascii (c);      /* make sure high bit is off          */
	    switch (c)
	    {
		case NULL:        /* nulls are simply skipped           */
		case '\r':        /* carriage returns are skipped       */
		case '\177':      /* DELs  are simply skipped           */
		    continue;

		case '\n':        /* end of line => end of packet       */
		    d_tscribe (packet, count);
		    d_tscribe ("\n", 1);
		    *op = '\0';
		    break;

		default:          /* add to packet buffer               */
		    if (op == packet && !isxdigit (c)) {
#ifdef D_DBGLOG
			d_dbglog ("d_rdport", "noise '%c'", c);
#endif D_DBGLOG
		    } else {       /* packets begin with hex digits      */
			*op++ = c;
			count++;
		    }
		    continue;
	    }

	    if (count <= MAXPACKET + 1)
		break;            /*  so far, so good, now check form   */

#ifdef D_LOG
	    d_log ("d_rdport", "rcv too long (%dc) '%s'",
			count, packet);
#endif D_LOG
	    d_tscribe (packet, count);
	}


/*  make sure that the packet is at least as long as the shortest one.  */
/*  this is done to make sure that when we use fixed offsets for the    */
/*  fields that we're not reading junk.  check the checksum.            */

	if (count < MINPACKET)
	{
#ifdef D_LOG
	    if (count > 0)  /* ignore blank lines */
	        d_log ("d_rdport", "rcv len err (%d)", count);
#endif D_LOG
	    continue;
	}

	if (d_cscheck (packet, count) == D_NO)
	{
#ifdef D_LOG
	    d_log ("d_rdport", "rcv checksum err '%s'", packet);
#endif D_LOG
	    continue;
	}

/*  make sure that we didn't send it.  the other guy may not have echo off  */

	flags = d_fromhex (packet[FLAGOFF]);

	if ((((flags & MASTERBIT) && d_master) ||
		    ((flags & MASTERBIT) == 0) && (d_master == 0)))
	{
#ifdef D_LOG
	    d_log ("d_rdport", "rcv from self");
#endif D_LOG
	    continue;
	}

/*  acknowledge the packet  */

	d_alrmtim = s_alarm (0);  /* ackpack uses the alarm clock */

	seqnum = flags & 03;

#ifdef D_DBGLOG
	d_dbglog ("d_rdport", "Received packet '%s'", packet);
#endif D_DBGLOG

	switch (d_ackpack (packet[TYPEOFF], seqnum))
	{
	    case D_FATAL:
		return (D_FATAL);

	    case D_NONFATAL:
		s_alarm (d_alrmtim);
		continue;

	    default:
		return (count);
	}
    }
}




/*
 *
 *     D_WAITPROMPT
 *
 *     this routine is called to wait for a prompt character from the 
 *     port.  If prompt is not received within a receive timeout period 
 *     then fall through.  Do not return an error. 
 *
 */

d_waitprompt ()
{
    extern  FILE * d_prtfp;
    register int   c;
    int promptalrm = 10;       /* timeout delay in seconds */
    char  ch;

    if (d_prompt == 0) return(0); 

    for (alarm (promptalrm); ; )
    { 
	    if ((c = d_getc (d_prtfp)) == EOF)
	    {                     /* truly an end of file?              */
		if (ferror (d_prtfp))
		{
		    d_alrmtim = alarm (0);
		    if (errno == EINTR)
		    {
                    if (d_prompt == toascii(c)) return(0);
#ifdef D_LOG
                        d_log("d_waitprompt","prompt alarm for %d",d_prompt);
#endif D_LOG
                        d_errno = D_TIMEOUT;
			return(D_INTRPT);
		    }
		    else
		    {
#ifdef D_LOG
			d_log("d_waitprompt","port read errno %d", errno);
#endif D_LOG
			d_errno = D_PORTRD;
			return(D_FATAL);
		    }
		}
		if (feof (d_prtfp))
		{
		    d_alrmtim = alarm (0);
#ifdef D_LOG
		    d_log("d_waitprompt", "port read eof");
#endif D_LOG
		    d_errno = D_PORTEOF;
		    return(D_NONFATAL);
		}
	    }

            ch = toascii(c);
            c  = toascii(c);
            d_tscribe((char *) &ch, 1);   /* put chars into transcript log    */
            if (d_prompt == c) {          /* hoo-rah, break out the champaign */
              promptalrm = alarm(0);      /* cancel alarm                     */
              return(D_OK);
            }
    }
}


/**/

/*
 *     D_WRTPORT
 *
 *     this routine writes a packet on the port.
 *
 *     text -- pointer to packet to be sent.  must have line terminator
 *             already attached.
 *
 *     length -- number of bytes in text
 *
 *     timeout -- number of seconds to wait for write to complete
 *
 *
 *     return values:
 *
 *     OK -- no errors
 *
 *     D_FATAL -- fatal error writing on port
 *
 *     D_NONFATAL -- incomplete packet written on port.  usually means disconnect.
 *
 *     INTERRUPT -- write timer went off
 */

d_wrtport (text, length)
register char  *text;
register int    length;
{
    extern  FILE * d_prtfp;
    register int    result;

    d_tscribe (text, length);

    /*
     *  Added for prompt wait.  Always wait before sending packet
     *  9/28/83   laubach@udel-relay
     */
    if (d_prompt != 0) d_waitprompt();

    /*  Set up an alarm so we won't hang
     *  indefintiely if something goes wrong
     */
    if (setjmp (timerest)) {
	d_errno = D_TIMEOUT;
#ifdef D_LOG
	d_log ("d_wrtport", "write alarm");
#endif D_LOG
	return (D_INTRPT);
    }
    s_alarm (XMITWAIT);

    result = write (fileno (d_prtfp), text, length);
    s_alarm (0);

    if (result == length)
	return (D_OK);

    if (result >= 0)
    {
	d_errno = D_PORTEOF;
	return (D_NONFATAL);
    }

    if (errno == EINTR) {
	d_errno = D_TIMEOUT;
#ifdef D_LOG
	d_log ("d_wrtport", "write alarm");
#endif D_LOG
	return (D_INTRPT);
    }

    d_errno = D_PORTWRT;
    return (D_FATAL);
}

/*
 *      D_BRKPORT
 *
 *      This routine does its best to cause a BREAK on the line.
 *
 */

d_brkport()
{
#ifdef V4_2BSD
#include <sys/ioctl.h>
	ioctl (fileno (d_prtfp), TIOCSBRK, 0);
	sleep ((unsigned) 1);
	ioctl (fileno (d_prtfp), TIOCCBRK, 0);
#else

#ifdef SYS5
#include <termio.h>
	ioctl (fileno(d_prtfp), TCSBRK, 0);
#else
#include <sgtty.h>
	struct sgttyb ttybuf;
	int spdsave;

	ioctl (fileno (d_prtfp), TIOCGETP, &ttybuf);
	spdsave = ttybuf.sg_ospeed;
	ttybuf.sg_ospeed = B150;
	ioctl (fileno (d_prtfp), TIOCSETP, &ttybuf);
	d_wrtport("\0\0\0\0\0", 5);
	ttybuf.sg_ospeed = spdsave;
	ioctl (fileno (d_prtfp), TIOCSETP, &ttybuf);
#endif SYS5
#endif V4_2BSD
}
