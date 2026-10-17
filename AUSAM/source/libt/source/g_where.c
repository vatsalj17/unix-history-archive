#include "cplot.h"

/*
 *		This is the clever part of the clipping:
 *
 *			Each point is assigned a number dependent
 *			on its position relative to the window.
 *
 *			The numbers are designed to permit logical
 *			operations on the two endpoints of a vector.
 *
 *				|		|
 *				|		|
 *		     1001	|     1000	|        1010
 *				|		|
 *		-------------------------------------------------
 *				|		|
 *				|		|
 *		     0001	|     0000	|        0010
 *				|		|
 *				|		|
 *		-------------------------------------------------
 *				|		|
 *		     0101	|     0100	|        0110
 *				|		|
 *				|		|
 *
 *
 *
 *			Numbers are represented in binary.
 *			The logical and of the two points is
 *			true if the line lies entirely off the screen.
 *
 */


g_where( x, y )
register int x, y;
{
	register int val = 0;

	if( x < g_sxlo )
	{
		val = 1;
	}
	else
	{
		if( x > g_sxhi )
		{
			val = 2;
		}
	}

	if( y < g_sylo )
	{
		val =| 4;
	}
	else
	{
		if( y > g_syhi ) val =| 8;
	}
	
	return( val );
}
