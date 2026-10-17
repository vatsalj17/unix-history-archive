# include	"hangman.h"
# include 	"mac/quickdraw.h"
# include 	"mac/osintf.h"
# include	"mac/toolintf.h"

struct QDVar QDVar ;
WindowRecord wRecord ;
WindowPtr theWindow ;
short wirect[] = { 50, 40, 300, 450 } ;
extern int DictRef ;

# define VIS 1

/*
 * This game written by Ken Arnold.
 * Modified to run on Macintosh by Dan Winkler.
 */
main()
{
	InitScreen();
	setup();
	for (;;) {
		Wordnum++;
		playgame();
		Average = (Average * (Wordnum - 1) + Errors) / Wordnum;
	}
	/* NOTREACHED */
}

/*
 * die:
 *	Die properly.
 */
die()
{
	FSClose ( DictRef ) ;  /* returns an error code but we aren't very
							* interested at this point.
							*/
	ExitToShell() ;
}

InitScreen()
{
     /* set up the quickdraw and the visible screen */
	 QD = & QDVar ;
	 InitGraf( & thePort ) ;
	 QD->randSeed = TickCount() ;
	 InitFonts() ;
     InitWindows() ;
	 InitCursor() ;
	 theWindow = NewWindow ( &wRecord, (Rect *) wirect, "Unix Hangman",
								VIS, documentProc, (WindowPtr) -1, 1, 0 ) ;
 	 FlushEvents(everyEvent,0);
	 SetPort ( theWindow ) ;
	 TextMode ( srcCopy ) ;
}
