#include "cplot.h"

finish()
{
	register int i;

	/*	Put cursor home.	*/
	g_graphics();
	g_putxy(HOMEX,HOMEY);

	/*	Put terminal in alpha mode.	*/
	g_alpha();

	/*	Restore to original mode.	*/
	g_closetk();

}

