#include "cplot.h"

uxputsa(x,y,s)
	  
float x,y;
char *s;
{

	return( sxputsa( (x-g_xlo) * (g_sxhi-g_sxlo) / (g_xhi-g_xlo),
		 (y-g_ylo) * (g_syhi-g_sylo) / (g_yhi-g_ylo), s ) );

}

