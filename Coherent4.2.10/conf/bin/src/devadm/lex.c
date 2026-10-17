/*
 * Simple ad-hoc lexer.
 */

/*
 *-IMPORTS:
 *	<sys/compat.h>
 *		USE_PROTO
 *		ARGS ()
 *	<string.h>
 *		strchr ()
 */

#include <sys/compat.h>
#include <string.h>

#include "ehand.h"
#include "ecodes.h"

#include "lex.h"


CONST char	LEX_WILD [1];		/* special wildcard value */

lex_t	WHITESPACE [] = {
	{ NULL, STD_FLUSH, STD_SEP, LEX_WILD, NULL }
};


/*
 * Here the code that uses the "lex_t" structure to classify a character has
 * been factored out.
 */

#if	USE_PROTO
int (classify) (lex_t * lexp, int ch, int flush)
#else
int
classify ARGS ((lexp, ch, flush))
lex_t	      *	lexp;
int		ch;
int		flush;
#endif
{
	int		valid = 0;
	int		invalid = 0;
	CONST lex_t   *	scan = lexp;

	if (ch < 0)
		throw_error (LEX_ERROR,
			     "Bad character passed to classify ()");

	do {
		/*
		 * Classify the new character.
		 */

		if (flush && scan->l_flush != NULL &&
		    strchr (scan->l_flush, ch) != 0)
			return CLASS_FLUSH;

		if (scan->l_sep != NULL && strchr (scan->l_sep, ch) != 0)
			return CLASS_SEP;

		if (scan->l_valid == LEX_WILD ||
		    (scan->l_valid != NULL &&
		     strchr (scan->l_valid, ch) != NULL))
			valid = ! invalid;

		if (scan->l_error == LEX_WILD)
			invalid = ! valid;
		else if (scan->l_error != NULL &&
			 strchr (scan->l_error, ch) != NULL) {

			invalid = ! valid;
			break;
		}
	} while ((scan = scan->l_prev) != NULL);


	if (! valid) {

		if (invalid)
			throw_error (LEX_ERROR,
				     "invalid character value 0x%x", ch);

		throw_error (LEX_ERROR,
			     "unexpected character value 0x%x", ch);
	}

	return CLASS_VALID;
}

