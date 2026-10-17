#include "ded.h"
#include "char.h"

/* table of characters for input (INMODE) mode */
char  ictab[] {
	/* 000 - 007, null - ^G */
	IGN,WLEFT,WRIGHT,WERASE,WRUBOUT,HEADERASE,TAILERASE,IGN,
	/* 010 - 017, ^H - ^O */
	LEFT,TAB,NL,IGN,IGN,CR,IGN,IGN,
	/* 020 - 027, ^P - ^W */
	IGN,IGN,CERASE,IGN,IGN,RIGHT,IGN,IGN,
	/* 030 - 037, ^X - ^_ */
	IGN,IGN,UP,MODEitoe,CONTROL,CHMODE,SPECIAL,DOWN,
	SPACE, ONE, ONE, ONE, ONE, ONE, ONE, ONE,
	ONE, ONE, ONE, ONE, ONE, ONE, ONE, ONE,
	ONE, ONE, ONE, ONE, ONE, ONE, ONE, ONE,
	ONE, ONE, ONE, ONE, ONE, ONE, ONE, ONE,
	ONE, ONE, ONE, ONE, ONE, ONE, ONE, ONE,
	ONE, ONE, ONE, ONE, ONE, ONE, ONE, ONE,
	ONE, ONE, ONE, ONE, ONE, ONE, ONE, ONE,
	ONE, ONE, ONE, ONE, ONE, ONE, ONE, ONE,
	TWO, ONE, ONE, ONE, ONE, ONE, ONE, ONE,
	ONE, ONE, ONE, ONE, ONE, ONE, ONE, ONE,
	ONE, ONE, ONE, ONE, ONE, ONE, ONE, ONE,
	ONE, ONE, ONE, TWO, TWO, TWO, TWO, RUBOUT,
};

/* table of characters for edit (EDMODE) mode. Much the same
 * as ictab, except for MODEetoi (replaces MODEitoe, naturally),
 * and the absence of UP or DOWN. CR or LF are also MODEetoi.
 */
char  ectab[] {
	/* 000 - 007, null - ^G */
	IGN,WLEFT,WRIGHT,WERASE,WRUBOUT,HEADERASE,TAILERASE,IGN,
	/* 010 - 017, ^H - ^O */
	LEFT, TAB, MODEetoi, IGN, IGN, MODEetoi, IGN, IGN,
	/* 020 - 027, ^P - ^W */
	IGN,IGN,CERASE,IGN,IGN,RIGHT,IGN,IGN,
	/* 030 - 037, ^X - ^_ */
	IGN,IGN,IGN,MODEetoi,CONTROL,CHMODE,SPECIAL,IGN,
	SPACE, ONE, ONE, ONE, ONE, ONE, ONE, ONE,
	ONE, ONE, ONE, ONE, ONE, ONE, ONE, ONE,
	ONE, ONE, ONE, ONE, ONE, ONE, ONE, ONE,
	ONE, ONE, ONE, ONE, ONE, ONE, ONE, ONE,
	ONE, ONE, ONE, ONE, ONE, ONE, ONE, ONE,
	ONE, ONE, ONE, ONE, ONE, ONE, ONE, ONE,
	ONE, ONE, ONE, ONE, ONE, ONE, ONE, ONE,
	ONE, ONE, ONE, ONE, ONE, ONE, ONE, ONE,
	TWO, ONE, ONE, ONE, ONE, ONE, ONE, ONE,
	ONE, ONE, ONE, ONE, ONE, ONE, ONE, ONE,
	ONE, ONE, ONE, ONE, ONE, ONE, ONE, ONE,
	ONE, ONE, ONE, TWO, TWO, TWO, TWO, RUBOUT,
};

/* convert those characters which cannot be printed or
 * typed on an ITT (CURSES)
 */
set_modifier(c)
char c;
 { register char rc, rcm;

    rc = c; rcm = c_MODIFY;

    if (rcm != 0)
      ictab[rcm] = ectab[rcm] = ONE;

    if (rc == 0)
     { two_to_one(ictab); two_to_one(ectab); }
    else
    if (ictab[rc] == ONE  && ectab[rc] == ONE)
      ictab[rc] = ectab[rc] = MODIFY;
    else editerror("invalid modifier character %o", c);

   c_MODIFY = rc;
 }

/* remove all mention of modifier character */
two_to_one(tab)
char *tab;
 { register int i;

    for (i=0; i<=127; i++)
      if (tab[i]==TWO) tab[i] = ONE;
 }

char unmodify(ch)
char ch;
 { register char c;
    c= ch;
    return (c=='{'?'(':
	    c=='}'?')':
	    c=='|'?'!':
	    c=='~'?'^':
	    c=='`'?'\'':
	    c);
 }

char modify(ch)
char ch;
 { register char c;
    c=ch;
    return (c=='('?'{':
	    c==')'?'}':
	    c=='!'?'|':
	    c=='^'?'~':
	    c=='\''?'`':
	    c);
 }

/* move cursor with minimum of fuss - this procedure is
 * only called from fixpos(), and then only when the cursor
 * needs to be moved
 */
move(srow, scol)
int srow, scol;
 { register int n_rows, n_cols;
    int ncr;
    register char *rp;

    n_rows = srow - real_c.row;
    n_cols = scol - real_c.col;
    ncr = scol+1;

    if ( n_rows>=0 && ((n_cols>=0 && n_cols<=ncr) ? n_rows+n_cols<=3 :
						 n_rows+ncr<=3))
     { if (n_cols<0 || ncr<n_cols)
	 { ttyout(c_CR); real_c.col = 0; n_cols = scol; }

	rp = rowmap[real_c.row]+real_c.col;
	while (n_cols-- > 0)
	 { ttyout(*rp++); real_c.col++; }

	while (n_rows-- > 0)
	{ ttyout(c_NL); real_c.row++; }
     }
    else
    if (ttytype == itt)
     { /* ^K, row+33, col+33 */
	ttyout(c_POSITION);
	ttyout( (real_c.row=srow) + 33);
	ttyout( (real_c.col=scol) + 33);
     }
    else
    if (ttytype == hazeltine)
     { /* tilde, DC1, column, row */
	ttyout(c_LEADIN); ttyout(021);
	ttyout( (real_c.col=scol) + (scol<=30 ? 96 : 0));
	ttyout( (real_c.row=srow) + 96);
     }
	else
	if (ttytype == VC404)
	{	/* ctrl P row+32 col+32  */
		ttyout('\020');
		ttyout((real_c.row=srow) + 32);
		ttyout((real_c.col=scol) + 32);
	}
	else
	if (ttytype == T1061)
	{	/* esc Y row+32 col+32   */
		ttyout('\033');
		ttyout('Y');
		ttyout((real_c.row=srow) + 32);
		ttyout((real_c.col=scol) + 32);
	}
    else editerror("tty type not set up (move)");
 }

complain()
 { if (ringing) ttyout(c_BELL);
    ttyflush();
 }
