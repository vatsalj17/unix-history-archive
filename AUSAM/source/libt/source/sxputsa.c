#include "cplot.h"

sxputsa(x,y,s)
int x,y;
register char	*s;
{
	register char c;
	g_positioned = 0;

	g_sax = x;
	g_say = y;

	/*	Put out each character only if it is on the screen.	*/
	while( c = *s++ ) 
	{
		g_cput( c );
	}

	/*	Notice graphics cursor is left unchanged.	*/
	g_status =& ~ONSCREEN;
}


