#include <cplot.h>


/*
**	A tektronix terminal tester.
*/


main()
{
	int x,y;
	char c;


	init(stdout);
	erase();
	sxputsa(0,0,"THE SCREEN JUST ERASED AND I AM AT THE BOTTOM LEFT");
	smovea(0,0);
	sdrawa(1023,0);
	sdrawa(1023,780);
	sdrawa(0,780);
	sdrawa(0,0);
	scircle(511,390,250);
	box(200,200);
	box(823,200);
	box(823,580);
	box(200,580);
	syputsa(500,590,"WELL FOCUSED VERTICAL");
	sxputsa(300,750,"THE CURSORS SHOULD BE FOCUSED AND VISIBLE");
	sxputsa(300,730,"PRESS ANY CHARACTER AND IT SHOULD BE PRINTED");
	while((c = scursor(&x,&y)) != EOF)
	{
		smovea(x,y);
		alpha();
		g_put(c);

	}

	finish();
}

box(x,y)
int x,y;
{

	smovea(x,y);
	smover(-71,-71);
	sdrawr(0,100);
	sdrawr(100,0);
	sdrawr(0,-100);
	sdrawr(-100,0);
}
