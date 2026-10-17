# include	"hangman.h"
# include	"mac/quickdraw.h"
# include	"mac/osintf.h"
# include	"mac/toolintf.h"

extern int DictRef ;

/*
 * getword:
 *	Get a valid word out of the dictionary file
 */
getword()
{
	register char		*wp, *gp;
	int					try ;

	for ( try = 0 ; try < MAXTRY ; try ++ ) 
	{
		/* move to a random point in the file */
		if ( SetFPos ( DictRef, 1, myabs(myrandom() % Dict_size)) != noErr )
		{
			error ("Can't set file marker position...") ;
			ExitToShell() ;
		}

		if (macfgets(Word, BUFSIZ, DictRef) == ERROR)
			continue;
		if (macfgets(Word, BUFSIZ, DictRef) == ERROR)
			continue;

		/* throw away short words */
		if (strlen(Word) < MINLEN)
		{
			/* error ("Got short word...") ; */
			continue;
		}

		/* throw away words with capital letters */
		for (wp = Word; *wp; wp++)
			if (!islower(*wp))
				goto cont;
		break;
		cont: /* error("Got word with capital letter...") */ ;
	}

	if ( try >= MAXTRY )
	{
		error ("Can't get good word.  Bye.") ;
		ExitToShell() ;
	}

	gp = Known;
	wp = Word;
	while (*wp) {
		*gp++ = '-';
		wp++;
	}
	*gp = '\0';
}

/*
 * abs:
 *	Return the absolute value of an integer
 */
off_t
myabs(i)
off_t	i;
{
	if (i < 0) return -(off_t) i;
	else return (off_t) i;
}


macfgets ( buffer, bufsize, filerefnum )
/* imitates unix fgets using macintosh file system */
	char * buffer ;
	int bufsize, filerefnum ;
{
	int count ;

	if ( bufsize < 2 ) 
	{
		error ("macfgets: can't read into zero length buffer...") ;
		return ERROR ;
	}

	/* skip non alpha characters if any */
	do
	{
		count = 1 ;
		if ( FSRead ( filerefnum, & count, buffer ) != noErr )
		{
			/* error ("macfgets: error skipping white space...") ; */
			return ERROR ;
		}
	}
	while ( ! isalpha( * buffer ) ) ;

	++buffer ; --bufsize ;

	/* continue reading until a non alpha character is reached */
	do
	{
		count = 1 ;

		if ( FSRead ( filerefnum, & count, buffer ) != noErr )
		{
			/* error ("macfgets: error reading string...") ; */
			return ERROR ;
		}

		if ( ! isalpha( *buffer ) )
		{
			*buffer = '\0' ;
			return OK ;
		}

		buffer ++ ; bufsize -- ;
	}
	while ( bufsize ) ;

	error ("macfgets: buffer is full...") ;
	return ERROR ;
}

myrandom()
/* return random integer using quickdraw's Random() which returns
 * a random short
 */
{
	return (Random() + ( ((int) Random()) << 16 )) ;
}

