# include	"../fileio.h"

/*
**  GETCHAR -- get character from standard input
**
**	This getchar() looks just like the getchar in libc.a
**	except that it returns EOF (-1) on end of file (instead of
**	zero).
**
**	Parameters:
**		none
**
**	Return:
**		EOF -- end-of-file or error
**		else -- next character from standard input
**
**	Requires:
**		getc -- to actually get
**
**	Defines:
**		getchar()
**
**	History:
**		12/30/77 -- written by eric
**		2/6/78 (eric) -- changed to return EOF (not zero) on
**			end of file or error.
*/

getchar()
{
	return (getc(stdin));
}
