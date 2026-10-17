# include  "d_proto.h"
# include  "d_returns.h"

/*
 *     D_SNDQUIT
 *
 *     this routine is called to send a QUIT packet to the other end
 *     and wait for the reply.
 */

d_snquit()
    {
    extern int  d_snseq;
    register int  length, result;
    char  packet[MAXPACKET + 2];

#ifdef D_LOG
    d_log("d_snquit", "sending QUIT");
#endif D_LOG

/*  build and send the QUIT packet  */

    d_snseq = d_incseq(d_snseq);
    length = d_bldpack(QUIT, d_snseq, 1, (char *) 0, packet);

    result = d_snpkt(QUIT, packet, length);

    return(result);
    }
