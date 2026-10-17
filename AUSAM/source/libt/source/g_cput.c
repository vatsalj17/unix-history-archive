#include "cplot.h"

g_cput( c )
char c;
{

	if( g_in(g_sax, g_say) )
	{
		if( ! g_positioned )
		{
			g_graphics();
			g_putxy(g_sax, g_say);
			/*	Put the terminal in alpha mode.	*/
			g_alpha();
			g_positioned = 1;
		}
		g_put(c);
	}
	g_sax =+ XCHSZ;
}
