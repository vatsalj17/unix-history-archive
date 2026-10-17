# include "mac/quickdraw.h"
# include "basic.h"

Point pt ;

clrtoeol()
{
	GetPen ( & pt ) ;
	while ( pt.h < SCREENWIDTH )
	{
		DrawChar (' ') ;
		pt.h += CharWidth(' ') ;
	}
}
