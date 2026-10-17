#include "ded.h"

int txin false;

int mode;
int toprow, bottomrow, leftcol, rightcol;
struct CURSOR in_c, edit_c;


mainloop(ctab, changemode, pmode)
char  ctab[];
int (*changemode)();
/* by which I mean - changemode is the address of a function */
int pmode;
 { register char c;
    char xc;
    register int ctype;
    int length;
    int col;
    char line[ENOUGH], *lp, *lpr;

    mode = pmode;
    if (mode==INMODE)
     { store_c(&in_c,&virt_c);
	toprow=0; bottomrow=LASTINROW;
	leftcol=0; rightcol=ncols-1;
     }
    else
     { store_c(&edit_c,&virt_c);
	toprow=bottomrow=EDITROW;
	leftcol=1; rightcol=ncols-2;
     }

    fixpos();
    ttyflush();

    while (true)
     { c=ttyin();
	switch (ctype=ctab[c])
	 { case IGN:   complain(); break;              /* don't type those */

	    case ONE:
	    case SPACE:
	    case MODIFY:
	    case CONTROL:
	    case TWO:
	    case TAB:
		if (virt_c.col>BELLMARGIN)
		 { /* complain(); (too bloody loud) */
		    col = (ctype==SPACE || ctype==TAB ? virt_c.col :
				sp_left(virt_c.col-1)+1);
		    if (txin && mode==INMODE && col>0)
		     /* copy the new part of the line down one position */
		     { copyrow(virt_c.row, col, line);
			redraw(virt_c.row, col, "   ");
			ins_row(virt_c.row, line, &virt_c);

			virt_c.row++; virt_c.col =- col;

			if (ctype==SPACE) break;
		     } /* IF txin */
		 } /* if BELLMARGIN */
		draw(c,ctype);
		/* if (lastcol(virt_c.row)>BELLMARGIN) complain();
		 * (too bloody loud)
		 */
		break;

	    case RUBOUT:
	    case CERASE:
	    case WRUBOUT:
	    case WERASE:
	    case HEADERASE:
	    case TAILERASE:
		draw(c,ctype); break;

	case UP:
	    if (virt_c.row<=toprow)
	     { virt_c.row=toprow;
		scrdown(SCROLL);
		adj_virt_c(-1,0);
	     }
	    else virt_c.row--;
	    break;

       case DOWN:
	    if (virt_c.row>=bottomrow)
	     { virt_c.row=bottomrow;
		scrup(SCROLL);
		adj_virt_c(1,0);
	     }
	    else virt_c.row++;
	    break;

       case LEFT:
	    if (virt_c.col<=leftcol) { virt_c.col=leftcol; complain(); }
	    else virt_c.col--;
	    break;

       case RIGHT:
	    if (mode==EDMODE && hit_diagnostic())
	     /* hack to prevent going into no-man's land of diagnostic
	      * message
	      */
	      complain();
	    else
	     { if (virt_c.col>=rightcol)
		 { virt_c.col=rightcol; complain(); }
		else virt_c.col++;
	     }
	    break;

       case WLEFT:
	    if ((col=backword(virt_c.col))==virt_c.col) complain();
	    else virt_c.col = col;
	    break;

       case WRIGHT:
	    if (mode==EDMODE && hit_diagnostic())
	      complain();
	    else
	     { if ((col=nextword(virt_c.col))==virt_c.col) complain();
		else virt_c.col = col;
	     }
	    break;

       case CR:
	    copytail(line);
	    cleartail();
	    ins_row(virt_c.row, line, &virt_c);
	    adj_virt_c(1,0);  virt_c.col = leftcol;
	    break;

       case NL:
	    if (topl+virt_c.row >= maxl) complain();
	    else
	     { if (virt_c.row >= bottomrow)
		 { virt_c.row=bottomrow;
		    scrup(SCROLL);
		    adj_virt_c(1,0);
		 }
		else virt_c.row++;
		virt_c.col = leftcol;
	     }
	    break;

	case LSTART:
	    goto lstart; /* aargh! */

	case LEND:
	    goto lend; /* aargh again ! */

	case SPECIAL:
	    { c = ttyin();
	       switch(ctab[c])
		{  case UP:
			if (virt_c.row != toprow) virt_c.row=toprow;
			else complain();
			break;

		    case DOWN:
			if (virt_c.row != bottomrow) virt_c.row=bottomrow;
			else complain();
			break;

		    case LEFT:
			if (virt_c.col != leftcol) virt_c.col=leftcol;
			else complain();
			break;

		    case RIGHT:
			col = (mode==EDMODE ? find_diag() : rightcol);
			if (virt_c.col >= col) complain();
			virt_c.col=col;
			break;

		    case WLEFT:
		    lstart:             /* look at this! */
			col=firstpos(virt_c.row);
			if (col <= rightcol)
			 { if (col==virt_c.col) complain();
			    else virt_c.col = col;
			 }
			else
			 { complain(); virt_c.col = leftcol; }
			break;

		    case WRIGHT:
		    lend:               /* look at this !  */
			col = (mode==EDMODE &&
			       (col=find_diag()) < rightcol ?
					col-1 :
					lastpos(virt_c.row));
			if (col >= leftcol)
			 { if (++col<rightcol) col++;
			    else col = rightcol;

			    if (col==virt_c.col) complain();
			    else
			      virt_c.col = col;
			 }
			else
			 { complain();  virt_c.col = leftcol; }
			break;

		    case SPECIAL:
			if (!do_command()) complain();
			break;

		    default:
			complain(); break;
		     }
		}
		break;

	    case CHMODE: (*changemode)(); break;

	    case MODEetoi:
		if (do_command())
		  (*changemode)();
		else complain();
		break;

	    case MODEitoe:
		redraw(EDITROW,0,">");
		set_c(EDITROW,1,&edit_c);
		(*changemode)();
		break;

	    default: editerror("bad character type (%d) in input",ctype);
		     break;
	 }
	fixpos();
	ttyflush();
    }
 }

int hit_diagnostic()
 { return((rowmap[virt_c.row])[virt_c.col]==0); }

int find_diag(option)
int option;
 { register int i;
    register char *rp;

    rp = rowmap[EDITROW];
    for (i=0; i<=rightcol; i++)
      if (rp[i] == 0) return(i);
    return(rightcol);
 }

itoe()
 { /* change from input to edit mode */
    store_c(&virt_c,&in_c);
    recall(&mainloop,ectab,etoi,EDMODE);
 }

etoi()
 { /* change from edit to input mode */
    store_c(&virt_c,&edit_c);
    recall(&mainloop,ictab,itoe,INMODE);
 }
