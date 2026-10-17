#include "cplot.h"

swindow(xl,xh,yl,yh)
register int xl,xh,yl;
int yh;
{
	/*	The whole screen is the most that can be asked for
		and is the most obtained!
	*/
	if ((xl >= xh) || (xl < 0) || (xh > SXMAX) ||
	    (yl >= yh) || (yl < 0) || (yh > SYMAX))
	{
		return(-1);
	}
	else
	{
	        g_sxlo = xl;
	        g_sxhi = xh;
	        g_sylo = yl;
	        g_syhi = yh;
	}
	g_sxtrans = (g_sxhi-g_sxlo) / (g_xhi-g_xlo);
	g_sytrans = (g_syhi-g_sylo) / (g_yhi-g_ylo);

	g_uxtrans = 1.0 / g_sxtrans;
	g_uytrans = 1.0 / g_sytrans;

	g_status =& ~USER;
	return(0);
}

