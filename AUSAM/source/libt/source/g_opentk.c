#include "cplot.h"

/*
Opens terminal for graphics and saves original mode.
*/

g_opentk(fd)
int fd;
{

	if( gtty(fd,&g_outbuf)==0 ) 
	{
		g_term = fd;
		g_ttyb.ispeed = g_outbuf.ispeed;
		g_ttyb.ospeed = g_outbuf.ospeed;
		g_ttyb.e = g_outbuf.e;
		g_ttyb.k = g_outbuf.k;
		g_ttyb.mode = g_outbuf.mode;

		g_ttyb.mode =| FF1;
		g_ttyb.mode =& ~(NLDELAY|MAPCASE);

		stty(fd,&g_ttyb);

		g_syncs = (g_ttyb.ospeed-8);
		if( g_syncs < 0 ) g_syncs = 0;
	}
	else
		g_term = 0;
}

