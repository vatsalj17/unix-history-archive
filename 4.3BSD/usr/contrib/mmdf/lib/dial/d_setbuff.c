#
# include  "d_proto.h"
# include  "d_returns.h"
# include  <stdio.h>
# include  "d_structs.h"

/*
 *	D_SETBUFF
 *
 *	Sends the control packet indicating how deeply packets
 *	can be buffered.  Actually, at the moment the only
 *	relevant information is whether buffering is allowed or
 *	not.  The depth of buffering is irrelavent.  However, this
 *	information can be included cheaply, and it may make later
 *	modifications easier, so it is given anyway.
 *
 *	nbuff - the depth of the buffering.
 *		This number is currently 0 to indicate no buffering
 *		(i.e., all packets must be ACKed immediately) or 1
 *		to indicate that one outstanding packet is allowed.
 *
*/

d_setbuff (nbuff)
  int nbuff;
    {
    char tbuff[3], packet[MAXPACKET + 2];
    int length;
    extern int d_snseq;

    tbuff[0] = d_tohex ((nbuff >> 4) & 017);
    tbuff[1] = d_tohex ((nbuff & 017));
    tbuff[2] = '\0';

    d_snseq = d_incseq (d_snseq);
    length = d_bldpack (NBUFF, d_snseq, 1, tbuff, packet);

    return (d_snpkt(NBUFF, packet, length));
}
