#include "cplot.h"

g_alpha()
{
	if( ! (g_status & ALPHA) ) g_put( US );
	g_status =| ALPHA;
}
