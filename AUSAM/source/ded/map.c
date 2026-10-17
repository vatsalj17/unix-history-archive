#include "ded.h"
#include "char.h"

/* to allow for differing screen sizes, ded must
 * allocate its own store
 */
char *curr_break 0, *curr_lim 0;

char *my_alloc(size)
 { register char *res, *new;

    if ( (new = curr_break+size) > curr_lim)
     { res = (size-(curr_lim-curr_break)+1024) & (~ 1023);
	if ( (new = sbrk(res)) == -1)
	  editerror("no more room (my_alloc)");
	else
	 { curr_lim = new+res;
	    if (curr_break==0) curr_break = new;
	    new = curr_break+size;
	 }
     }

    res = curr_break;
    curr_break = new;
    return(res);
 }

char **newscreen()
 { register char **res, *map;
    register int row;
    int width;

    res = my_alloc(nrows*2);
    width = (ncols+2) & (~ 1);
    if (dbug('o'))
      editerror("nrows is %d, ncols is %d, width is %d", nrows, ncols, width);
    map = my_alloc(nrows*width);

    /* set pointers in map */
    for (row=0; row<nrows; row++)
     res[row] = map+row*width;

    return(res);
 }

setupscreen()
 { clearscreen();
    refreshscreen();
 }

refreshscreen()
 { register int row;
    char lbuf[ENOUGH];

    /* display file contents */
    for (row=0; row<=LASTINROW; row++)
     { getfline(topl+row,lbuf);
	redraw(row,0,lbuf);
     }

    showedit();
    ttyflush();
}

clearscreen()
 { register int row, col;
    register char *rm;
    int width;

    clear();

    /* store spaces in screen map */
    if (rowmap==0)
      editerror("screen map not yet set up (clearscreen)");

    for (row=0; row<=FOOTROW; row++)
     { rm = rowmap[row];
	for (col=0; col<=ncols-1; col++)
	  *rm++ = c_SPACE;
     }
 }

char *copyrow(srow,scol,buf)
int srow, scol;
char *buf;
 { register char *pmap, *pbuf;
    register int count;

    pmap = rowmap[srow]+scol;
    if ( (pbuf = buf)==0 ) editerror("row not set up (copyrow)");
    count = lastcol(srow)-scol+1;

    while (count-- > 0)
      *pbuf++ = *pmap++;

    *pbuf = 0;
 }

/* insert a row AFTER screen row srow.
 * this must be done so that the data on screen moves
 * (in order that the user can see that the insert really has occurred)
 * and also so that the minimum number of characters are transferred
 */
ins_row(srow,buffer,pos)
int srow;
char *buffer;
struct CURSOR *pos;
 { register int i, reg_row;
    struct CURSOR tmp;

    reg_row = srow;

    if (reg_row<=2 ||
	reg_row<LASTINROW &&
	    (reg_row<firstrow() ||
	     charsin(0,reg_row)>charsin(reg_row+1,LASTINROW)) )
    /* top rows stay on screen - just pull lower lines
     * down one
     */
     { saverow(LASTINROW);

	if (ttytype==hazeltine)
	 { saveedit(); forcedown(reg_row+1); showedit(); }
	else
	  for (i=LASTINROW; i>reg_row+1; i--)
	    redraw(i,0,rowmap[i-1]);

	if (pos->row>reg_row && pos->row<LASTINROW) pos->row++;
     }
    else
    /* make room by pushing top row off screen  - there
     * are two ways of doing this */
     { if ( ttytype==hazeltine ||
	    charsin(reg_row+1,FOOTROW)<charsin(0,reg_row) )
	/* roll up screen, and pull lower lines down one */
	 { saveedit();
	    rollup(1);
	    if (ttytype==hazeltine)
	     forcedown(reg_row);
	    else
	    { showedit();
	      for (i=LASTINROW; i>reg_row; i--)
		redraw(i,0,rowmap[i-1]);
	    }
	 }
	else
	/* push top rows up one position */
	 { saverow(0);
	    inc_topl();
	    for (i=0; i<reg_row; i++)
	      redraw(i,0,rowmap[i+1]);
	 }

	if (pos->row<=reg_row && pos->row>0) pos->row--;
	reg_row--;
     }

    shuffile(topl+reg_row+1,1);
    redraw(reg_row+1,0,buffer);
 }

/* delete a screen row - as above this should usually
 * involve moving the picture somewhat
 */
del_row(srow,pos)
int srow;
struct CURSOR *pos;
 { register int i, reg_row;
    char linebuf[ENOUGH];
    struct CURSOR tmp;

    reg_row=srow;

    if (topl!=0 &&
	(reg_row<=firstrow() ?
	   !emptyline(topl-1) ||
	    (reg_row>lastrow() && topl+LASTINROW>=maxl) :
	   topl+LASTINROW>=maxl ||
	    (reg_row>=lastrow() ?
	       emptyline(topl+LASTINROW+1) :
	       (ttytype==itt &&
		charsin(0,reg_row-1)<charsin(reg_row+1,LASTINROW))) ))
    /* pull the top lines down */
     { for (i=reg_row; i>=0; i--)
	 { getline(topl+i-1,linebuf);
	    redraw(i,0,linebuf);
	 }
	if (pos->row<=reg_row && pos->row<LASTINROW) pos->row++;
	shuffile(topl+reg_row+1,-1);
	topl--;
     }
    else
    /* pull the bottom lines up */
     { while (topl+LASTINROW>=maxl) inc_maxl();

	if (ttytype==hazeltine)
	 { saveedit(); forceup(reg_row);
	   getline(topl+LASTINROW+1, linebuf);
	   redraw(LASTINROW, 0, linebuf);
	   showedit();
	 }
	else
	  for (i=reg_row; i<=LASTINROW; i++)
	   { getline(topl+i+1,linebuf);
	     redraw(i,0,linebuf);
	   }

	if (pos->row>reg_row && pos->row>0) pos->row--;
	shuffile(topl+reg_row+1,-1);
     }
 }

/* contents of editrow - saved by saveedit etc. */
char erow[ENOUGH];

/* two functions to help when rolling about the screen */
saveedit()
 { copyrow(EDITROW,0,erow); }

showedit()
 { redraw(EDITROW,0,erow); ttyflush(); }


clear()
{	switch (ttytype)
	{
	case itt:	ttyout(c_CLEAR);
			break;
	case hazeltine: ttyout(c_LEADIN);
			ttyout('\034');
			break;
	case VC404:	ttyout('\030');
			ttyout('\014'); /* for screen clear delay */
			break;
	case T1061:	ttyout('\014');
			break;
	default:	 editerror("tty type not set up (clear)");
	}

    real_c.row = real_c.col = 0;
 }

/* looks at the screen row srow and the file line fline.
 * If they are different, it rewrites the line.
 */
int tmp_changed false;

saverow(srow)
int srow;
 { char sstr[ENOUGH], fstr[ENOUGH];

    copyrow(srow, 0, sstr); /* essential to convert it into a string */
    getfline(topl+srow,fstr);

    if (!streq(sstr, fstr))
     { addline(topl+srow, sstr); tmp_changed = true; }
 }

savescreen()
 { int i;

    for (i=0; i<=LASTINROW; i++)
     saverow(i);

    saveedit();
 }

/* procedures to help in hardware-scrolling terminals */
forcedown(row)
int row;
 { register int reg_row;
   struct CURSOR tmp;

   store_c(&virt_c, &tmp);
   virt_c.row = reg_row = row;
   if (ttytype==hazeltine)
    { fixpos(); ttyout(c_LEADIN); ttyout(04032); /* confuse ttyout */
      real_c.col = 0;
    }
   else editerror("forcedown on non-hazeltine");

   store_c(&tmp, &virt_c);
   shufscreen(reg_row, FOOTROW, 1);
 }

forceup(row)
int row;
 { register int reg_row;
   struct CURSOR tmp;

   store_c(&virt_c, &tmp);
   virt_c.row = reg_row = row;
   if (ttytype==hazeltine)
    { fixpos(); ttyout(c_LEADIN); ttyout(023);
      real_c.col = 0;
    }
   else editerror("forceup on non-hazeltine");

   store_c(&tmp, &virt_c);
   shufscreen(reg_row, FOOTROW, -1);
 }
