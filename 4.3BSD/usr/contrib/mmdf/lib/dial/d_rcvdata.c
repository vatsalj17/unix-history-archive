# include  "d_returns.h"

/*
 *     D_RCVDATA
 *
 *     this routine is called to read data bytes from the other side.
 *     if there are currently any unread characters in the internal
 *     read queue, they are returned before another packet is read.
 *
 *     buffer -- place to load the data
 *
 *     nbytes -- the maximum number of bytes that should be returned
 *               if an oet is found, then not all that the caller
 *               requests may be returned.
 *
 *     eot -- pointer to a variable that is set if the packet, or the
 *            one queued, contained an EOT
 */

d_rcvdata(buffer, nbytes, eot)
  register char  *buffer;
  register int  nbytes;
  int  *eot;
    {
    extern char  *d_rdqpt;
    extern int  d_rdqcnt, d_rdeot;
    register int  count;
    int  result;

    count = 0;

    while (1)
      {
      /*  transfer data until the max is reached or this packet
       *  is emptied.
       */
      while (nbytes && d_rdqcnt)
        {
        *buffer++ = *d_rdqpt++;
        count++;
        nbytes--;
        d_rdqcnt--;
        }

      /*  If there are no more packets available, return EOT  */
      if ((d_rdqcnt == 0) && d_rdeot)
        {
        *eot = 1;
        d_rdeot = 0;
        return(count);
        }

      /*  If there is no more room to transfer to, just return  */
      if (nbytes == 0)
        {
        *eot = 0;
        return(count);
        }

      /*  Else we come here.  No more data in this packet, but there
       *  are more packets to read.  Do it.
       */
      if ((result = d_getdata()) < 0)
        return(result);
      }
    }
