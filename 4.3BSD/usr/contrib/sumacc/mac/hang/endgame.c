# include	"hangman.h"
# include 	"mac/quickdraw.h"

/*
 * endgame:
 *	Do what's necessary at the end of the game
 */
endgame()
{
	register char	ch;

	prman();
	if (Errors >= MAXERRS)
		Errors = MAXERRS + 2;
	prword();
	prdata();
	MoveTo (MESGH, MESGV);
	if (Errors > MAXERRS)
	{
		DrawString ("Sorry, the word was \"") ;
		DrawString ( Word ) ;
		DrawString ("\"                                         ") ;
	}
	else
		DrawString ("You got it!                                          ");

	for (;;) {
		MoveTo (MESGH, MESGV + LINEHEIGHT ) ;
		DrawString ( "Another word? ");
		if ((ch = readch()) == 'n')
			die();
		else if (ch == 'y')
			break;
		MoveTo (MESGH, MESGV + 2 * LINEHEIGHT) ;
		DrawString ("Please type 'y' or 'n'");
	}
	eraseman() ;
	MoveTo (MESGH, MESGV) ;
	clrtoeol() ;
	MoveTo (MESGH, MESGV + LINEHEIGHT) ;
	clrtoeol() ;
	MoveTo (MESGH, MESGV + 2 * LINEHEIGHT) ;
	clrtoeol() ;
	MoveTo(KNOWNH, KNOWNV);
	DrawString ("Word: ");
}
