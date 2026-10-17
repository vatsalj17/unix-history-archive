#include "ded.h"
#include "char.h"

firstrow()
 { register int row;

    for (row=0; row<=LASTINROW; row++)
      if (!emptyrow(row)) return(row);

    return(LASTINROW+1);
 }

lastrow()
 { register int row;

    for (row=LASTINROW; row>=0; row--)
      if (!emptyrow(row)) return(row);

    return(-1);
 }

int emptyrow(srow)
int srow;
 { return(lastcol(srow)<0); }

int emptyscreen()
 { return(lastrow()<0); }

midrow()
 { return((lastrow()+firstrow())/2); }

int firstcol(srow)
int srow;
 { register int i;
    register char *linep;

    linep = rowmap[srow];
    for (i=0; i<=ncols-1; i++)
     if (*linep++ != c_SPACE) return(i);

    return(ncols);
 }

int lastcol(srow)
int srow;
 { register int i;
    register char *linep;

    linep = rowmap[srow]+ncols-1;
    for (i=ncols-1; i>=0; i--)
      if (*linep-- != c_SPACE) return(i);

    return (-1);
 }

int firstpos(srow)
int srow;
 { register int i;
    register char *linep;

    linep = rowmap[srow]+leftcol;
    for (i=leftcol; i<=rightcol; i++)
     if (*linep++ != c_SPACE) return(i);

    return(rightcol+1);
 }

int lastpos(srow)
int srow;
 { register int i;
    register char *linep;

    linep = rowmap[srow]+rightcol;
    for (i=rightcol; i>=leftcol; i--)
      if (*linep-- != c_SPACE) return(i);

    return (leftcol-1);
 }

/* return the next tab position - multiple of 8 */
int nextab(scol)
int scol;
 { if (scol<leftcol) return(leftcol);
    scol = (scol/8)*8+8;
    return(scol>=rightcol ? rightcol : scol);
 }

charsin(r1,r2)
int r1,r2;
 { register int count, row, cc;

    count = 0;

    for (row=r1; row<=r2; row++)
     { cc = lastcol(row) - firstcol(row)+1;
	if (cc>0) count =+ cc;
     }

    return(count);
 }
