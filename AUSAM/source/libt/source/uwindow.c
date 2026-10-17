#include "cplot.h"

uwindow(xl,xh,yl,yh)
float xl,xh,yl,yh;
{

	/*	Cant afford to mess up plotting with error messages,
		so window defaults the screen if unreasonable windows given.
	*/
	if ((xl == xh) || (yl == yh)) return(-1);
        g_xlo = xl;
        g_xhi = xh;
        g_ylo = yl;
        g_yhi = yh;

	g_sxtrans = (g_sxhi-g_sxlo)/(g_xhi-g_xlo);
	g_sytrans = (g_syhi-g_sylo)/(g_yhi-g_ylo);

	g_uxtrans = 1.0 / g_sxtrans;
	g_uytrans = 1.0 / g_sytrans;

	g_status =& ~USER;
	return(0);
}

