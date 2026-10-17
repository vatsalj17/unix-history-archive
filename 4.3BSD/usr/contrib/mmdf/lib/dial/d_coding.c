# include  "util.h"
# include  "d_proto.h"
# include  "d_returns.h"

# define  TAB         '\11'     /*  tab  */
# define  LINEFEED    '\12'     /*  line feed  */
# define  FORMFEED    '\14'     /*  form feed  */
# define  CR          '\15'     /*  carriage return  */

/* speed optimization */

#define	D_TESTBIT(b,v)	((!isascii(b))||((v)[(b)>>4]&(1<<((b)&017))))

/*
 *     D_BLDCVEC
 *
 *     this routine builds a code vector from a hex digit string which
 *     specifies 'illegal' characters for transmission to a host.  each
 *     of the digits in the input vector to be sure that it is a legal
 *     hex digit.
 *
 *     hexvec -- pointer to a 32 character string of hex digits.
 *
 *     codevec -- pointer to an 8 word bit map table for the output.
 */

d_bldcvec(hexvec, codevec)
  register char  *hexvec;
  unsigned  short codevec[];
    {
    extern int  d_errno;
    register int  word, digit;
    unsigned short bits;
    int  value;

/*  each word of the code vector is constructed from 4 hex digits  */

    for (word = 0; word <= 7; word++)
      {
      bits = 0;

      for (digit = 0; digit <= 3; digit++)
        {
        if ((value = d_fromhex(*hexvec++)) < 0)
          {
#ifdef D_LOG
          d_log("d_bldcvec", "bad hex digit in path illegal character field");
#endif D_LOG
          d_errno = D_PACKERR;
          return(D_FATAL);
          }

        bits |= (value & 017) << (4 * digit);
        }

      codevec[word] = bits;
      }

    return(D_OK);
    }

/*
 *     D_BLDHVEC
 *
 *     this routine takes a code vector and builds a string of 32 hex
 *     digits for transmission in a RESET packet.
 *
 *     codevec -- pointer to the code vector to be converted
 *
 *     hexvec -- pointer to place to load the 32 hex digits
 */

d_bldhvec(codevec, hexvec)
  unsigned  short codevec[];
  register char  *hexvec;
    {
    register unsigned  bits, digit;
    int  word;

    for (word = 0; word <= 7; word++)
      {
      bits = codevec[word];

      for (digit = 0; digit <= 3; digit++)
        {
        *hexvec++ = d_tohex(bits & 017);
        bits >>= 4;
        }
      }

    *hexvec = '\0';
    return(D_OK);
    }

/*
 *     D_DECODE
 *
 *     this routine takes encoded strings and forms the plaintext
 *     output.
 *
 *     input -- input string to be decoded
 *
 *     ninput -- the number of characters in the input string
 *
 *     output -- pointer to place for the decoded output
 *
 *     the routine returns the number of output characters.
 */

d_decode(input, ninput, output)
  register char  *input, *output;
  register int  ninput;
    {
    extern char  d_rcvesc;
    extern int  d_errno;
    int  chigh, clow, noutput;

    noutput = 0;

/*  cycle through the input string.  if a character isn't the escape, just  */
/*  copy it to the output.                                                  */

    while (ninput)
      {
      if (*input != d_rcvesc)
        {
        *output++ = *input++;
        ninput--;
        noutput++;
        continue;
        }

/*  must be the escape character.  if the next one is the escape also  */
/*  then load one of them into the output.  otherwise switch for the   */
/*  special cases.                                                     */

      input++;

      if (*input == d_rcvesc)
        {
        *output++ = *input++;
        ninput -= 2;
        noutput++;
        continue;
        }

      switch (*input)
        {
        case  'I':

            *output++ = TAB;
            break;

        case  'J':

            *output++ = LINEFEED;
            break;

        case  'L':

            *output++ = FORMFEED;
            break;

        case  'M':

            *output++ = CR;
            break;

        default:

            chigh = d_fromhex(*input++);
            clow = d_fromhex(*input++);

            if ((chigh < 0) || (clow < 0))
              {
#ifdef D_LOG
              d_log("d_decode", "illegal character encoding");
#endif D_LOG
              d_errno = D_PACKERR;
              return(D_FATAL);
              }

            *output++ = (chigh << 4) | (clow & 017);
            noutput++;
            ninput -= 3;;
            continue;
        }

      input++;
      ninput -= 2;
      noutput++;
      }

    return(noutput);
    }

/*
 *     D_CCODE
 *
 *     this routine computes the encoding for a single character, if any,
 *     based on the bit vector which specifies whether the target host
 *     can receive the character without undesired side effects.
 *
 *     c -- the character to be encoded
 *
 *     string -- place to put the encoding
 */

d_ccode(c, string)
  register unsigned c;
  register char  *string;
    {
    extern unsigned  short d_lcvec[];
    extern char  d_snesc;

/*  if we're encoding the escape character itself, just double it.  */

    if (c == d_snesc)
      {
      *string++ = d_snesc;
      *string = d_snesc;
      return(2);
      }

/*  if the bit corresponding to the character is on in the vector, then we  */
/*  have to encode it.  switch for special cases, otherwise use hex digits  */

    if (D_TESTBIT(c, d_lcvec) == 0)
      {
      *string = c;
      return(1);
      }

    *string++ = d_snesc;

    switch (c)
      {
      case  TAB:

          *string = 'I';
          return(2);

      case  LINEFEED:

          *string = 'J';
          return(2);

      case  FORMFEED:

          *string = 'L';
          return(2);

      case  CR:

          *string = 'M';
          return(2);

      default:

          *string++ = d_tohex(c >> 4);
          *string++ = d_tohex(c & 017);
          return(3);
      }
    }

/*
 *     D_TESTBIT
 *
 *     this routine is called to test if a given bit is on in a bit vector
 *     the routine returns 1 if it is, 0 otherwise.
 *
 *     bit -- the bit to test
 *
 *     bitvector -- the bit vector
 */

d_testbit(bit, bitvector)
  register int  bit;
  register unsigned  short bitvector[];
    {
	return(D_TESTBIT(bit,bitvector));
    }

/*
 *     D_SETBIT
 *
 *     this routine is called to set a bit in a bit vector.
 *
 *     bit -- the bit to set
 *
 *     bitvector -- the bit vector to set it in
 */

d_setbit(bit, bitvector)
  register int  bit;
  register unsigned  short bitvector[];
    {

    bit &= 0177;

    bitvector[bit >> 4] |= (1 << (bit & 017));

    return(D_OK);
    }

/*
 *     D_ORBITVEC
 *
 *     this routine computes a word by word logical or of the input bit
 *     vectors
 *
 *     a, b -- the input bit vector
 *
 *     out -- the bit vector which becomes the bit-wise 'or' of a and b
 */

d_orbitvec(a, b, out)
  register unsigned  short a[], b[], out[];
    {
    int  word;

    for (word = 0; word < 8; word++)
      out[word] = a[word] | b[word];

    return(D_OK);
    }

/*
 *     D_TOHEX
 *
 *     routine which returns the value of its argument as a single hex
 *     character
 *
 *     value -- the value to be converted
 */

d_tohex(value)
  register int  value;
    {

    if ((value >= 0) && (value <= 9))
      return(value + '0');

    if ((value >= 10) && (value <= 15))
      return(value - 10 + 'A');

    return(-1);
    }

/*
 *     D_FROMHEX
 *
 *     routine which returns the value of the hex character given as the
 *     argument.
 *
 *     c -- the character whose value is desired
 */

d_fromhex(c)
  register int  c;
    {

    if ((c >= '0') && (c <= '9'))
      return(c - '0');

    if ((c >= 'A') && (c <= 'F'))
      return(c - 'A' + 10);

    return(-1);
    }

/*
 *     D_SETILL
 *
 *     this routine is used to copy the illegal character bit vectors
 *
 *     in -- pointer to the source bit vector
 *
 *     out -- pointer to the destination bit vector
 */

d_setill(in, out)
  register unsigned  short in[], out[];
    {
    register int  word;

    for (word = 0; word < 8; word++)
      out[word] = in[word];

    return(D_OK);
    }
