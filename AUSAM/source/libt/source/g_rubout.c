#include "cplot.h"

/*
Deletes trap to this routine, which puts the terminal in initial mode.
*/
g_rubout()
{
	signal( 2 , 1 );
	finish();
	exit(1);
}

