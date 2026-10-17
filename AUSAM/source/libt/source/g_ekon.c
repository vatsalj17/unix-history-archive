#include "cplot.h"

/*
 *		Restoring erase and kill status
 */

g_ekon()
{
	if( g_nekmod )
	{
		stty( fileno( stderr ), &g_ttybuf );
	}
}
