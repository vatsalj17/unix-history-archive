# include	"hangman.h"
# include	"mac/quickdraw.h"
# include	"mac/osintf.h"

/*
 * setup:
 *	Set up the strings on the screen.
 */

extern int DictRef ; /* reference number for dictionary file */
extern int Dict_size ;

setup()
{
	PolyHandle			PH ;

	MoveTo(KNOWNH, KNOWNV);
	DrawString ("Word: ");
	MoveTo(KNOWNH, KNOWNV + 2 * LINEHEIGHT) ;
	DrawString("Written by Ken Arnold.  Ported to Mac by Dan Winkler.") ;

	MoveTo(GUESSH, GUESSV) ;
	DrawString ("Guessed:");
	MoveTo(NUMBERH, NUMBERV) ;
	DrawString ("Word #:");
	MoveTo(AVGH, AVGV) ; 
	DrawString ("Current Average:");
	MoveTo(AVGH, AVGV + LINEHEIGHT) ;
	DrawString ("Overall Average:");
	MoveTo(AVGH, AVGV + 3 * LINEHEIGHT) ;
	DrawString ("Press Tab to quit.") ;

	/* draw the gallows */
	MoveTo ( STARTH + 9, STARTV - 2 ) ;
	PenSize ( 3, 3 ) ;
	LineTo ( STARTH + 9, STARTV - 10 ) ;
	LineTo ( STARTH - 40, STARTV - 10 ) ;
	LineTo ( STARTH - 40, STARTV + 90 ) ;
	PenSize ( 2, 2 ) ;

	PH = OpenPoly() ;
	LineTo ( STARTH, STARTV + 90 ) ;
	LineTo ( STARTH, STARTV + 105 ) ;
	LineTo ( STARTH + 30, STARTV + 105 ) ;
	LineTo ( STARTH + 30, STARTV + 120 ) ;
	LineTo ( STARTH - 70, STARTV + 120 ) ;
	LineTo ( STARTH - 70, STARTV + 90 ) ;
	LineTo ( STARTH - 30, STARTV + 90 ) ;
	ClosePoly() ;
	FillPoly ( PH, & QD->black ) ;
	KillPoly ( PH ) ;

	/* open the dictionary file */
	if ( FSOpen (DICT, 0, & DictRef) != noErr) 
	{
		MoveTo(KNOWNH, KNOWNV - LINEHEIGHT);
		DrawString ("Can't open dictionary file ") ;
		DrawString ( DICT ) ;
		DrawString (".  Bye.") ;
		ExitToShell() ;
	}

	/* get the dictionary size */
	if ( GetEOF ( DictRef, & Dict_size ) != noErr )
	{
		MoveTo(KNOWNH, KNOWNV - LINEHEIGHT);
		DrawString ("Can't determine size of dictionary file ") ;
		DrawString ( DICT ) ;
		DrawString (".  Bye.") ;
		ExitToShell() ;
	}
}

error( s )
/* print an error message */
	char * s ;
{
		MoveTo(KNOWNH, KNOWNV - LINEHEIGHT);
		DrawString ( s ) ;
}
