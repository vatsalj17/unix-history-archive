#
# include  "d_proto.h"
# include  "d_returns.h"
# include  <signal.h>
# include  <stdio.h>
# include  "d_structs.h"
# include  "d_syscodes.h"

extern int d_master;
extern int d_errno;
extern int d_xretry;
extern int d_nbuff;
extern int d_toack;
extern int d_rcvseq;
extern int d_todata;
static struct pkbuff packst[NBUFFMAX];
static int first;		/*  index of first packet in buff  */
static int nout;		/*  number of packets in buff      */

/*  Jun 81  D. Crocker      Major rework of alarm usage
 *                          Converted some d_dbglogs to d_log
 *                          fix sndpacket() timeout return value
 *  Jul 81  D. Crocker      fix ack timeout to cause retransmit
 *  Dec 81  D. Crocker      add retry hack to separate packets
 *  Feb 84  D. Long         changed separating string to nn instead of nrnr
 */


/*
 *     D_BLDPACK
 *
 *     routine which builds a packet for transmission.  most of the
 *     understanding of the packet format is in this function.
 *
 *     type -- packet type code
 *
 *     seqnum -- packet sequence number
 *
 *     eof -- end of file indicator.  when non-zero, it causes the eof bit
 *            to be set in the outgoing packet
 *
 *     text -- if non-zero, then this is taken to be a pointer to the text to
 *             be included in the packet
 *
 *     buffer -- place to load the complete packet
 */

d_bldpack (type, seqnum, eof, text, buffer)
char    type,
       *buffer;
register char  *text;
int     seqnum,
        eof;
{
    register char  *cp;
    register int    flags;
    int     length;

#ifdef D_DBGLOG
    d_dbglog ("d_bldpack", "building packet - type '%c', seqnum %d, eof %d",
	    type, seqnum, eof);
#endif D_DBGLOG

    buffer[TYPEOFF] = type;

    /*  load the flag byte  */
    flags = seqnum & 03;

    if (eof)
	flags |= EOFBIT;

    if (d_master)
	flags |= MASTERBIT;

    buffer[FLAGOFF] = d_tohex (flags);

    /*  load the text, if any  */
    cp = &buffer[TEXTOFF];
    length = 6;

    if (text)
	while (*text)
	{
	    *cp++ = *text++;
	    length++;
	}

    /*  tack on the checksum and the line terminator  */
    d_cscomp (buffer, length);

    *cp++ = '\r';
    *cp++ = '\n';
    /*  although the '\0' is not used by this package, it makes
     *  printing easier.
     */
    *cp = '\0';
    length += 2;

    return (length);
}

/*
 *
 *     D_SNPKT
 *
 *     this routine sends a packet and waits for a given type response
 *     packet from the other side.  retransmissions are done here.
 *
 *      type -- type of packet being sent; used to determine reply type.
 *
 *     packet -- pointer to fully formed packet
 *
 *     length -- length of the packet
 *
 */

d_snpkt (type, packet, length)
char    type;
char   *packet;
int     length;
{
    register int    result;
    int     next;

#ifdef D_DBGLOG
    d_dbglog ("d_snpkt", "sending packet - code '%c'", packet[TYPEOFF]);
#endif D_DBGLOG

	/*
	 * Should we wait for an outstanding packet before sending this one?
	 */

	if (type != DATAACK && nout == d_nbuff) {
		result = d_confirm (&packst[first]);
		/*  
		 * We always want to pull this msg off the front of
		 * the queue, even if the 'confirm' failed.  Ours is
		 * not to judge whether this is bad or not, but merely
		 * to indicate to the calling routine what happened.
		 * If it doesn't care that the packet wasn't ACKed,
		 * then we don't want to screw things up by leaving
		 * the packet in the queue.
		 */
		first = (first + 1) % NBUFFMAX;
		nout--;
		switch (result)
		{
		case D_OK:
			break;

		case D_FATAL:
			/*
			 * Couldn't get an ACK on the last packet.
			 * Return an error.
			 */
			return (D_FATAL);

		default:
#ifdef D_LOG
			d_log ("d_snpkt", "Unknown return (%d) from d_confirm", result);
#endif D_LOG
			return (D_FATAL);
		}
	}
	/*
	 * We now have a space for this packet, store it.
	 */
	next = (first + nout) % NBUFFMAX;
	d_packset(type, packet, length, &packst[next]);
	/*
	 * Send out the packet for the first time.
	 */
	result = d_transmit(&packst[next]);
	/*
	 * If this was an ACK packet, or if the write to the port failed,
	 * then there is no need to wait around for an ACK.  Note that in
	 * this case the packet was essentially never queued in the buffer
	 * because we never incremented 'nout'.
	 */
	if ((result == D_OK) || (result < 0))
		return (result);

	/* Update the information about outstanding packets */
	nout++;

	/*
	 * In some cases the packet just sent must be ACKed immediately.
	 * These cases are: 1) if buffering is not allowed at all,
	 * 2) If this is an end-of-logical-record packet, or 3) if this
	 * is not a data packet (which implies end of record).
	 * Of course, if we are going to require that the last packet
	 * be ACKed, we had better get ACK's for all the ones preceding
	 * it.
	 *
	 * Performance improvement note:  the above is only strictly true
	 * for QUIT packets.  We can wait for the next transmitted
	 * packet to get the previous ACK in all other cases. -- DR
	 */
	if (((d_fromhex (packst[next].p_data[FLAGOFF]) & EOFBIT) != 0) ||
	    (packst[next].p_data[TYPEOFF] != DATA)) {

		/*
		 * Get an immediate ACK for all outstanding packets
		 */
		while (nout > 0) {
#ifdef D_DBGLOG
			d_dbglog ("d_snpkt", "nout = %d, pack index = %d", nout, first);
#endif D_DBGLOG
			result = d_confirm (&packst[first]);
			/*  See note above for explanation of next two lines  */
			first = (first + 1) % NBUFFMAX;
			nout--;
			switch (result) {
			case D_OK:
				break;

			case D_FATAL:
				return (D_FATAL);

			default:
#ifdef D_LOG
				d_log ("d_snpkt", "Unknown return (%d) from d_transmit",
				        result);
#endif D_LOG
				return (D_FATAL);
			}
		}
		return (D_OK);
	}

    /*  If we get here, then this packet has not yet been ACKed, but
     *  we are going to fudge and return D_OK, planning all the while
     *  to get the ACK later.  If this packet is never ACKed, then its
     *  error will be returned on a later packet.
     */
    return (D_OK);
}

/*
 *
 *	D_TRANSMIT
 *
 *	this routine does the actual work of transmitting the packet.
 *	It returns a value indicating whether an ACK should be expected.
 *
 *	packet - a pointer to the packet structure
 *
*/

d_transmit(pack)
  struct pkbuff *pack;
    {
    register int result;

#ifdef D_DBGLOG
    d_dbglog ("d_transmit", "transmitting '%s'", pack->p_data);
#endif D_DBGLOG

    if ((result = d_wrtport (pack->p_data, pack->p_length)) < 0)
    {
#ifdef D_LOG
	d_log ("d_transmit", "on packet [%c](%d)'%s'",
		pack->p_pktype, pack->p_length, pack->p_data);
#endif D_LOG
	return (result);
    }

    /*  If this is an ACK packet, then it doesn't get acknowledged  */
    switch (pack->p_pktype)
    {
	case DATAACK:	      /*  acknowledge DATA packet           */
	case XPATHACK:        /*  acknowledge XPATH packet          */
	case RPATHACK:        /*  acknowledge RPATH packet          */
	case ESCAPEACK:       /*  acknowledge ESCAPE packet         */
	case QUITACK: 	      /*  acknowledge QUIT packet           */
	case NBUFFACK:	      /*  acknowledge NBUFF packet	    */
	    return (D_OK);

	default:
	    return (D_CONTIN);
    }
}

/*
 *
 *	D_PACKSET
 *
 *	type - the type code for this packet
 *	packet - pointer to the start of the packet itself
 *	length - length of the packet in chars
 *	pack - a structure with data about the packet being sent
 *
 *
*/

d_packset (type, packet, length, pack)
  char type;
  char packet[];
  int length;
  struct pkbuff *pack;
    {
    register char *cp;
    register int index;

    /*  transfer appropriate information to the structure  */
    for (cp = pack->p_data, index = 0;  index < length;  index++)
	*cp++ = packet[index];
    /*  Makes printing easier if needed after an error  */
    *cp = '\0';
    pack->p_length = length;
    pack->p_pktype = type;
    pack->p_seqnum = d_fromhex (packet[FLAGOFF]) & 03;

    switch (type)
    {
	case DATA: 		  /*  data packet  */
	    /*  Remember, if data was sent, an ACK is being waited on  */
	    pack->p_timeo = d_toack;
	    pack->p_acktype = DATAACK;
	    return;

	case XPATH: 	  /*  transmit path packet  */
	    pack->p_timeo = XPAWAIT;
	    pack->p_acktype = XPATHACK;
	    return;

	case RPATH: 	  /*  receive path packet  */
	    pack->p_timeo = RPAWAIT;
	    pack->p_acktype = RPATHACK;
	    return;

	case ESCAPE: 	  /*  escape packet  */
	    pack->p_timeo = EACKWAIT;
	    pack->p_acktype = ESCAPEACK;
	    return;

	case QUIT: 		  /*  quit packet  */
	    pack->p_timeo = QACKWAIT;
	    pack->p_acktype = QUITACK;
	    return;

	case NBUFF:		/* nbuffer packet */
	    pack->p_timeo = NBACKWAIT;
	    pack->p_acktype = NBUFFACK;
	    return;
    }
}

/*
 *
 *	D_CONFIRM
 *
 *	this routine looks for the ACK for the given pack.  If
 *	it fails to recv it in a reasonable time, it retransmits
 *	the pack.
 *
 *	packet - a pointer to the struct containing info about the
 *		 packet we are trying to get ACKed.
 *
*/

d_confirm (packet)
struct pkbuff *packet;
{
	register int result, index;
	register int retries = 0;

	/*  loop, checking for the ACK and trying a retransmit  */

#ifdef D_DBGLOG
	d_dbglog("d_confirm", "Looking for ack of packet [%c](%d)'%s'", 
		packet->p_pktype, packet->p_length, packet->p_data);
#endif D_DBGLOG

	while (retries++ < d_xretry) {
		/*  The packet was sent once before this routine was called.
		 *  Therfore, start off looking for the ACK.
		 */
		result = d_getack (packet);

		if (result > 0)
			return (result);

		if (result == D_RETRY) {
			/*
			 * Because the later packets should've been dropped
			 * due to bad sequence numbers, we must retransmit
			 * this packet and its successors.
			 * Send newlines to clear out any leftover noise.
			 */
#ifdef D_DBGLOG
			d_dbglog("d_confirm", "Retransmitting %d packet[s]", nout);
#endif D_DBGLOG
#ifdef D_LOG
			d_log ("d_confirm", "separating");
#endif D_LOG
			d_wrtport ("\n\n", 2);
			for(index = 0;  index < nout;  index++) {
				if (d_transmit (&packst[(first+index) % NBUFFMAX]) < 0) {
#ifdef D_DBGLOG
					d_dbglog("d_confirm", "Error on retransmission");
#endif D_DBGLOG
					return (D_FATAL);
				}
			}
			continue;
		}

		/*  An unknown error  */
#ifdef D_DBGLOG
		d_dbglog ("d_confirm", "Unknown error from 'getack()'");
#endif D_DBGLOG
		return (result);
	}

#ifdef D_LOG
	d_log ("d_confirm", "retransmissions (%d) exhausted", retries-1);
	d_log ("d_confirm", "on packet [%c](%d)'%s'",
		packet->p_pktype, packet->p_length, packet->p_data);
#endif D_LOG
	d_errno = D_NORESP;
	return (D_FATAL);
}


/*
 *
 *	D_GETACK
 *
 *	Actually tries to get the ACK.  This routine will set up
 *	the time out alarm.  It returns when the alarm sounds, or
 *	when the ACK has been received.	
 *
 *	pack - a pointer to the structure representing the packet
 *
*/

d_getack (pack)
  struct pkbuff *pack;
    {
    char respbuff[MAXPACKET + 2];
    register int recvseq, result;

    /*  read incoming packets and watch for errors.  interrupted
     *  reads return, allowing the calling routine to retransmit.
     *  Other errors cause an immediate return.
     */
    d_alarm (pack->p_timeo);

#ifdef D_DBGLOG
    d_dbglog ("d_getack", "Wanted: ack type '%c', seqnum %d",
				pack->p_acktype, pack->p_seqnum);
#endif D_DBGLOG
    for ( ;; )
    {
	if ((result = d_rdport (respbuff)) < D_OK)
	{
	    if (result == D_INTRPT)
		/*  The ack timed out  */
		return (D_RETRY);
	    return (result);
	}

	/*  check the type and sequence number  */
	if (respbuff[TYPEOFF] == pack->p_acktype)
	{
	    recvseq = d_fromhex (respbuff[FLAGOFF]) & 03;

	    if (recvseq == pack->p_seqnum)
	    {
#ifdef D_DBGLOG
		d_dbglog ("d_getack", "response received");
#endif D_DBGLOG
		return (D_OK);
	    }
#ifdef D_LOG
	    else
		d_log ("d_getack", "bad packet seq num '%s'", respbuff);
#endif D_LOG
	}
#ifdef D_LOG
	else
	    d_log ("d_getack", "bad packet type '%s'", respbuff);
#endif D_LOG
    }

}



/*
 *
 *     D_ACKPACK
 *
 *     this routine is called to acknowledge a received packet.  it builds
 *     the ack and sends it.  an ack won't be acknowledged.
 *
 *     type -- the type of the packet to be acknowledged
 *
 *     seqnum -- the sequence number of the packet
 *
 *
 *     return values:
 *
 *          D_NONFATAL -- returned if the received packet has an unknown type
 */

d_ackpack (type, seqnum)
char    type;
register int    seqnum;
{
    char    ackbuff[MAXPACKET + 2];
    register int    ackleng,
                    result;

#ifdef D_DBGLOG
    d_dbglog ("d_ackpack", "acknowledging packet type '%c', seqnum %d",
	    type, seqnum);
#endif D_DBGLOG

    /*  switch on the type.  some packet don't get ack's sent  */
    switch (type)
    {
	case DATAACK: 
	case XPATHACK: 
	case RPATHACK: 
	case QUITACK: 
	case ESCAPEACK: 
	case NBUFFACK:
	    return (D_OK);
    }

    /*
     * this code produces a protocol change which acts to alleviate
     * a protocol deadlock problem in which the peers got out of sync
     * during the envelope-verification phase.
     */
    /* if this packet is too badly out of sequence, do not ack it */
    if (seqnum == d_incseq(d_rcvseq))
    {
#ifdef D_DBGLOG
	d_dbglog("d_ackpack", "packet way out of seq--no ack--d_rcvseq %d", d_rcvseq);
#endif D_DBGLOG
    	return(D_OK);
    }

    /* now, generate the proper ACK */
    switch(type)
    {
	case DATA: 
	    ackleng = d_bldpack ((type = DATAACK), seqnum,
					1, (char *) 0, ackbuff);
	    break;

	case RPATH: 
	    ackleng = d_bldpack ((type = RPATHACK), seqnum,
					1, (char *) 0, ackbuff);
	    break;

	case XPATH: 
	    ackleng = d_bldpack ((type = XPATHACK), seqnum,
					1, (char *) 0, ackbuff);
	    break;

	case QUIT: 
	    ackleng = d_bldpack ((type = QUITACK), seqnum,
					1, (char *) 0, ackbuff);
	    break;

	case ESCAPE: 
	    ackleng = d_bldpack ((type = ESCAPEACK), seqnum,
					1, (char *) 0, ackbuff);
	    break;

	case NBUFF:
	    ackleng = d_bldpack ((type = NBUFFACK), seqnum,
					1, (char *) 0, ackbuff);
	    break;

	default: 
#ifdef D_LOG
	    d_log ("d_ackpack", "unknown packet type '%c'", type);
#endif D_LOG
	    return (D_NONFATAL);
    }

    /*  send that pack that we've built  */
    result = d_snpkt (type, ackbuff, ackleng);
    return (result);
}

/*
 *
 *     D_WATCH
 *
 *     this routine is called to watch the incoming packets for one of
 *     a specified type.  all other packets, except QUIT's, are ignored.
 *
 *     packet -- place to load the received packet
 *
 *     type -- the type of the packet we're looking for
 */

d_watch (packet, type)
register char  *packet;
char    type;
{
    register int    length;
    int     recvseq;

#ifdef D_DBGLOG
    d_dbglog ("d_watch", "watching for packet with code '%c'", type);
#endif D_DBGLOG

    switch (type)		  /* set timer, for delayed packets     */
    {
	case DATA: 		  /*  data packet                       */
	    d_alarm (d_todata);
	    break;

	case XPATH: 		  /*  transmit path packet              */
	    d_alarm (XPATHWAIT);
	    break;

	case RPATH: 		  /*  receive path packet               */
	    d_alarm (RPATHWAIT);
	    break;

	case ESCAPE: 		  /*  escape packet  */
	    d_alarm (ESCAPEWAIT);
	    break;

	case NBUFF:		/* windowsize packet */
		d_alarm(NBUFFWAIT);
		break;
    }

/*  read packets and search.  QUIT's cause a return after acknowlegement  */

    d_rcvseq = d_incseq (d_rcvseq);

    for ( ;; )
    {
	length = d_rdport (packet);

	if (length < 0)
	{
#ifdef D_LOG
	    d_log ("d_watch", "bad watch read (%d)", length);
#endif D_LOG
	    break;
	}
	recvseq = d_fromhex (packet[FLAGOFF]) & 03;

	if (recvseq != d_rcvseq)
	{
#ifdef D_LOG
	    d_log ("d_watch",
		    "packet out of sequence - type '%c', got %d, expected %d",
		    packet[TYPEOFF], recvseq, d_rcvseq);
	    d_log ("d_watch", "in '%s'", packet);
#endif D_LOG
	    continue;
	}

	if (packet[TYPEOFF] == type)
	    break;            /* success                            */

	/*  We have received a packet, but it is not the type we
	 *  wanted.  Let's try to interrpret it as a special control
	 *  packet.
	 */
	switch (d_control (packet, length))
	{
	    case D_FATAL:
		return (D_FATAL);

	    case D_OK:
	    case D_NONFATAL:
		break;
	}
    }

    return (length);
}

/*
 *
 *     D_INCSEQ
 *
 *     this routine increments the sequence number passed as the argument
 *     and returns the result.
 *
 *     seqnum -- the sequence number to be incremented
 */

d_incseq (seqnum)
register int    seqnum;
{

    seqnum = (seqnum + 1) & 03;
    return (seqnum);
}
