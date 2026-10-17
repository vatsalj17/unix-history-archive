#include "cplot.h"

udasha(x,y,type)
float x,y;
int type;
{
	int tempx,tempy;
	g_sax = g_spx;
	g_say = g_spy;
	g_chtscr(g_px = x, g_py = y);
	tempx = g_spx;
	g_spx = g_sax;
	tempy = g_spy;
	g_spy = g_say;
	sdasha(tempx, tempy, type);
}

