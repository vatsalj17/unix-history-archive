#include "ded.h"
#include "match.h"
#include "char.h"

/* two macro definitions, to help with writing recursive calls */
#define REC fline, fcol, buf, &fline, &fcol, &buf
#define TAILREC fline, fcol, buf, resline, rescol, resbufp


/************************************************************************
 * backtracking pattern matcher, capable of making multi-line matches   *
 *                                                                      *
 * the (enormous number of) parameters are as follows -                 *
 *                                                                      *
 *  pat : an RE pattern, built by build_re                              *
 *                                                                      *
 *  fline, fcol : the current match position                            *
 *      (fcol is -1 at the start of the line,  EOLCOL at the end of the *
 *       line, to help with 'begin-line' and 'end-line' character       *
 *       processing)                                                    *
 *                                                                      *
 *  buf : address of current line buffer                                *
 *                                                                      *
 *  resline, rescol : addresses of variables which must receive         *
 *      the values of fline, fcol should the match succeed              *
 *                                                                      *
 *  resbufp : the address of a variable which must                      *
 *      receive the address of current line buffer should the match     *
 *      succeed                                                         *
 *                                                                      *
 *  NOTE : multi-line matches carry a run-time overhead,                *
 *      though by providing a special screen buffer I avoid use of the  *
 *      heap. The screen buffer manipulations are efficient and clear   *
 *      (I hope!) and in fact this procedure would be more complicated  *
 *      and less efficient had it used the heap or the stack.           *
 ************************************************************************/

int match(pat, fline, fcol, buf, resline, rescol, resbufp)
struct RE pat[];
int fline, fcol;
char *buf;
int *resline, *rescol;
char **resbufp;
 { register struct RE *pat_element;
    register char *buf_element, c;

    pat_element = pat;
    buf_element = buf+(fcol<0 ? 0 : fcol);

    while (true)
     { switch (pat_element->type)
	 { case p_FIN:
		/* success !!!! */
	    succeed:
		*resline = fline;
		*rescol = fcol;
		*resbufp = buf;
		return(true);

	    case p_CHAR:
		if (fcol==EOLCOL) break;
		else
		if (*buf_element++ != (pat_element++)->info)
		  return(false);
		else
		 { fcol = buf_element-buf; continue; }

	    /* separator between words ('<space>) */
	    case p_SEP:
		if (fcol==EOLCOL) break;
		else
		if (*buf_element==c_SPACE ||
		    *buf_element==0)
		 { while (*buf_element==c_SPACE) buf_element++;
		    if (*buf_element==0)
		     { fcol = EOLCOL;
			if (match(pat_element, TAILREC)) return(true);
		     }
		    else
		      fcol = buf_element-buf;

		    pat_element++;
		    continue;
		 }
		else
		  return(false);

	    case p_ANY:
		if (fcol==EOLCOL) break;
		else
		if (*buf_element++ == 0) return(false);
		else
		 { fcol = buf_element-buf; pat_element++; continue; }

	    case p_BOL:
		/* multiple begin-line characters don't have much
		 * meaning - at least not one I can program
		 */
		if (fcol==EOLCOL) break;
		else
		if (fcol >= 0) return(false);
		else
		 { fcol = 0; pat_element++; continue; }

	    case p_EOL:
		/* this code implements multiple end-line
		 * characters rather neatly
		 */
		if (fcol==EOLCOL) break;
		else
		if (*buf_element != 0) return(false);
		else
		 { pat_element++; fcol = EOLCOL; continue; }

	    case p_OPT:
		/* '? - e.g. '(abc')'?def -
		 * searches for LONGEST matching string
		 */
		if ( match(pat_element+1, REC) )
		  buf_element = buf+(fcol<0 ? 0 : fcol);
		pat_element =+ pat_element->info;
		continue;

	    case p_PLUS:
		/* '+ - e.g. abc'[0-9']'+ -
		 * searches for LONGEST matching string
		 */
		if (match(pat_element+1, REC))
		  buf_element = buf+(fcol<0 ? 0 : fcol);
		else return(false);

		if (match(pat_element, REC)) goto succeed;
		else
		 { pat_element =+ pat_element->info; continue; }

	    case p_PPLUS:
		/* '++ - e.g. .fn'$'++.en -
		 * searches for SHORTEST matching string
		 * (useful for multi-line matches)
		 */
		while (true)
		 { if (!match(pat_element+1, REC)) return(false);

		    if (match(pat_element+pat_element->info, REC))
		      goto succeed;
		 }

	    case p_STAR:
		/* '* - e.g. '^'.'*'$ -
		 * searches for LONGEST matching string
		 */
		if (bothmatch(pat_element+1, pat_element, REC))
		  goto succeed;
		else
		 { pat_element =+ pat_element->info; continue; }

	    case p_SSTAR:
		/* '** - e.g. '^.fn'$'('.'*')'**'^-*'$ -
		 * searches for SHORTEST matching string -
		 * like '++ this is useful in multi-line matches
		 */
		while (true)
		 { if (match(pat_element+pat_element->info, REC))
		      goto succeed;

		    if (!match(pat_element+1, REC)) return(false);
		 }

	    case p_INSET:
	    case p_OUTSET:
		if (fcol==EOLCOL) break;
		else
		 { c = *buf_element++;

		    if (matchset(c,pat_element+1,fcol) ==
			(pat_element->type==p_INSET ? false : true) )
		      return(false);
		    else
		     { pat_element =+ pat_element->info;
			if (c==0) fcol = EOLCOL;
			else fcol = buf_element-buf;
			continue;
		     }
		 }

	    default:
		editerror("invalid pattern type (%d) in match",
				pat_element->type);
	 }

	/* end of switch - come here on break (only when fcol==EOLCOL)
	 * - read in next line
	 * - do not match past end of file, and do not match more than a
	 * single screen full of data
	 */
	fline++;
	if (fline>maxl || fline-ms_line>=NINROWS) return(false);

	buf = cachebuf(fline);
	buf_element = buf;
	fcol = -1;
     } /* end of while (true) */
 }
/* end of match procedure */

/************************************************************************
 *  a procedure to help with multiple backtracking - when the first     *
 *      part of a multiple match works, subsequent matches must proceed *
 *      from that position, but when any part fails, matching must      *
 *      backtrack right over any successful previous matches            *
 ************************************************************************/

int bothmatch(pat1,pat2,fline,fcol,buf,resline,rescol,resbufp)
struct RE pat1[], pat2[];
int fline, fcol;
char *buf;
int *resline, *rescol;
char **resbufp;
 { if (match(pat1, REC))
      return(match(pat2,TAILREC));
    else
      return(false);
 }
/* end of bothmatch procedure */

/************************************************************************
 * procedure to help with matching character sets - e.g. '[a-z0-9']     *
 ************************************************************************/

/* memo to self : do something about efficiency of 0-9, a-z etc. */
int matchset(p_c, set, fcol)
char p_c;
struct RE set[];
 { register struct RE *set_element;
    register char c;

    c = p_c;
    set_element = set;

    while(true)
     { switch (set_element->type)
	 { case p_CHAR:
		if (c==set_element->info) return(true);
		else break;

	    case p_BOL:
		if (fcol<0) return(true);
		else break;

	    case p_EOL:
		if (c==0) return(true);
		else break;

	    case p_ONEOF:
		/* there follows a piece of horrid conciseness -
		 * C lovers would like it, I suppose
		 */
		if ((set_element++)->info<=c && c<=set_element->info)
		  return(true);
		else break;

	    case p_FIN:
		return(false);

	    default:
		editerror("invalid set type (%d) in matchset",
				set_element->type);
	 }

	set_element++;
     }
 }
/* end of matchset procedure */
