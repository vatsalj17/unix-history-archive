#include "cplot.h"

spointa(x,y)
int x,y;
{
	g_spx = x;
	g_spy = y;

	g_point();
	g_status =& ~(USER);
}
