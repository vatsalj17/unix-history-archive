#include "ded.h"
#include "char.h"

/* make the necessary movements on the screen */
draw(ch, chtype)
char ch;
int chtype;
 { register char c;
    register int ctype;
    char tail[ENOUGH];
    int i;
    int col, col1, col2;
    int ntpos;
    int fline;

    c=ch; ctype=chtype;
    do
     { switch (ctype)
     { case CONTROL:
	case ONE:
	case MODIFY:
	case SPACE:
	    if (!roomfor(1, virt_c.row))
	      break;
	    else
	     { copytail(tail);
		show(c);
		if (virt_c.col<rightcol)
		 { virt_c.col++; showtail(tail); }
	     }
	    return;

	case TWO:
	    if (!roomfor(2, virt_c.row))
	      break;
	    else
	     { copytail(tail);
		show(c_MODIFY); virt_c.col++;
		show(unmodify(c));
		if (virt_c.col<rightcol)
		 { virt_c.col++; showtail(tail); }
	     }
	    return;

	case TAB:
	    if ((ntpos = nextab(virt_c.col)) > BELLMARGIN);
	    /* complain();
	     * (too bloody loud)
	     */
	    col = virt_c.col;
	    while ((rowmap[virt_c.row])[col]==c_SPACE && col<ntpos) col++;
	    if (!roomfor(ntpos-col, virt_c.row))
	      cdiag("!! no room on line !!");
	    else
	     { virt_c.col = col;
		if (virt_c.col != ntpos)
		 { copytail(tail);
		    while (virt_c.col < ntpos)
		     { show(c_SPACE); adj_virt_c(0,1); }
		    showtail(tail);
		 }
	     }
	    return;

	case RUBOUT:
	    if (virt_c.col<=leftcol)
	     { virt_c.col=leftcol;
		if (mode==INMODE)
		 { if (virt_c.row==toprow)
		      scrdown(SCROLL);

		    if (virt_c.row==toprow)
		      cdiag("!! top of file !!");
		    else
		    if (!roomfor(lastcol(virt_c.row)+2, virt_c.row-1))
		      cdiag("!! no room on line above !!");
		    else
		     {  copytail(tail);
			fline = topl+virt_c.row;
			del_row(virt_c.row, &virt_c);
			virt_c.row = fline-topl-1;
			position(virt_c.row, lastcol(virt_c.row)+2);
			showtail(tail);
		     } /* shift line */
		 } /* mode==INMODE */
		else complain(); /* edit mode bleep at left col */
	     }
	    else
	     { copytail(tail);
		virt_c.col--;
		showtail(tail);
	     }
	    return;

	case CERASE:
	    copytail(tail);
	    if (*tail == 0) complain();
	    else showtail(tail+1);
	    return;

	case WRUBOUT:
	    if (virt_c.col<=leftcol) complain();
	    else
	     { col = backword(virt_c.col);
		copyrow(virt_c.row, virt_c.col, tail);
		virt_c.col = col;
		showtail(tail);
	     }
	    return;

	case WERASE:
	    if (virt_c.col > lastpos(virt_c.row) ||
		(mode==EDMODE && hit_diagnostic()) )
			complain();
	    else
	     { col = nextword(virt_c.col);
		copyrow(virt_c.row,col,tail);
		showtail(tail);
	     }
	    return;

	case HEADERASE:
	    if (virt_c.col <= leftcol) { virt_c.col=leftcol; complain(); }
	    else
	     { copytail(tail);
		virt_c.col = leftcol;
		showtail(tail);
	     }
	    return;

	case TAILERASE:
	    if (lastcol(virt_c.row)<virt_c.col) complain();
	    else cleartail();
	    return;

	default:    editerror("invalid character type in draw %d",ctype);
      }

    /* come here (via 'break') when line is absolutely full
     * - break line in two if possible
     */
    col1 = sp_right(virt_c.col)+1;
    col2 = sp_left(virt_c.col-1)+1;
    if (txin && mode==INMODE &&
	 ( (col1>BELLMARGIN && (col=col2)>0) ||
	   (col=col1)>0 ))
     { copyrow(virt_c.row, col, tail);
	redraw(virt_c.row, col, "  ");
	ins_row(virt_c.row, tail, &virt_c.row);
	if (col<virt_c.col)
	 { virt_c.row++; virt_c.col =- col; }
	continue;
     }
    else
     { cdiag("!! no room on line !!"); return; }
    } while (true);
 }


roomfor(num, srow)
int num, srow;
 { register char *sp;
    register int n;

    n=num;
    sp = rowmap[srow]+rightcol;

    while (n-- > 0) if (*sp-- != c_SPACE) return(false);
    return(true);
 }


show(c)
char c;
 { register char rc;

    fixpos();
    ttyout(rc=c);
    (rowmap[real_c.row])[real_c.col] = rc;
    if (++real_c.col >= ncols)
     { if (++real_c.row >= nrows) editerror("rolled off bottom of screen");
	/* god only knows where it goes */
	real_c.col = ncols*10; real_c.row = nrows*10;
     }
 }

showtail(s)
char *s;
 { redraw(virt_c.row, virt_c.col, s); }

copytail(tail)
char *tail;
 { copyrow(virt_c.row, virt_c.col, tail); }


/* in order to avoid too many characters, only draw those
 * characters which have altered
 */
char blanks[] "                                                       ";

redraw(srow,scol,p_newrow)
int srow,scol;
char *p_newrow;
 { struct CURSOR tmp;
    register char *oldrow, *newrow;
    register char c;
    char *oldrowlim;

    store_c(&virt_c, &tmp);

    if ((newrow=p_newrow) == 0) newrow = blanks;
    oldrow = rowmap[srow]+scol;
    oldrowlim = rowmap[srow]+(srow==FOOTROW ? ncols-2 : ncols-1);

    virt_c.row = srow;
    while (oldrow <= oldrowlim)
     { if ((c = *newrow++) != *oldrow++ || c==0)
	 { if (c==0)
	     { newrow = blanks; oldrow--; }
	    else
	     { virt_c.col=oldrow-rowmap[srow]-1;
	       show(c);
	     }
	 }
     }

    store_c(&tmp,&virt_c);
 }

cleartail()
 { redraw(virt_c.row, virt_c.col, blanks); }

backword(scol)
int scol;
 { register char *rp, *rplim;

    rp = rowmap[virt_c.row] + scol;
    rplim = rowmap[virt_c.row] + leftcol;

    if (rp>rplim)
     { rp--; /* move back to previous character */

	/* the next piece of nastiness moves back to the previous character
	 * if the cursor is pointing at a space
	 */
	if (rp>rplim && *rp ==c_SPACE && *--rp ==c_SPACE)
	 { while (rp>rplim && *(rp-1) == c_SPACE) rp--; /* space sequence */
	    if (rp!=rplim) rp++; /* move one right except at lhs */
	 }
	else
	if (rp>rplim && alphmer(*rp))  /* word */
	  while (rp>rplim && alphmer(*(rp-1))) rp--;
	/* it was punctuation - do nothing else */
     }

    return(rp-rowmap[virt_c.row]);
 }

nextword(scol)
int scol;
 { register char *rp, *rplim;

    if (mode==EDMODE && hit_diagnostic()) return(virt_c.col);

    rp = rowmap[virt_c.row]+scol;
    rplim = rowmap[virt_c.row] + rightcol;

    if (rp<rplim && *rp==c_SPACE) /* space */
     { if (scol==0 || rp[-1] == c_SPACE)
	  while (rp<rplim && *rp==c_SPACE)
	    rp++; /* skip all the spaces in a sequence */
     }
    else
    if (rp<rplim && alphmer(*rp)) /* word */
      while (rp<rplim && alphmer(*rp)) rp++; /* skip word */
    else /* punctuation */
    if (rp<rplim) rp++;

    /* whatever happens, skip space following */
    if (rp<rplim && *rp==c_SPACE) rp++;

    return(rp-rowmap[virt_c.row]);
 }

int sp_right(scol)
int scol;
 { register int i;
    register char *rp;

    rp = rowmap[virt_c.row]+scol;
    for (i=scol; i<=rightcol; i++)
      if (*rp++ == c_SPACE) return(i);

    return(-1);
 }

int sp_left(scol)
int scol;
 { register int i;
    register char *rp;

    rp = rowmap[virt_c.row]+scol;
    for (i=scol; i>=leftcol; i--)
      if (*rp-- == c_SPACE) return(i);

    return(-1);
 }
