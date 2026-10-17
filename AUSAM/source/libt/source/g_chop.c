#include "cplot.h"

g_chop( x, y, xc, yc, c )
int *x, *y, xc, yc, c;
{

	if( c & 1 )
	{
		/*	Must force floating point or funny overflow conditions arise.	*/
		*y =+ (yc - *y) * 1.0 * (g_sxlo - *x)/(xc - *x);
		*x = g_sxlo;
	}
	else
		if( c & 2 )
		{
			*y =+ (yc - *y) * 1.0 * (g_sxhi - *x) / (xc - *x);
			*x = g_sxhi;
		}
		else
			if( c & 4 )
			{
				*x =+ (xc - *x) * 1.0 * (g_sylo - *y)/(yc - *y);
				*y = g_sylo;
			}
			else
				if( c & 8 )
				{
					*x =+ (xc - *x) * 1.0 * (g_syhi - *y)/(yc - *y);
					*y = g_syhi;
				}

	return( g_where( *x, *y ));
}
