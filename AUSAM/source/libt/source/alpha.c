#include "cplot.h"

alpha()

{

	/*	If conceptual cursor is on the screen move the real cursor there.	*/
	if( !(g_status & ONSCREEN) || (g_status & ALPHA) )
	{
		if( g_in( g_spx, g_spy ) )
		{
			g_graphics();
			g_putxy( g_spx, g_spy);
		}
	}

	/*	Put terminal in alpha mode.	*/
	g_alpha();
	/* If output is buffered, then flush is  */
	fflush(g_out);

}

