#include  "d_proto.h"
#include  "d_returns.h"
#include <signal.h>
#include  "d_syscodes.h"


/*
 *     D_SNDDATA
 *
 *     this function is called to send the stuff in the data transmit queue.
 *
 *     eot -- set to non-zero if the EOT bit should be set on the packet
 */

d_sndata(eot)
  register int  eot;
    {
    extern char  d_xqueue[], *d_xqpt;
    extern int  d_snseq, d_xqcnt;
    register int  length, result;
    char  packet[MAXPACKET + 2];

    d_snseq = d_incseq(d_snseq);
    length = d_bldpack(DATA, d_snseq, eot, d_xqueue, packet);

    d_xqcnt = 0;
    d_xqpt = d_xqueue;

    result = d_snpkt(DATA, packet, length);
    return(result);
    }

/*
 *     D_GETDATA
 *
 *     this function is called to read a DATA packet into the receive
 *     data queue.
 */

d_getdata()
    {
    extern char  d_rdqueue[], *d_rdqpt;
    extern int  d_rdqcnt, d_rdeot;
    register int  nout, length;
    char  packet[MAXPACKET + 2];

#ifdef D_DBGLOG
    d_dbglog("d_getdata", "called for more stuff");
#endif D_DBGLOG

    length = d_watch(packet, DATA);

    if (length < 0)
      return(length);


    if ((nout = d_decode(&packet[TEXTOFF], length - 6, d_rdqueue)) < 0)
      return(nout);

    d_rdqcnt = nout;
    d_rdqpt = d_rdqueue;

    if (d_fromhex(packet[FLAGOFF]) & EOFBIT)
      d_rdeot = 1;

    return(D_OK);
    }
