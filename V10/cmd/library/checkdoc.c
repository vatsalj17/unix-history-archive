/*	library:checkdoc.c	1.7	*/
#include "sccsid.h"
VERSION(@(#)library:checkdoc.c	1.7)

/*************   checkdoc.c   ***************/

/* This routine checks whether a given document id is legal.
 * It checks in file
 *	WHERE/known.list
 * to see if the document id meets any of the document ID formats
 * listed there.
 *
 * The lines in known.list  have 3 forms:
 *   	start with ? mark - help text
 *	start with #   - comments
 *	start with ^regular expression.  This causes all the following
 *	      lines to be scanned, ignoring if the given expression starts the
 *	      document id
 *	all others are regular expression document ID formats.
 * Return value:
 *	<0   on failure.
 *	0    success - but no remarks allowed
 *	>0   success - remarks allowed.
 */
#include <stdio.h>
#include <ctype.h>
static char *exp_ptr;

#define INIT		register char *sp = exp_ptr;
#define GETC()		(*sp++)
#define PEEKC()		(*sp)
#define UNGETC(x)	(--sp)
#define RETURN(c)	{ exp_ptr = sp; return(c); }
#define ERROR(c)	return(NULL);
#include "regexp.h"
#define LBSIZE 256
char *strchr();

checkdoc(str)
char *str;
 {
    char expbuf[LBSIZE], rhsbuf[LBSIZE];
    char genbuf[LBSIZE], line[100];
    register char *ptr, *mstr;
    static FILE *infile=NULL;
    long value;
    static char *bchecks="ABCDEFGHJKLMNPQRTSVWXYZ";
    static char *schecks="ABCDEFHJKLMNPQRTVWXY";

    if (infile == NULL) {
	    sprintf(line, "%s/known.list", WHERE);
	    if ((infile = fopen(line, "r")) == NULL) {
		fprintf(stderr, "No ID format file!! No requests sent!!\n");
		exit(1);
		}
	    }
    rewind(infile);
    *line = '^';
    mstr = str;
    while (fgets(line+1, 100, infile) != NULL) {
	ptr = line+1;
	if (*ptr == '?') continue;
	if (*ptr == '#') continue;
	if (*ptr == '\n') continue;
	if (*ptr == '^') {
	    ptr++;
	    /* just a ^ resets to start of string */
	    if (*ptr == '\n') {
		mstr = str;
		continue;
		}
	    *expbuf = 0;
	    exp_ptr = line+1;
	    if (compile((char *) 0, expbuf, expbuf+LBSIZE, '\t') == NULL)
		    return(-2);
	    compsub(rhsbuf, '\t'); 
	    if (match(expbuf, 0, mstr, genbuf)) mstr = loc2;
	    continue;
	    }
	*expbuf = 0;
	exp_ptr = line;
	if (compile((char *) 0, expbuf, expbuf+LBSIZE, '\t') == NULL)
		return(-2);
	compsub(rhsbuf, '\t'); 
	if (match(expbuf, 0, mstr, genbuf)) {
	    ptr = strchr(line, '\t');
	    if (ptr++ == NULL) return(0);
	    if (strncmp(ptr, "REMARK", 6) == 0) return(1);
	    else if (strncmp(ptr+1, "CHECK", 5) == 0) {
		    if (strlen(loc2) != 1) return(-3);
		    /* accept a question mark */
		    if (*loc2 == '?') return(0);
		    /* get the sum of the digits - note done with mstr */
		    for (value = 0, mstr=str; *mstr; mstr++)
			if (isdigit(*mstr)) value = value * 10 + *mstr - '0';
		    /* check the check char */
		    if (*ptr == 'B')
			    ptr = bchecks;
		    else    ptr = schecks;
		    value = (value % strlen(ptr));
		    if (*loc2 != ptr[value]) return(-3);
		    return(0);
		    }
	    if (strlen(loc2)) continue; /* more id than allowed - continue */
	    return(0);
	    }
	}
    return(-1);
    }

match(expbuf, gf, linebuf, genbuf)
char	*expbuf, *linebuf, *genbuf;
{
    register char   *p1, *p2;

    if(gf) {
	    if (circf) return(0);
	    p1 = linebuf;
	    p2 = genbuf;
	    while(*p1++ = *p2++);
	    locs = p1 = loc2;
	    }
    else {
	    p1 = linebuf;
	    locs = 0;
	    }
    return(step(p1, expbuf));
    }

compsub(rhsbuf, sseof)
char    *rhsbuf, sseof;
{
    register char   *p, *q;

    p = rhsbuf;
    q = exp_ptr;
    for(;;) {
	if((*p = *q++) == '\\') {
		*p = *q++;
		if(*p > nbra + '0' && *p <= '9') return(NULL); 
		*p++ |= 0200;
		continue;
		}
	if(*p == sseof) {
		*p++ = '\0';
		exp_ptr = q;
		break;
		}
	if(*p++ == '\0') return(-1);
	}
    return(0);
    }
