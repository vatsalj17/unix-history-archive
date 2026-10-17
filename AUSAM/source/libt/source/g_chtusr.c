#include "cplot.h"

/*	Changes	to user co-ords.	*/

g_chtusr(x,y)
int x,y;
{
	g_px = (x-g_sxlo) * g_uxtrans  + g_xlo;
	g_py = (y-g_sylo) * g_uytrans  + g_ylo;
	g_status =| USER;
}

