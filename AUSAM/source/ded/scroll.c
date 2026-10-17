#include "ded.h"
#include "char.h"

scrdown(n)
int n;
 { int i;

    if (n<0 || n>NINROWS) editerror("too far to scroll down (%d)",n);
    else
    if (topl<=0)
      { topl=0; complain(); return; }

    set_topl(topl-n);
    if (mode==INMODE) adj_virt_c(n,0);
 }

scrup(n)
int n;
 { char lbuf[ENOUGH];
    int i;

    if (n<0 || n>NINROWS) editerror("too far to scroll up (%d)",n);
    else
    if (topl+LASTINROW >=maxl)
      { complain(); return; }

    if (topl+LASTINROW+n > maxl) n = maxl-(topl+LASTINROW);

    saveedit();

    for (i=1; i<=n; i++)
     { rollup(1);
	getfline(topl+LASTINROW,lbuf);
	redraw(LASTINROW,0,lbuf);
     }

    showedit();

    if (mode==INMODE) adj_virt_c(-n,0);
 }

rollup(n)
 { struct CURSOR tmp;

    store_c(&virt_c, &tmp);

    while (n-- > 0)
     { position(FOOTROW,real_c.col);
	ttyout(c_NL);

	saverow(0);
	inc_topl();

	shufscreen(0, FOOTROW, -1);
     }

    store_c(&tmp, &virt_c);
 }

/* move lines up or down the screen */
shufscreen(row1, row2, incr)
int row1, row2;
 { register int i, r;
   register char *spare;

   if (incr<0) /* move up the screen */
    { spare = rowmap[row1];

      for (i=row1; i<row2; i++)
	rowmap[i] = rowmap[i+1];

      rowmap[row2] = spare;
    }
   else /* move down the screen */
    { spare = rowmap[row2];

      for (i=row2; i>row1; i--)
	rowmap[i] = rowmap[i-1];

      rowmap[row1] = spare;
    }

   for (i=0; i<=ncols-1; i++)
     *spare++ = c_SPACE;
}

set_topl(n)
int n;
 { if (n<0) n=0;
    else
    if (n+LASTINROW>maxl) n=maxl-LASTINROW;

    if (topl<n && n<=topl+LASTINROW) scrup(n-topl);
    else
    if (n != topl)
     { savescreen();
	saveedit();
	topl = n;
	setupscreen();
	showedit();
     }
 }

inc_topl()
 { topl++;
    while (topl+LASTINROW>maxl) inc_maxl();
 }

inc_maxl()
 { if (++maxl>NLINES)
      editerror("too many lines in file (inc_topl)");
    else
      l_block[maxl] = l_offset[maxl] = -1;
 }
