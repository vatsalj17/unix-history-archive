#include  "d_proto.h"
#include <signal.h>
#include  "d_syscodes.h"
#include  "d_returns.h"

/*
 *     D_GETNBUFF
 *
 *     this routine watches for an incoming NBUFF packet from the remote
 *     host.  it thens set the transmit buffering limit.
 *
 */

d_getnbuff()
    {
    extern int  d_errno;
    extern int  d_nbuff;
    register int  length;
    char  packet[MAXPACKET + 2];

#ifdef D_DBGLOG
    d_dbglog("d_getnbuff", "looking for NBUFF from other end");
#endif D_DBGLOG

/*  set timer so we don't wait forever  */

    length = d_watch(packet, NBUFF);

    if (length < 0)
	return(length);

    if (length != LNBUFF)
    {
#ifdef D_DBGLOG
      d_dbglog("d_getnbuff", "bad NBUFF packet length (%d)", length);
#endif D_DBGLOG
      d_errno = D_INITERR;
      return (D_FATAL);
    }

      d_nbuff = d_fromhex(packet[NBOFF]);
      d_nbuff = (d_nbuff << 4) | d_fromhex(packet[NBOFF + 1]);
      return(D_OK);
    }
