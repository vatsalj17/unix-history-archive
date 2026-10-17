
#include "cplot.h"
g_in( x , y)
int x, y;
{


	if( (g_sxlo>x) || (g_sxhi<x) || (g_sylo>y) || (g_syhi<y) ) return(0);
	return(1);
}

