# include  "util.h"
# include  "d_returns.h"
# include  "d_proto.h"
# include  <stdio.h>
# include  "d_structs.h"

/*
 *     D_NUMPARSE
 *
 *     this routine is called to parse a phone number specification string
 *     of the form:
 *
 *     speed|phone number, speed|phone number, ...
 *
 *     such as:
 *
 *     300|9:738-8003,1200|9:738-2928
 *
 *     and fill a table of structures with the appropriate information.
 *     the elements of each table entry contain the speed index corresponding
 *     to the speed designation (300, 1200, etc.) and the phone number is
 *     converted from the external form into that required by the acu driver.
 *
 *     string - pointer to specification string as above
 *
 *     numtab - pointer to array of structures for the speed index, phone
 *              number pairs
 *
 *     filename - name of the file from which the specification string came.
 *                this name is reported when errors are encountered to aid
 *                in debugging.
 *
 *     lineno - number of the line in the above file which contains the
 *              string
 *
 *
 *      Mar 84 D. Long  added support for "<type>" in phone numbers 
 */

d_numparse(string, numtab, maxnums, filename, lineno)
  char  *string, *filename;
  struct telspeed  *numtab;
  int  maxnums;
  unsigned lineno;
    {
    register char  *in, *ap, *cp;
    int  count;
    char  accum[30], tempbuf[50];
    int  index, num, linetype;

    in = string;
    num = 0;
#ifdef D_DBGLOG
    d_dbglog("d_numparse", "parsing '%s'", string);
#endif D_DBGLOG

/*  treat each speed/phone number pair separately  */

    while (1)
      {

/*  isolate a number and copy it into a temporary buffer  */

      if (*in == ',')
        in++;

      if (*in == '\0')
        return(num);

      if (num >= maxnums)
      {
	  d_fillog(filename, lineno, "d_numparse",
			"more than %d phone numbers", maxnums);
	  return(D_FATAL);
      }

      cp = tempbuf;
      count = 0;

      while (*in && (*in != ','))
	if (++count > 50)
          {
          d_fillog(filename, lineno, "d_numparse", "phone number too long");
          return(D_FATAL);
          }

        else
          *cp++ = *in++;

      *cp = '\0';
      cp = tempbuf;

/*  if the string begins with a letter, then it is taken to be the name of  */
/*  a direct connect line.  the speed index will be set to 0.               */

      if (isalpha(tempbuf[0]))
        {
        if (strlen(tempbuf) > 25)
          {
          d_fillog(filename, lineno, "d_numparse", "direct line name too long");
          return(D_FATAL);
          }

        (void) strcpy(numtab[num].t_number, tempbuf);
        numtab[num].t_speed = 0;
#ifdef D_DBGLOG
	d_dbglog ("d_numparse", "%d: (speed %d) num '%s'",
		    num, numtab[num].t_speed, numtab[num].t_number);
#endif D_DBGLOG
        num++;

        continue;
        }

/*  if the string to be parsed contains a '|', then it is supposed to  */
/*  have a preceeding speed indicator.  get at most 4 digits for this  */

      if (d_instring('|', tempbuf))
        {
        ap = accum;
        count = 0;

        while ((*cp != '|') && (++count <= 4))
          if (isdigit(*cp))
            *ap++ = *cp++;

          else
            {
            d_fillog(filename, lineno, "d_numparse", "bad character in speed");
            return(D_FATAL);
            }

        if (count > 4)
          {
          d_fillog(filename, lineno, "d_numparse", "invalid speed");
          return(D_FATAL);
          }

/*  now look up the string to get the index  */

        *ap = '\0';

        if ((index = d_spdindex(accum)) < 0)
          {
          d_fillog(filename, lineno, "d_numparse", "unknown speed");
          return(D_FATAL);
          }

        numtab[num].t_speed = index;
        cp++;
        }

/*  if there's no speed given, assume a default of 300 baud  */

      else
        numtab[num].t_speed = 7;

/*  accumulate up to 25 characters of phone number  */

      ap = numtab[num].t_number;
      count = 0;
      linetype = 0;

      while (*cp && (*cp != ',') && (count < 25))
        {

        if (isdigit(*cp) || (linetype && (*cp != '>')) )
          {
          *ap++ = *cp++;
          count++;
          continue;
          }

        switch (*cp)
          {
          case '(':
          case ')':
          case '-':

              cp++;
              continue;

          case ':':

	      *ap++ = '=';      /* wait for next dial tone */
              count++;
              cp++;
              continue;

	  case '<':
	      *ap++ = *cp++;
	      count++;
              linetype = 1;
              continue;

	  case '>':
	      *ap++ = *cp++;
	      count++;
	      linetype = 0;
              continue;

	  default:              /* may be wrong, but let it pass  */

              d_fillog(filename, lineno, "d_numparse",
		  "illegal dial character '%c' (%3o'O) in phone number?",
                  *cp, *cp);
              count++;
              cp++;
              continue;

          }
        }

/*  make sure the number isn't too long  */

      if (count >= 25)
        {
        d_fillog(filename, lineno, "d_numparse", "phone number too long");
        return(D_FATAL);
        }

/*  make sure the <> were balanced */

      if (linetype)
        {
        d_fillog(filename, lineno, "d_numparse", "Line type field invalid");
        return(D_FATAL);
        }

      *ap = '\0';

#ifdef D_DBGLOG
      d_dbglog ("d_numparse", "%d: (speed %d) num '%s'",
		  num, numtab[num].t_speed, numtab[num].t_number);
#endif D_DBGLOG
      num++;
      }
    }

/*
 *     D_SPDINDEX
 *
 *     this routine looks up the speed designation string in the table
 *     of legal identifiers and returns the speed index.
 *
 *     speed -- pointer to the speed string
 */

d_spdindex(speed)
  register char  *speed;
    {
    extern struct speedtab  d_spdtab[];
    extern int  d_errno;
    register struct speedtab  *sp;

    sp = d_spdtab;

    while (sp -> s_speed)
      {
      if (strcmp(sp -> s_speed, speed) == 0)
        return(sp -> s_index);

      sp++;
      }

    d_errno = D_BADSPD;
    return(D_FATAL);
    }

/*
 *     D_INSTRING
 *
 *     routine which returns a non-zero value if the specified character
 *     is in the string.
 *
 *     c -- the character to be searched for
 *
 *     string -- the string to search
 */

d_instring(c, string)
  char  c;
  register char  *string;
    {

    while (*string)
      if (c == *string++)
        return(1);

    return(0);
    }
