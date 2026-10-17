# include  "d_returns.h"

extern char  d_xqueue[], *d_xqpt;
extern int  d_xqcnt, d_maxtext;
  
/*
 *     D_SNDSTREAM
 *
 *	this routine is called to send data
 *	to the other side without necessarily
 *	implying any packet boundaries.
 *
 *     buffer -- pointer to the data to be sent
 *
 *     nbytes -- number of bytes to be sent
 */

d_snstream(buffer, nbytes)
  char  *buffer;
  int  nbytes;
    {
    register int  length, result;
    register char  *bp;

#ifdef D_DBGLOG
    d_dbglog("d_snstream", "sending  '%.*s'", nbytes, buffer);
#endif D_DBGLOG

    bp = buffer;

    while (nbytes--)
      {
      length = d_ccode(*bp & 0177, d_xqpt);

/*  if the encoding of this character will overflow the text, send off what  */
/*  we've already got.                                                       */

      if ((length + d_xqcnt) > d_maxtext)
        {
	/*  Note that this does not actually send the character that
	 *  would overflow the queue.  It never increments the pointer.
	 *  It overwrites the queued char(s) with a null.  It resets
	 *  (adds 1 to) the nbyte counter.  The continue causes it
	 *  to requeue the same char.
	 */
        *d_xqpt = '\0';

	if ((result = d_sndata(0)) < 0)
          return(result);

        nbytes++;
        continue;
        }

/*  otherwise adjust the count and pointers  */

      d_xqcnt += length;
      d_xqpt += length;
      bp++;
      }

    return(D_OK);
    }

/*
 *     D_SNDEOT
 *
 *     this routine is called to flush the transmit buffer and mark the
 *     packet with an EOT
 */

d_sneot()
    {
    register int  result;

    *d_xqpt = '\0';
#ifdef D_DBGLOG
    d_dbglog("d_sneot", "packet '%s'", d_xqueue);
#endif D_DBGLOG

    result = d_sndata(1);
    return(result);
    }
