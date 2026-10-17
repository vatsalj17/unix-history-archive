#include "cplot.h"

syputsa(x,y,s)
register int x,y;
char *s;
{
	register char c;
	while( c = *s++) 

	{
		if( g_in(x,y)  )
		{
			g_graphics();
			g_putxy(x,y);
			g_alpha();
			g_put(c);
			y =- YCHSZ;
		}
	}
	g_status =& ~ONSCREEN;
}
