#
# include  <ctype.h>
# include  <stdio.h>
# include  "d_proto.h"
# include  "d_returns.h"

extern int d_nfields;
extern char *d_fields[];
extern char *strcpy();
extern char *strncpy();

/*
 *     D_LINPARSE
 *
 *     routine which takes a line and breaks it into fields on blanks.  it
 *     simply accumulates strings embedded in quotes without regard to blanks
 *     until the closing quote.  the input string is altered in place by the
 *     routine by the intsertion of nulls where there was blanks.
 *
 *     fields - pointer to array which will be loaded with pointers to the
 *              fields in the input string.
 *
 *     linebuf - pointer to the input line buffer
 *
 *     maxfields - the maximum number of fields allowed on a line
 *
 *     the routine returns -1 if there are more than the indicated maximum
 *     number of fields; -2 for unclosed quoted strings.  in the normal
 *     case the number of fields isolated is returned.
 */

d_linparse(fields, linebuf, maxfields)
  char  *fields[], linebuf[];
  int  maxfields;
    {
    char *cp, *ncp;
    register int nfields, result;

    nfields = 0;

    if ( (result = d_dollar(linebuf)) < 0)
	return (result);

    cp = linebuf;

    for (;;)
    {
	result = d_getword(cp, &fields[nfields], &ncp);
	switch (result)
	{
	    case -1:
		/*  end of line  */
		return (nfields);

	    case -2:
		/*  unterminated quotes  */
		return (-2);

	    default:
		/*  another call, another word  */
		break;
	}
	if (++nfields > maxfields)
	    return (-1);
	cp = ncp;
    }
}



d_getword (linebuf, start, next)
  char *linebuf;
  char **start, **next;
    {
    register char *cp;

    cp = linebuf;

    /*  skip over any leading white space  */
    while ((*cp == ' ') || (*cp == '\t'))
        *cp++ = '\0';

    /*  return -1 to indicate that there was no word  */
    if ((*cp == '\n') || (*cp == '\0'))
    {
        *cp = '\0';
        return (-1);
    }

    /*  save the pointer to the start of the field  */
    *start = cp;

    /*  special case for quoted strings.  embedded blanks are
     *  ignored.  must find another '"' before the newline or null.
     */
    if (*cp == '"')
    {
        cp++;

        while ((*cp != '"') && (*cp != '\n') && (*cp != '\0'))
            cp++;

        if ((*cp == '\n') || (*cp == '\0'))
            return(-2);

        cp++;
    }
    else
        while ((*cp != ' ') && (*cp != '\t') &&
               (*cp != '\n') && (*cp != '\0'))
            cp++;
    *cp++ = '\0';
    *next = cp;
    return (0);
}



d_dollar(line)
  char *line;
    {
    char templine[MAXSCRLINE], number[10];
    register char *cp;
    register int index;
    int changes = 0;
    int len = 0;

    cp = line;

    for ( ; ;)
    {
      switch (*cp)
      {
	case '\0':
	case '\n':
	     if (changes)
	     {
		templine[len++] = '\0';  
	        (void) strcpy(line, templine);
	        line[len] = '\0'; /* parser requires 2 nulls */
	     }

	     return(0);

	case '$': 
	     changes++;
	     cp++;

	     if (*cp == '$')  /* double $$ expands to $ */
	     {
	        if (addout(templine, &len,  cp++, 1))
		    return(-5);
	        break;
	     }

     	     /* gobble up number and expand */
	     for (index = 0; 
		  isdigit(*cp) && (*cp!='\0') && (*cp!='\n') && (index<10);
		  index++)
     	        number[index] = *cp++;
	     number[++index] = '\0';
     	     index = atoi(number);
#ifdef D_DBGLOG
	     d_dbglog("d_dollar", "variable number: %d, max: %d", 
		      index, d_nfields);
#endif
     	     if ((index <= 0) || (index > d_nfields))
     	        return(-4);
	     if (addout(templine,&len, d_fields[index],strlen(d_fields[index])))
		return(-5);
	     break;

	default:
	     if (addout(templine, &len, cp++, 1))
	        return(-5);
             break;
       }
   }        
}	    

addout(buf, len, str, cnt)
char *buf, *str;
int *len, cnt;
{
    if (*len + cnt >= MAXSCRLINE - 2)
	return(1);
    (void) strncpy(&buf[*len],str,cnt);
    *len += cnt;    
    return(0);
}
