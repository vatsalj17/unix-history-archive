#include "ded.h"
#include "char.h"

diag_clear()
 { register int i;
    register char *ep;

    ep = rowmap[EDITROW];

    for (i=0; i<=ncols-2; i++)
      if (ep[i] == 0)
	{ redraw(EDITROW,i,"   "); return; }
 }

diag_show(c)
char c;
 { if (virt_c.col<=ncols-2)
     { show(c); virt_c.col++; }
 }

diag(str, c1, c2, c3, c4, c5)
char *str;
int c1, c2, c3, c4, c5;
 { struct CURSOR tmp;

    diag_clear();
    store_c(&virt_c,&tmp);

    virt_c.row = EDITROW;
    virt_c.col = lastcol(EDITROW)+2;

    fixpos();
    if (virt_c.col<=ncols)
     { (rowmap[EDITROW])[virt_c.col-1] = 0;
	if (mode==EDMODE && tmp.col>=virt_c.col) tmp.col = virt_c.col-1;
     }

    diag_print(str, c1,c2,c3,c4,c5);
    ttyflush();

    store_c(&tmp, &virt_c);
 }

diag_print(str, c1,c2,c3,c4,c5)
char *str;
int c1,c2,c3,c4,c5;
 { char c;
    int *par;

    par = &c1;

    while (c = *str++)
      if (c=='%')
       {   switch (lcase(c = *str++))
	     { case 'c':
		    diag_show(*par++); break;

		case 's':
		    diag_print(*par++); break;

		case 'd':
		    diag_num(*par++); break;

		case '\0':
		    diag_show('%');
		    return;

		default:
		    diag_show('%'); diag_show(c);
		    break;
	     }
      }
    else
     diag_show(c);
 }

diag_num(n)
int n;
 { if (n>=10) diag_num(n/10);
    diag_show( (n%10)+'0' );
 }

blobit()
 { diag("%c",c_BLOB); }

cdiag(s, p1, p2, p3, p4, p5, p6)
char *s;
int p1, p2, p3, p4, p5, p6;
 { diag(s, p1, p2, p3, p4, p5, p6);
    complain();
 }
