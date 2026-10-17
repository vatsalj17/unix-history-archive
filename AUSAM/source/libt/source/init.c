#include "cplot.h"
int	g_dashtable[NUM_DASHES][4]
{
0,0,0,0,
5,5,5,5,
20,10,20,10,
20,10,5,10,
30,10,30,10,
20,10,20,10,
20,10,1,10,
0,0,0,0,
0,0,0,0,
0,0,0,0
};

extern g_rubout();

init( out )
FILE *out;
{
	int fd;
	/*	Set rotates and scales to reasonable values.	*/

	g_xsin = 0.0;
	g_ysin = 0.0;

	g_xcos = 1.0;
	g_ycos = 1.0;

	g_xscale = 1.0;
	g_yscale = 1.0;

	/*	Change terminal mode for graphics.	*/
	fd = fileno( out );
	g_opentk( fd );

	if( gtty( fileno( stdout ), &g_ttybuf ) != 0 )
	{
		/* this guy is out of his mind */
		g_nekmod = 0;
	}
	else
		g_nekmod = 1;
	g_out = out;
	/*	Screen window defaults the whole screen.	*/
	g_sxhi = SXMAX;
	g_sylo = 0 ;
	g_syhi = SYMAX;
	g_sxlo=0;


	signal( 2,g_rubout);


	/*	Default user window.	*/
	g_xlo = 0;
	g_xhi = SXMAX;
	g_ylo = 0;
	g_yhi = SYMAX;


	swindow(g_sxlo, g_sxhi, g_sylo, g_syhi );
	uwindow(g_xlo, g_xhi, g_ylo, g_yhi);

	/*	Initialize terminal output bytes.	*/
	g_b1 = 0;
	g_b2 = 0;
	g_b3 = 0;
	g_b4 = 0;
	
	g_status = (SCREEN|USER|ONSCREEN);
	g_alpha();
}

