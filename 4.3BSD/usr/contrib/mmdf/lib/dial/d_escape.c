#include  "d_proto.h"
#include <signal.h>
#include  "d_syscodes.h"
#include  "d_returns.h"

/*
 *     D_FINDESC
 *
 *     this routine attempts to find a suitable escape character to be used
 *     by the remote host for packets transmitted to us.  the character is
 *     chosen from the 'd_esclist' by checking each of the proposed escapes
 *     to see if it can be both transmitted by the remote system and received
 *     by us.  the selected character is returned, if one is found; otherwise
 *     D_FATAL.
 */

char
	d_findesc()
    {
    extern char  d_esclist[];
    extern unsigned  short d_lrill[], d_rxill[];
    extern int  d_errno;
    register char  *cp;

    for (cp = d_esclist; *cp; cp++)
      if (!d_testbit(*cp, d_lrill) && !d_testbit(*cp, d_rxill))
        {
#ifdef D_DBGLOG
        d_dbglog("d_findesc", "Character '%c' selected as escape for receiving",
            *cp);
#endif D_DBGLOG
        return(*cp);
        }

#ifdef D_LOG
    d_log("d_findesc", "Can't find suitable escape character");
#endif D_LOG
    d_errno = D_NOESC;
    return(D_FATAL);
}

/*
 *     D_SNDESCAPE
 *
 *     this routine forms an ESCAPE packet and sends it to the remote host
 */

d_snfescape()
    {
    extern char  d_rcvesc;
    extern int  d_snseq;
    register int  length, result;
    char  packet[MAXPACKET + 2], esctemp[4];

#ifdef D_DBGLOG
    d_dbglog("d_snfescape", "sending ESCAPE to other end");
#endif D_DBGLOG

    /*  find an escape character  */
    if ((d_rcvesc = d_findesc()) < 0)
      return(d_rcvesc);

    /*  form text containing the ascii value of the escape
     *  character expressed as a 2 digit hex number and send it.
     */
    esctemp[0] = d_tohex((d_rcvesc >> 4) & 017);
    esctemp[1] = d_tohex(d_rcvesc & 017);
    esctemp[2] = '\0';

    d_snseq = d_incseq(d_snseq);
    length = d_bldpack(ESCAPE, d_snseq, 1, esctemp, packet);

    result = d_snpkt(ESCAPE, packet, length);
    return(result);
}


/*
 *     D_GETESCAPE
 *
 *     this routine watches for an incoming ESCAPE packet from the remote
 *     host.  it thens set the transmit escape character.
 *
 */

d_getescape()
    {
    extern int  d_errno;
    extern char  d_snesc;
    register int  length;
    char  packet[MAXPACKET + 2];

#ifdef D_DBGLOG
    d_dbglog("d_getescape", "looking for ESCAPE from other end");
#endif D_DBGLOG

/*  set timer so we don't wait forever  */

    length = d_watch(packet, ESCAPE);

    if (length < 0)
	return(length);

    if (length != LESCAPE)
    {
#ifdef D_DBGLOG
      d_dbglog("d_escape", "bad ESCAPE packet length (%d)", length);
#endif D_DBGLOG
      d_errno = D_INITERR;
      return (D_FATAL);
    }

      d_snesc = d_fromhex(packet[ESCOFF]);
      d_snesc = (d_snesc << 4) | d_fromhex(packet[ESCOFF + 1]);
      return(D_OK);
    }
