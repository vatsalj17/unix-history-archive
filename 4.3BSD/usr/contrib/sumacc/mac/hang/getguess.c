

# include	"hangman.h"
# include	"mac/quickdraw.h"
# include	"mac/toolintf.h"
# include	"mac/osintf.h"

EventRecord theEvent ;

/*
 * getguess:
 *	Get another guess
 */
getguess()
{
	register int	i;
	register int	ch;
	register bool	correct;

	for (;;) {
		MoveTo (PROMPTH + StringWidth("Guess:  "), PROMPTV );
		ch = readch();
		if (isalpha(ch)) {
			if (isupper(ch))
				ch = tolower(ch);
			if (Guessed[ch - 'a'])
			{
				MoveTo(MESGH, MESGV) ;
				clrtoeol() ;
				MoveTo(MESGH, MESGV) ;
				DrawString ("Already guessed '") ;
				DrawChar ( ( char ) ch ) ;
				DrawString ("'") ;
			}
			else
				break;
		}
		else if (ch == QUITCHAR)
			die();
		else
		{
			MoveTo (MESGH, MESGV) ;
			clrtoeol() ;
			MoveTo (MESGH, MESGV) ;
			DrawString ("Not a valid guess.") ;
		}
	}
	MoveTo (MESGH, MESGV);
	clrtoeol();

	Guessed[ch - 'a'] = TRUE;
	correct = FALSE;
	for (i = 0; Word[i] != '\0'; i++)
		if (Word[i] == ch) {
			Known[i] = ch;
			correct = TRUE;
		}
	if (!correct)
		Errors++;
}

/*
 * readch;
 *	Read a character from the keyboard
 */
readch()
{
	do 
		GetNextEvent( everyEvent, & theEvent ) ;
	while ( theEvent.what != keyDown ) ;

	return ( int ) ( theEvent.message & 0xff ) ;
}

