#include "cplot.h"

/*
Restores terminal to original mode.
*/

g_closetk()
{
	if( g_term ) stty( g_term, &g_outbuf );
}

