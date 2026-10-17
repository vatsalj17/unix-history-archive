# include	"hangman.h"
# include	"mac/quickdraw.h"

/*
 * prword:
 *	Print out the current state of the word
 */
prword()
{
	MoveTo (KNOWNH + StringWidth ( "Word:  " ), KNOWNV );
	DrawString (Known);
	clrtoeol();
}
