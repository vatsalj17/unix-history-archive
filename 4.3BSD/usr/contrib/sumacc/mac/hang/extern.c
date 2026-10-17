# include	"hangman.h"

bool	Guessed[26];

char	Word[BUFSIZ],
	    Known[BUFSIZ] ;

int	Errors,
	Wordnum = 0;

float	Average = 0.0;

FILE	*Dict = NULL;

off_t	Dict_size, DictRef ;

short HeadRect[] = { STARTV, STARTH, STARTV+20, STARTH+20 } ;
