# include	"hangman.h"
# include	"mac/quickdraw.h"

/*
 * prdata:
 *	Print out the current guesses
 */
prdata()
{
	register bool	*bp;
	char 			drawbuf[ 200 ] ;
	int 			width ;

	MoveTo (GUESSH + StringWidth("Guessed:  "), GUESSV);
	bp = Guessed;
	while (bp < &Guessed[26])
		if (*bp++)
			DrawChar ( (char) (((unsigned) bp - (unsigned) Guessed) + 'a' - 1));
	clrtoeol();

	width = StringWidth("Current Average:      ") ;

	MoveTo ( NUMBERH + width, NUMBERV ) ;
	sprintf ( drawbuf, "%d", Wordnum ) ;
	DrawString ( drawbuf ) ;

	MoveTo ( AVGH + width, AVGV ) ;
	sprintf (drawbuf, "%.3f", (Average * (Wordnum - 1) + Errors) / Wordnum);
	DrawString ( drawbuf ) ;

	MoveTo ( AVGH + width, AVGV + LINEHEIGHT ) ;
	sprintf (drawbuf, "%.3f", Average) ;
	DrawString ( drawbuf ) ;

}
