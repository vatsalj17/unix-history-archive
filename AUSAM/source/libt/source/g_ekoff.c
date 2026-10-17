#include "cplot.h"

/*
	Turns erase and kill processing off ( for cursor input especially)
*/

g_ekoff()
{
	struct g_ttystruct no_ek;

	if( g_nekmod )
	{
		no_ek.ispeed = g_ttybuf.ispeed;
		no_ek.ospeed = g_ttybuf.ospeed;
		no_ek.e = 0;
		no_ek.k = 0;
		no_ek.mode = g_ttybuf.mode ;
		no_ek.mode =& ~(ECHO);
		stty(fileno(stdout), &no_ek);
	}
}
