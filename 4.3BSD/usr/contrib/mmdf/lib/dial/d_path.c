# include  "d_proto.h"
# include  "d_returns.h"
#include <signal.h>
# include  "d_syscodes.h"

/*  Jul 81  D. Crocker      convert maxlen>255 -> 255
 */

/*
 *     D_SNPATH
 *
 *     this routine is called to send a PATH packet.  it is used for both
 *     XPATH and RPATH packets.
 *
 *     type -- the type of the packet to send (XPATH or RPATH)
 *
 *     ack -- the type of acknowledgement to wait for
 *
 *     ackwait -- the amount of time to wait for the acknowledgement
 *
 *     maxleng -- the maximum packet length for this path
 *
 *     bitvector -- the bit vector of illegal characters (local) for this
 *                  path.
 */

d_snpath(type, maxleng, bitvector)
  char  type;
  int  maxleng;
  unsigned short bitvector[];
    {
    extern int  d_snseq;
    register int  length, result;
    char  packet[MAXPACKET + 2], txtbuf[LPATH];

    /*  assemble the text portion  */
    if (maxleng > 255)            /* must fit into 8-bit field          */
    {
#ifdef D_LOG
	d_log("d_snpath", "max packet len > 255 (%d)", maxleng);
#endif D_LOG
	maxleng = 255;            /* should be safe to trim it down     */
    }
    txtbuf[0] = d_tohex((maxleng >> 4) & 017);
    txtbuf[1] = d_tohex(maxleng & 017);

    d_bldhvec(bitvector, &txtbuf[2]);

    /*  build the complete packet and send it  */
    d_snseq = d_incseq(d_snseq);
    length = d_bldpack(type, d_snseq, 1, txtbuf, packet);

    result = d_snpkt(type, packet, length);
    return(result);
}


/*
 *     D_GETPATH
 *
 *     this routine is called to get a path packet of the specified type
 *     from the port.  it thens calls the routine to decode it.
 *
 *     type -- the type of path packet we're looking for
 *
 *     timeout -- the amount of time that we're willing to wait to receive
 *                it
 *
 *     maxleng -- pointer to where the maximum packet length specified in the
 *                packet should be loaded
 *
 *     bitvector -- pointer to the bit vector where the illegal character
 *                  bit vector should be placed
 */

d_getpath(type, maxleng, bitvector)
  char  type;
  int  *maxleng;
  unsigned short bitvector[];
    {
    register int  length, result;
    char  packet[MAXPACKET + 2];

    length = d_watch(packet, type);

    if (length < 0)
      return(length);

/*  send the packet to the decoding routine  */

    result = d_setpath(packet, length, maxleng, bitvector);
    return(result);
    }

/*
 *     D_SETPATH
 *
 *     this routine decodes the text portion of a path packet and loads
 *     the given variables
 *
 *     packet -- pointer to the path packet
 *
 *     length -- length of the path packet
 *
 *     maxleng -- pointer to where the maximum packet length given in the
 *                packet should be loaded
 *
 *     bitvector -- pointer to the bit vector to be loaded
 */

d_setpath(packet, length, maxleng, bitvector)
  char  packet[];
  int  length, *maxleng;
  unsigned short bitvector[];
    {
    extern int  d_errno;
    register int  dig1, dig2, max;
    int  result;

/*  check the length  */

    if (length != LPATH)
      {
#ifdef D_LOG
      d_log("d_setpath", "bad length (%d) path packet received", length);
#endif D_LOG
      d_errno = D_PATHERR;
      return(D_FATAL);
      }

/*  decode the maximum packet length  */

    dig1 = d_fromhex(packet[TEXTOFF]);
    dig2 = d_fromhex(packet[TEXTOFF + 1]);

    if ((dig1 < 0) || (dig2 < 0))
      {
#ifdef D_LOG
      d_log("d_setpath", "bad hex character in path (length '%c%c', type '%c')",
          dig1, dig2, packet[TYPEOFF]);
#endif D_LOG
      d_errno = D_PATHERR;
      return(D_FATAL);
      }

    max = (dig1 << 4) | dig2;

    if ((max < LPATH) || (max > MAXPACKET))
      {
#ifdef D_LOG
      d_log("d_setpath", "bad max packet size (%d) in path packet, type '%c'",
          max, packet[TYPEOFF]);
#endif D_LOG
      d_errno = D_PATHERR;
      return(D_FATAL);
      }

    *maxleng = max;

/*  decode the illegal character string  */

    result = d_bldcvec(&packet[TEXTOFF + 2], bitvector);
    return(result);
    }
