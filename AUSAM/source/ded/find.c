#include "ded.h"
#include "match.h"
#include "signal.h"

extern char *cachebuf();
extern char *cachesetup();
int ms_line, ms_col; /* where the match started */
int mf_line, mf_col; /* and where it finished */

matchinterrupt()
 { signal(SIGINTR, ttyinterrupt);
    tdiag("! search abandoned by ^C");
 }

find(pat, direction, p_fline, p_fcol)
struct RE pat[];
int direction, p_fline, p_fcol;
 { register int fline, fcol;
    int startline, startcol, length;
    char *buf;

    fline = p_fline; fcol=p_fcol;

    startline = fline;
    buf = cachesetup(fline);

    if (fcol > (length = strlength(buf)) )
      fcol = length;

    if (fcol==0)
      startcol = fcol = -1;
    else
      startcol=fcol;

    /* if ^C is typed during a search, abort it and leave the
     * command routine.
     */
    signal(SIGINTR, matchinterrupt);

    do
     { if (direction==FORWARD)
	 { if (fcol<0) fcol=0;
	    fcol++;
	    if (fcol>length)
	     { fline++;

		if (fline>maxl)
		 { fline=0; buf = cachesetup(0); }
		else buf = cacheshuffle(FORWARD);

		length=strlength(buf);
		fcol = -1;
	     }
	 }
	else
	if (direction==BACKWARD)
	 { fcol--;
	    if (fcol<0)
	     { fline--;

		if (fline<0)
		 { fline=maxl; buf = cachesetup(maxl); }
		else buf = cacheshuffle(BACKWARD);

		fcol = length = strlength(buf);
	     }
	    if (fcol==0) fcol = -1;
	 }

	if ( match(pat, fline, fcol, buf, &mf_line, &mf_col, &buf) )
	 { ms_col = (fcol<0 ? 0 : fcol);
	    if (mf_col==EOLCOL) mf_col = strlength(buf);
	    signal(SIGINTR, ttyinterrupt);
	    return(true);
	 }
     } while (fline!=startline || fcol!=startcol);

    signal(SIGINTR, ttyinterrupt);
    return(false);
 }

/****************************************************************
 * three procedures to help the matcher in multi-line matches.  *
 *                                                              *
 * Because of the restriction on matches that they all fit on   *
 *      the screen, we can get away with a small buffer         *
 ****************************************************************/

struct CACHE
 { int empty;
    char *cbuf;
 };

struct CACHE *cache;

char *cachesetup(fline)
int fline;
 { register int i;

    /* set up the space on first entry */
    if (auxscreen==0)
     { auxscreen = newscreen(); cache = my_alloc(2*2*NINROWS); }

    for (i=0; i<NINROWS; i++)
     { cache[i].cbuf = auxscreen[i];
	cache[i].empty = true;
     }
    ms_line=fline;
    return(cachebuf(fline));
 }

char *cachebuf(fline)
int fline;
 { register int i;
    i = fline-ms_line;
    if (i<0 || i>=NINROWS)
      editerror("fline (%d) out of range (ms_line %d) in cachebuf",
			fline, ms_line);
    else
    if (cache[i].empty)
     { getline(fline,cache[i].cbuf); cache[i].empty = false; }

    return(cache[i].cbuf);
 }

char *cacheshuffle(direction)
int direction;
 { register int i;
    register char *buf;

    if (direction==FORWARD)
     { buf = cache[0].cbuf;
	for (i=1; i<NINROWS; i++)
	 { cache[i-1].empty = cache[i].empty;
	    cache[i-1].cbuf = cache[i].cbuf;
	 }
	cache[NINROWS-1].empty = true;
	cache[NINROWS-1].cbuf = buf;
	ms_line++;
     }
    else
    if (direction==BACKWARD)
     { buf = cache[NINROWS-1].cbuf;
	for (i=NINROWS-1; i>0; i--)
	 { cache[i].empty = cache[i-1].empty;
	    cache[i].cbuf = cache[i-1].cbuf;
	 }
	cache[0].empty = true;
	cache[0].cbuf = buf;
	ms_line--;
     }

    return(cachebuf(ms_line));
 }
