#include "cplot.h"
sdasha(x,y,type)
int x,y,type;
{

	/*
	** This routine has a bug in that it relies
	** totaly on floats. A better routine should be
	** written, but we don't have the time to devote to
	** it at the moment. So please feel free to write
	** a better one and tell the authors about it
	*/
	double dist, tdist;
	double deltax, deltay;
	int len, saved;
	double sqrt();

	/*
	** TYPE 0 IS ALWAYS DEFINED AS A DRAW
	*/
	if( !type )
	{
		sdrawa(x,y);
		return;
	}

	if( !g_solve(g_spx,g_spy,x,y) )
	{
		/*
		** If both points are off screen then
		** don't do anything much
		*/
		g_scan = 0;
		g_spx = x;
		g_spy = y;
		g_status = g_last & ~ONSCREEN;
		return;
	}

	deltax = g_sqx2 - g_sqx1;
	deltay = g_sqy2 - g_sqy1;
	g_xold = g_xpos = g_sqx1;
	g_yold = g_ypos = g_sqy1;

	/*
	** TDIST CHANGES BUT DIST DOES NOT
	*/
	tdist = dist = sqrt(deltax*deltax + deltay*deltay );
	if( (g_status & ALPHA) || ! (g_status & ONSCREEN) )
	{
		g_last =& ~ONSCREEN;
	}
	if( dist<= g_dashbit)
	{
		go(g_xpos = g_sqx2, g_ypos =  g_sqy2);
		g_dashbit =- dist;
		g_spx = x;
		g_spy = y;
		g_status = g_last;
		return;
	}

	go( g_xpos =+ (deltax*g_dashbit)/dist, g_ypos =+ (deltay*g_dashbit)/dist );
	tdist =- g_dashbit;
	/* Flip the dash status */
	if( g_last & DASHMOVE )
	{
		g_last =& ~(DASHMOVE);
		g_last =| ONSCREEN;
	}
	else
	{
		g_last =| DASHMOVE;
		g_last =& ~ONSCREEN;
	}


	while( tdist> (len= g_dashtable[type][g_scan = (g_scan+1) %4]) )
	{
		go(g_xpos =+ (deltax*len)/dist, g_ypos =+ (deltay*len)/dist);
		tdist =- len;
		if( g_last & DASHMOVE )
		{
			g_last =& ~(DASHMOVE);
			g_last =& ~ONSCREEN;
		}
		else
		{
			g_last =| DASHMOVE;
			g_last =| ONSCREEN;
		}
	}

	go( g_xpos = g_sqx2, g_ypos =  g_sqy2);	/* GO THE LAST BIT */
	g_dashbit = len - tdist;

	g_spx = x;
	g_spy = y;
	g_status = g_last;
}
go(x,y)
double x,y;
{
	int xint,yint;
	if( !(g_last & DASHMOVE))
	{
/*
		if(! ((g_last & ONSCREEN) || (g_last & ALPHA)) )
		{
*/
			g_graphics();
			g_putxy(xint=g_xold,yint=g_yold);
/*
			g_last =| ONSCREEN;
		}
*/
		g_putxy( xint=x,yint=y);
	}
	else
	{
/*
		g_last =& ~ONSCREEN;
*/
	}
	g_xold = x;
	g_yold = y;
}
