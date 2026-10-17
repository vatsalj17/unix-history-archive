#include "ded.h"
#include "match.h"
#include "char.h"

int c_row, c_col;

/* this procedure, to avoid inefficiency, uses auxscreen
 * and rowmap to manipulate the screen. This is tricky.
 */

change(pat,str,direction,fin_ch,fline,fcol)
struct RE pat[];
char *str;
int direction;
char fin_ch;
int fline, fcol;
 { char line[ENOUGH];
    int newsize, oldsize;
    int border, linesafter;
    int i, srow;
    char reply;
    struct CURSOR tmp;
    int fin_line, fin_col;
    int nx_line, nx_col;

    if ( find(pat,direction,fline,fcol) )
     { store_c(&virt_c,&tmp);

	/* auxscreen has been set up by find */
	c_row=0; c_col=ms_col;
	getline(ms_line, auxscreen[0]);
	copyover(str, true, fin_ch);
	fin_line = ms_line+c_row; fin_col = c_col-1;
	getline(mf_line,line);
	copyover(line+mf_col, false, fin_ch);

	newsize=c_row+1;
	oldsize=mf_line-ms_line+1;

	border=(NINROWS-newsize)/2;
	if (border<2) border=0;
	else
	if (border>2) border=2;

	if (ms_line-border < 0) border = ms_line;

	savescreen(); /* includes saveedit */

	if (newsize>NINROWS)
	  fdiag("!! replacement is too big !!");
	else
	if ( topl<=ms_line-border &&
	     ms_line+c_row+border-LASTINROW<=topl+LASTINROW )
	 { /* rewrite the screen by rolling up -
	     * first get the top border on the screen
	     */
	    if (ms_line>topl+LASTINROW+1) set_topl(ms_line-NINROWS);

	    if (ms_line+c_row+border<=topl+LASTINROW)
	      linesafter = (topl+LASTINROW)-(ms_line+c_row);
	    else
	    if (ms_line+c_row+border>maxl)
	      linesafter = maxl-(ms_line+c_row);
	    else
	      linesafter = border;

	    for (i=0; i<newsize+linesafter; i++)
	     { srow = ms_line-topl+i;
		getfline(ms_line+i, line); /* old copy */
		if (i>=newsize)
		  getfline(ms_line+oldsize+(i-newsize), auxscreen[i]);

		if (srow<=LASTINROW)
		  redraw(srow, 0, auxscreen[i]);
		else
		if (srow==LASTINROW+1)
		 { rollup(1);
		    redraw(LASTINROW, 0, auxscreen[i]);
		 }
		else editerror("change can't rollup");

		/* put old copy in auxscreen now */
		cat2(line,"",auxscreen[i]);
	     }
	 }
	else
	 { /* rewrite screen */
	    clearscreen();

	    topl=ms_line-((NINROWS-newsize)/2);
	    if (topl<0) topl=0;
	    else
	    if (topl+LASTINROW>maxl) topl=maxl-LASTINROW;

	    for (i=topl; i<=topl+LASTINROW; i++)
	     { if (i<ms_line)
		 { getfline(i,line); redraw(i-topl,0,line); }
		else
		 { getfline(i, line);

		    if (i>=ms_line+newsize)
		      getfline(i-newsize+oldsize, auxscreen[i-ms_line]);

		    redraw(i-topl, 0, auxscreen[i-ms_line]);

		    cat2(line,"",auxscreen[i-ms_line]);
		 }
	     }
	 }

	/* now see if the change is acceptable to the user */
	redraw(EDITROW,0,">ok?");
	position(ms_line-topl,ms_col);
	fixpos();
	complain();

	do
	 { reply = ttyin();
	    if (reply==c_ESCAPE ||  lcase(reply)=='y')
	     { showedit();
		if (newsize != oldsize)
		  shuffile(mf_line+1,newsize-oldsize);
		nx_line = (direction==FORWARD ? fin_line : ms_line);
		nx_col = (direction==FORWARD ? fin_col : ms_col);
	     }
	    else
	    if (lcase(reply)=='n' || reply==c_RUBOUT)
	     { showedit();
		for (i=ms_line; i<=topl+LASTINROW; i++)
		  redraw(i-topl, 0, auxscreen[i-ms_line]);
		nx_line = ms_line; nx_col = ms_col;
	     }
	    else
	     { redraw(EDITROW,0,">OK - Y OR N?");
		fixpos();
		complain();
		continue;
	     }

	    if (reply==c_ESCAPE || reply==c_RUBOUT)
	     /* round again! */
	     { store_c(&tmp, &virt_c);
		set_c(ms_line-topl, ms_col, (mode==EDMODE ? &in_c : &virt_c));
		blobit(); ttyflush();
		recall(change, pat, str, direction, fin_ch, nx_line, nx_col);
	     }
	    else
	     /* win or lose, you end up at the same position */
	     { store_c(&tmp,&virt_c);
		i_mode(ms_line-topl,ms_col);
	     }
	 } while (true);
     }
    else
      tdiag("?? can't find it ??");
 }

/* at present, 'special' just says whether to terminate on a '/' */
copyover(str, special, fin_ch)
char *str;
int special;
char fin_ch;
 { register char c;

    while (true)
     { switch (c = *str++)
	 { case '\'':
		if (special)
		 { c = *str++;
		    switch (c)
		     { case '\0':
			    fdiag("!! replacement string ends with ' !!");
			    /* no need to break */

			case '$':
			    auxscreen[c_row][c_col]=0;
			    goto nxline;

			default:
			    break;
		     }
		 }
	    /* fall through on most characters */
		goto normalchar;

	    default:
		if (c==fin_ch && special)
		 { if (*str!=0)
		      fdiag("?? string doesn't end after '%c' !!", fin_ch);
		    else
		     { auxscreen[c_row][c_col] = 0;
			return;
		     }
		 }
	    /* fall through on most characters */

	    normalchar:
		if (c_col>=ncols) break;
		else
		 { auxscreen[c_row][c_col++] = c;
		    continue;
		 }

	    case '\0':
		 auxscreen[c_row][c_col] = 0;
		 return;
	 }

	/* move to a new row */
	str--; /* rescan character */
    nxline:
	if (++c_row >= NINROWS)
	  fdiag("?? replacement is too large ??");
	else
	  c_col=0;
     }
}
