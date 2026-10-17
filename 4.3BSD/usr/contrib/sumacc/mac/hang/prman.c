# include	"hangman.h"
# include	"mac/quickdraw.h"

/*
 * prman:
 *	Print out the hanged man appropriately for the given number
 *	of incorrect guesses.
 */

extern short HeadRect[] ; /* rectangle that bounds the man's head */

prman()
{
	switch ( Errors )
	{
		case 1 :	FrameOval ( (Rect *) HeadRect ) ; break ;	
		case 2 :	MoveTo ( STARTH + 10, STARTV + 20 ) ; 
					LineTo ( STARTH + 10, STARTV + 40 ) ; 
					break ;
		case 3 :	MoveTo ( STARTH + 10, STARTV + 40 ) ; 
					LineTo ( STARTH + 10, STARTV + 60 ) ; 
					break ;
		case 4 :	MoveTo ( STARTH - 10, STARTV + 20 ) ; 
					LineTo ( STARTH + 10, STARTV + 40 ) ; 
					break ;
		case 5 :	MoveTo ( STARTH + 30, STARTV + 20 ) ; 
					LineTo ( STARTH + 10, STARTV + 40 ) ; 
					break ;
		case 6 :	MoveTo ( STARTH - 10, STARTV + 80 ) ; 
					LineTo ( STARTH + 10, STARTV + 60 ) ; 
					break ;
		case 7 :	MoveTo ( STARTH + 30, STARTV + 80 ) ; 
					LineTo ( STARTH + 10, STARTV + 60 ) ; 
					break ;
	}
}

eraseman()
/* erases hanged man at end of game */
{
	int olderrs ;

	olderrs = Errors ;
	PenPat ( & QD->white ) ;
	for ( Errors = 1 ; Errors <= 7 ; Errors ++ )
		prman() ;
	PenPat ( & QD->black ) ;
	Errors = olderrs ;
}
