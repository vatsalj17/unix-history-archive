#include "ded.h"

struct CURSOR real_c, virt_c;

/* make the screen cursor (real_c)
 * same as the user cursor (virt_c)
 */
fixpos()
 { register int srow, scol;

    if ( (srow=virt_c.row) <0) virt_c.row=srow=0;
    else
    if (srow>FOOTROW) virt_c.row=srow=FOOTROW;

    if ( (scol=virt_c.col) <0) virt_c.col=scol=0;
    else
    if (scol>ncols-1) virt_c.col=scol=ncols-1;

    if (srow != real_c.row || scol !=real_c.col)
      move(srow,scol);
 }

/* set user cursor to a particular address */
set_virt_c(srow,scol)
int srow, scol;
 { virt_c.row = srow; virt_c.col = scol; }

/* store cursor */
store_c(from, to)
struct CURSOR *from, *to;
 { to->row = from->row;
    to->col = from->col;
 }

/* adjust cursor position */
adj_virt_c(r,c)
int r,c;
 { if ((virt_c.row =+ r)<toprow) virt_c.row=toprow;
    else
    if (virt_c.row > bottomrow) virt_c.row = bottomrow;

    if ((virt_c.col =+ c)<leftcol) virt_c.col = leftcol;
    else
    if (virt_c.col > rightcol) virt_c.col = rightcol;
 }

/* set some arbitrary cursor */
set_c(r,c,pos)
int r,c;
struct CURSOR *pos;
 { pos->row = r;
    pos->col = c;
 }

/* move cursor to some address */
position(srow,scol)
int srow,scol;
 { set_virt_c(srow,scol); fixpos(); }
