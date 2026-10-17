#include "cplot.h"

smover(x,y)
int x,y;
{
	g_spx =+ x;
	g_spy =+ y;
	g_status =& ~(USER|ONSCREEN);
}
