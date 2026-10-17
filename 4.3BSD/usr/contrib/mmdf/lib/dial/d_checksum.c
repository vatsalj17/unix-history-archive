# include  "d_returns.h"








/*
 *     D_CSCOMP
 *
 *     routine which computes the checksum for a packet and loads it into
 *     the first 4 bytes as hex digits
 *
 *     text -- pointer to packet to be check summed.  this packet should
 *             be complete except for the checksum field.  the bytes to
 *             be included in the sum should start at text + 4;
 *
 *     length -- total length of the packet.  this count should include the
 *               4 checksum digits but not the line terminator.
 */

d_cscomp(text, length)
  char  *text;
  int  length;
    {
    register char  *cp;
    register int  sum, value;
    int  digit;

/*  add up the bytes  */

    cp = &text[4];
    sum = 0;

    while (length > 4)
      {
      sum += *cp++;
      length--;
      }

/*  build and load the checksum  */

    cp = &text[3];

    for (digit = 0; digit <= 3; digit++)
      {
      value = sum & 017;
      sum >>= 4;
      *cp-- = d_tohex(value);
      }

    return(D_OK);
    }












/*
 *     D_CSCHECK
 *
 *     this routine checks that a packet has the correct checksum.  D_OK is
 *     returned if it's alright, otherwise D_NO.
 *
 *     text -- pointer to the packet, including the checksum bytes
 *
 *     length -- number of bytes in the packet, excluding the line terminator
 */

d_cscheck(text, length)
  char  *text;
  int  length;
    {
    register char  *cp;
    register int  sum, cnum;
    int  digit, chksum, value;

/*  sum up the bytes  */

    cp = &text[4];
    sum = 0;

    for (cnum = 4; cnum < length; cnum++)
      sum += *cp++;

/*  convert the checksum and see if ti matches what we've just computed  */

    cp = text;
    chksum = 0;

    for (digit = 0; digit <= 3; digit++)
      {
      if ((value = d_fromhex(*cp++)) < 0)
        return(D_NO);

      chksum = (chksum << 4) | value;
      }

    if (chksum == sum)
      return(D_OK);

    return(D_NO);
    }
