#
# include  "d_proto.h"
# include  "d_returns.h"

/*
 *	D_CONTROL
 *
 *	This routine tries to do something with unexpected
 *	packet types.  If 'rdport' is looking for one packet
 *	type and gets another, then this routine is called.
 *	This allows certain control type packets to be sent
 *	and processed without prior arrangements.  If the
 *	packet is not one of these control packets, then an
 *	error is returned.
 *
 *	packet - a pointer to the start of the packet.
 *
*/

d_control (packet, length)
  char *packet;
  int length;
    {
    int dig1, dig2;
    extern int d_nbuff, d_errno, d_rcvseq;

    /*  switch out to the code to process this packet  */
    switch (packet[TYPEOFF])
    {
	case NBUFF:
	    /*  This is a control packet indicating whether buffering
	     *  is allowed.
	     */
	    dig1 = d_fromhex(packet[TEXTOFF]);
	    dig2 = d_fromhex(packet[TEXTOFF + 1]);

	    /* This should stylistically be done elsewhere, but nobody
	     * else will know we got this packet. */
	    d_rcvseq = d_incseq (d_rcvseq);

	    /*  Make sure the characters are reasonable  */
	    if ((dig1 < 0) ||
		(dig2 < 0)   )
	    {
#ifdef D_LOG
		d_log ("d_control", "NBUFF: Bad hex char in packet ('%c%c')",
				packet[LHEADER], packet[LHEADER+1]);
#endif D_LOG
		return (D_NONFATAL);
	    }

	    /*  transfer to the buffer counter  */
	    d_nbuff = (dig1 << 4) | dig2;
	    return (D_OK);

	case QUIT:
#ifdef D_LOG
	    d_log ("d_control", "received QUIT packet");
#endif D_LOG
	    if (length == LQUIT)
	    {
		d_errno = D_RCVQUIT;
		return (D_FATAL);
	    }
#ifdef D_LOG
	    d_log ("d_control", "Quit packet bad length (%d)", length);
#endif D_LOG
	    return (D_NONFATAL);

	default:
	    /*  If the packet type is not one of the special control
	     *  packets, then just log it as an error.
	     */
#ifdef D_LOG
	    d_log ("d_control", "Bad packet type '%s'", packet);
#endif D_LOG
	    return (D_NONFATAL);
    }
}
