#include "cplot.h"

/*	Changes to screen co-ords.	*/

g_chtscr(x,y)
float x,y;
{

	g_spx = (x-g_xlo) * g_sxtrans + g_sxlo;
	g_spy = (y-g_ylo) * g_sytrans + g_sylo;
}

