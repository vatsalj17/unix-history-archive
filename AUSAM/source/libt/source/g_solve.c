
#include "cplot.h"

/*
 *
 *		Well here it is, the new, amazing, improved solve.
 *
 *		It is faster, smaller, easier to follow and in general
 *		bears no relationship to earlier programs which appeared
 *		mysteriously and disappeared in much the same way.
 *
 *		The clipping algorithm is straightforward and based on
 *		the one in "Principles of Interactive Computer Graphics".
 *		It has the advantage that divisions by zero do not need to
 *		be checked for because they dont happen.
 *		Also, the time taken for clipping is proportional to the
 *		extent to which it is out of the window, which is optimal
 *		since most vectors will be in the window anyway.
 *
 *
 */

g_solve( x1, y1, x2, y2 )
int	x1, x2, y1, y2;
{
	int c1, c2;

	g_last = g_status;

	c1 = g_where( x1, y1 );
	c2 = g_where( x2, y2 );

	/*	solve the first point   (x1, y1)		*/

	if( c1 )
	{
		g_status =& ~(ONSCREEN);
		/*	Clip until endpoint is on screen.	*/
		while( c1 )
		{
			/*	return immediately if the line is off screen	*/
			if( c1 & c2 )
			{
				return( 0 );
			}
			c1 = g_chop( &x1, &y1, x2, y2, c1 );
		}
	}

	g_sqx1 = x1;
	g_sqy1 = y1;


	/*	solve the second point (x2,y2)			*/
	if( c2 )
	{
		g_last =& ~ONSCREEN;
		/*	Clip until point is on the screen.	*/
		while( c2 )
		{
			/*	return immediately if the line is off screen	*/
			if( c1 & c2 )
			{
				return( 0 );
			}
			c2 = g_chop( &x2, &y2, x1, y1, c2 );
		}
	}
	else
	{
		g_last =| ONSCREEN;
	}

	g_sqx2 = x2;
	g_sqy2 = y2;

	g_last =& ~ALPHA;
	return( 1 );
}
