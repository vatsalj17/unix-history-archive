#include <cplot.h>
main()
{
	int x,y;
	register int a;
	register int num;


	init(stderr);
	erase();
	sxputsa( 20, 700, "type digits and ctrl/d to exit" );
	while((num = scursor(&x,&y)) != EOF )
	{
		num =- '0';
		num =* 50;
		for(a=5;a<num;a=+ 7)
			scircle(x,y,a);
	}
	syputsa( x, y, "bye bye" );
	finish();
}
