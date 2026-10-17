#include "cplot.h"

g_point()
{
	if( g_in(g_spx, g_spy) )
	{
		g_graphics();
		g_putxy(g_spx, g_spy);
		g_putxy(g_spx, g_spy);
	}
}
